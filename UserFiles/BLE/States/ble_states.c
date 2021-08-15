

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"
#include "hosted_functions.h"
#include "security_manager.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	LOCAL_SUPPORTED_COMMANDS,
	LOCAL_SUPPORTED_FEATURES,
	SET_EVENT_MASK,
	SET_LE_EVENT_MASK,
	READ_BUFFER_SIZE,
	LE_READ_BUFFER_SIZE,
	LE_LOCAL_SUPPORTED_FEATURES,
	READ_BD_ADDRESS,
	GENERATE_STATIC_RANDOM_ADDRESS,
	READ_LOCAL_VERSION,
	CLEAR_WHITE_LIST,
	ADDRESS_RESOLUTION,
	CLEAR_RESOLVING_LIST,
	READ_RESOLVING_LIST_SIZE,
	SET_RPA_TIMEOUT,
	CLEAR_TIMER,
	WAIT_STATUS,
	BLE_INIT_DONE
}BLE_INIT_STEPS;


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
void Set_BLE_State( BLE_STATES NewBLEState );
static uint8_t Reset_Controller( void );
static void Reset_Complete( CONTROLLER_ERROR_CODES Status );
static uint8_t BLE_Init( void );
static void Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status );
static void Clear_White_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Read_Resolving_List_Size_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Resolving_List_Size );
static void LE_Set_Resolvable_Private_Address_Timeout_Complete( CONTROLLER_ERROR_CODES Status );
static void Read_Local_Version_Information_Complete( CONTROLLER_ERROR_CODES Status,
		LOCAL_VERSION_INFORMATION* Local_Version_Information );
static void Read_Local_Supported_Commands_Complete( CONTROLLER_ERROR_CODES Status,
		SUPPORTED_COMMANDS* Supported_Commands );
static void Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status,
		SUPPORTED_FEATURES* LMP_Features );
static void Read_BD_ADDR_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* BD_ADDR );
static void LE_Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Read_Buffer_Size_Complete( CONTROLLER_ERROR_CODES Status, uint16_t LE_ACL_Data_Packet_Length,
		uint8_t Total_Num_LE_ACL_Data_Packets );
static void LE_Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status,
		LE_SUPPORTED_FEATURES* LE_Features );
static void Hal_Device_Standby_Event( CONTROLLER_ERROR_CODES Status );


/****************************************************************/
/* extern functions declaration                                 */
/****************************************************************/
extern int8_t Advertising_Config( void );
extern void Advertising( void );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static BLE_STATES BLEState = RESET_CONTROLLER;
static BLE_INIT_STEPS BLEInitSteps = LOCAL_SUPPORTED_COMMANDS;
static BLE_STATUS Controller_Reset_Flag = BLE_ERROR;
static BLE_STATUS Standby_Flag = BLE_FALSE;
static SUPPORTED_COMMANDS HCI_Supported_Commands;
static SUPPORTED_FEATURES HCI_LMP_Features;
static LE_SUPPORTED_FEATURES HCI_LE_Features;
static uint16_t LE_ACL_Data_Packet_Length_Supported;
static uint8_t Total_Num_LE_ACL_Data_Packets_Supported;
static LOCAL_VERSION_INFORMATION LocalInfo;
static uint8_t ControllerResolvingListSize;


/****************************************************************/
/* Run_BLE()        	        								*/
/* Location: 					 								*/
/* Purpose: Run the BLE protocol								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Run_BLE( void )
{
	int8_t AdvConfigStatus;

	switch( Get_BLE_State() )
	{
	case RESET_CONTROLLER:
		if( Reset_Controller(  ) )
		{
			Set_BLE_State( VENDOR_SPECIFIC_INIT );
		}
		break;

	case VENDOR_SPECIFIC_INIT:
		if( Vendor_Specific_Init(  ) )
		{
			Set_BLE_State( BLE_INITIAL_SETUP );
		}
		break;

	case BLE_INITIAL_SETUP:
		if( BLE_Init(  ) )
		{
			Standby_Flag = BLE_FALSE;
			Set_BLE_State( STANDBY_STATE );
		}
		break;

	case STANDBY_STATE:
		if( Standby_Flag != BLE_TRUE )
		{
			if( ACI_Hal_Device_Standby( &Hal_Device_Standby_Event, NULL ) )
			{
				Standby_Flag = BLE_TRUE;
			}
		}
		break;

	case CONFIG_ADVERTISING:
		AdvConfigStatus = Advertising_Config(  );
		if( AdvConfigStatus == TRUE )
		{
			Set_BLE_State( ADVERTISING_STATE );
		}else if( AdvConfigStatus < 0 )
		{
			Standby_Flag = BLE_FALSE;
			Set_BLE_State( STANDBY_STATE );
		}
		break;

	case ADVERTISING_STATE:
		Advertising(  );
		break;

	case SCANNING_STATE:
		break;

	case INITIATING_STATE:
		break;

	case CONNECTION_STATE:
		break;

	case SYNCHRONIZATION_STATE:
		break;

	case ISOCHRONOUS_BROADCASTING_STATE:
		break;
	}

	Vendor_Specific_Process();
	Hosted_Functions_Process();
}


/****************************************************************/
/* Get_BLE_State()        	       								*/
/* Location: 					 								*/
/* Purpose: Get the operating BLE state							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BLE_STATES Get_BLE_State( void )
{
	return( BLEState );
}


/****************************************************************/
/* Set_BLE_State()        	       								*/
/* Location: 					 								*/
/* Purpose: Set the operating BLE state							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Set_BLE_State( BLE_STATES NewBLEState )
{
	BLEState = NewBLEState;
}


/****************************************************************/
/* Get_Supported_Commands()        	       						*/
/* Location: 					 								*/
/* Purpose: Get the Controller supported commands.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
SUPPORTED_COMMANDS* Get_Supported_Commands( void )
{
	return ( &HCI_Supported_Commands );
}


/****************************************************************/
/* Get_Supported_Features()        	       						*/
/* Location: 					 								*/
/* Purpose: Get the Controller supported features.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
SUPPORTED_FEATURES* Get_Supported_Features( void )
{
	return ( &HCI_LMP_Features );
}


/****************************************************************/
/* Get_Local_Version_Information()        	 					*/
/* Location: 					 								*/
/* Purpose: Get the Controller local version.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
LOCAL_VERSION_INFORMATION* Get_Local_Version_Information( void )
{
	return ( &LocalInfo );
}


/****************************************************************/
/* BLE_Init()        	       									*/
/* Location: 					 								*/
/* Purpose: Init BLE 											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t BLE_Init( void )
{
	static uint32_t WaitCmdTimer;

	switch ( BLEInitSteps )
	{
	case LOCAL_SUPPORTED_COMMANDS:
		BLEInitSteps = HCI_Read_Local_Supported_Commands( &Read_Local_Supported_Commands_Complete, NULL ) ? CLEAR_TIMER : LOCAL_SUPPORTED_COMMANDS;
		break;

	case LOCAL_SUPPORTED_FEATURES:
		memset( &HCI_LMP_Features, 0, sizeof(HCI_LMP_Features) );
		if( HCI_Supported_Commands.Bits.HCI_Read_Local_Supported_Features )
		{
			BLEInitSteps = HCI_Read_Local_Supported_Features( &Read_Local_Supported_Features_Complete, NULL ) ? CLEAR_TIMER : LOCAL_SUPPORTED_FEATURES;
		}else
		{
			BLEInitSteps = SET_EVENT_MASK;
		}
		break;

	case SET_EVENT_MASK:
		if( HCI_Supported_Commands.Bits.HCI_Set_Event_Mask )
		{
			EVENT_MASK Event_Mask;
			memset( &Event_Mask, 0, sizeof(Event_Mask) );

			/* TODO: should add connection request and disconnection complete? */
			/* Flush occurred might also be important. HCI_Data_Buffer_Overflow also? */
			Event_Mask.Bits.Read_Remote_Supported_Features_Complete_event = 1;
			Event_Mask.Bits.Read_Remote_Version_Information_Complete_event = 1;
			Event_Mask.Bits.Hardware_Error_event = 1;
			Event_Mask.Bits.Read_Remote_Extended_Features_Complete_event = 1;
			Event_Mask.Bits.LE_Meta_event = 1;

			BLEInitSteps = HCI_Set_Event_Mask( Event_Mask, &Set_Event_Mask_Complete, NULL ) ? CLEAR_TIMER : SET_EVENT_MASK;
		}else
		{
			BLEInitSteps = SET_LE_EVENT_MASK;
		}
		break;

	case SET_LE_EVENT_MASK:
		if( HCI_LMP_Features.Bits.LE_Supported && HCI_Supported_Commands.Bits.HCI_LE_Set_Event_Mask )
		{
			LE_EVENT_MASK LE_Event_Mask;

			/* Enable all events from the LE Controller */
			memset( &LE_Event_Mask, 0xFF, sizeof(LE_Event_Mask) );

			BLEInitSteps = HCI_LE_Set_Event_Mask( LE_Event_Mask, &LE_Set_Event_Mask_Complete, NULL ) ? CLEAR_TIMER : SET_LE_EVENT_MASK;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case LE_READ_BUFFER_SIZE:
		if( HCI_Supported_Commands.Bits.HCI_LE_Read_Buffer_Size_v2 )
		{
			/* TODO: this command should exist only in more updated BLE versions. */
			BLEInitSteps = CLEAR_TIMER;
		}else if( HCI_Supported_Commands.Bits.HCI_LE_Read_Buffer_Size_v1 )
		{
			BLEInitSteps = HCI_LE_Read_Buffer_Size( &LE_Read_Buffer_Size_Complete, NULL ) ? CLEAR_TIMER : LE_READ_BUFFER_SIZE;
		}else
		{
			BLEInitSteps = READ_BUFFER_SIZE;
		}
		break;

	case READ_BUFFER_SIZE:
		if( HCI_Supported_Commands.Bits.HCI_Read_Buffer_Size )
		{
			/* TODO: this implementation does not make use of HCI_Read_Buffer_Size() used in BR/EDR Bluetooth. */
			BLEInitSteps = CLEAR_TIMER;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case LE_LOCAL_SUPPORTED_FEATURES:
		if( HCI_Supported_Commands.Bits.HCI_LE_Read_Local_Supported_Features )
		{
			BLEInitSteps = HCI_LE_Read_Local_Supported_Features( &LE_Read_Local_Supported_Features_Complete, NULL ) ? CLEAR_TIMER : LE_LOCAL_SUPPORTED_FEATURES;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case READ_BD_ADDRESS:
		if( HCI_Supported_Commands.Bits.HCI_Read_BD_ADDR )
		{
			/* Read Public Device Address */
			BLEInitSteps = HCI_Read_BD_ADDR( &Read_BD_ADDR_Complete, NULL ) ? CLEAR_TIMER : READ_BD_ADDRESS;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case GENERATE_STATIC_RANDOM_ADDRESS:
		if( Get_Static_Random_Device_Address().Status != TRUE ) /* Verify if an static address is available */
		{
			if( Generate_Device_Address( &HCI_Supported_Commands, STATIC_DEVICE_ADDRESS, NULL, 1 ) != NULL )
			{
				BLEInitSteps = READ_LOCAL_VERSION;
			}
		}else
		{
			BLEInitSteps = READ_LOCAL_VERSION;
		}
		break;

	case READ_LOCAL_VERSION:
		if( HCI_Supported_Commands.Bits.HCI_Read_Local_Version_Information )
		{
			BLEInitSteps = HCI_Read_Local_Version_Information( &Read_Local_Version_Information_Complete, NULL ) ? CLEAR_TIMER : READ_LOCAL_VERSION;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case CLEAR_WHITE_LIST:
		if( HCI_Supported_Commands.Bits.HCI_LE_Clear_White_List )
		{
			BLEInitSteps = HCI_LE_Clear_White_List( &Clear_White_List_Complete, NULL ) ? CLEAR_TIMER : CLEAR_WHITE_LIST;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case ADDRESS_RESOLUTION:
		/* Disable address resolution */
		BLEInitSteps = HCI_LE_Set_Address_Resolution_Enable( FALSE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? CLEAR_TIMER : ADDRESS_RESOLUTION;
		break;

	case CLEAR_RESOLVING_LIST:
		BLEInitSteps = HCI_LE_Clear_Resolving_List( &LE_Clear_Resolving_List_Complete, NULL ) ? CLEAR_TIMER : CLEAR_RESOLVING_LIST;
		break;

	case READ_RESOLVING_LIST_SIZE:
		BLEInitSteps = HCI_LE_Read_Resolving_List_Size( &LE_Read_Resolving_List_Size_Complete, NULL ) ? CLEAR_TIMER : READ_RESOLVING_LIST_SIZE;
		break;

	case SET_RPA_TIMEOUT:
		/* 900 seconds is the default value */
		BLEInitSteps = HCI_LE_Set_Resolvable_Private_Address_Timeout( 900, &LE_Set_Resolvable_Private_Address_Timeout_Complete, NULL ) ? CLEAR_TIMER : SET_RPA_TIMEOUT;
		break;

	case CLEAR_TIMER:
		BLEInitSteps = WAIT_STATUS;
		WaitCmdTimer = 0;
		break;

	case WAIT_STATUS:
		BLEInitSteps = TimeBase_DelayMs( &WaitCmdTimer, 500, TRUE ) ? LOCAL_SUPPORTED_COMMANDS : WAIT_STATUS;
		break;

	case BLE_INIT_DONE:
		BLEInitSteps = LOCAL_SUPPORTED_COMMANDS;
		return (TRUE);
		break;

	default:
		break;
	}

	return (FALSE);
}


/****************************************************************/
/* Read_Local_Supported_Commands_Complete()      				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Supported_Commands_Complete( CONTROLLER_ERROR_CODES Status, SUPPORTED_COMMANDS* Supported_Commands )
{
	if( Status == COMMAND_SUCCESS )
	{
		HCI_Supported_Commands = *Supported_Commands;
		BLEInitSteps = LOCAL_SUPPORTED_FEATURES;
	}else
	{
		BLEInitSteps = LOCAL_SUPPORTED_COMMANDS;
	}
}


/****************************************************************/
/* Read_Local_Supported_Features_Complete()      				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status, SUPPORTED_FEATURES* LMP_Features )
{
	if( Status == COMMAND_SUCCESS )
	{
		HCI_LMP_Features = *LMP_Features;
		BLEInitSteps = SET_EVENT_MASK;
	}else
	{
		BLEInitSteps = LOCAL_SUPPORTED_FEATURES;
	}
}


/****************************************************************/
/* Clear_White_List_Complete()        							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Clear_White_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? ADDRESS_RESOLUTION : CLEAR_WHITE_LIST;
}


/****************************************************************/
/* LE_Set_Address_Resolution_Enable_Complete()       			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? CLEAR_RESOLVING_LIST : ADDRESS_RESOLUTION;
}


/****************************************************************/
/* LE_Clear_Resolving_List_Complete()        					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? READ_RESOLVING_LIST_SIZE : CLEAR_RESOLVING_LIST;
}


/****************************************************************/
/* LE_Read_Resolving_List_Size_Complete()      					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Resolving_List_Size_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Resolving_List_Size )
{
	ControllerResolvingListSize = Resolving_List_Size;
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? SET_RPA_TIMEOUT : READ_RESOLVING_LIST_SIZE;
}


/****************************************************************/
/* LE_Set_Resolvable_Private_Address_Timeout_Complete()  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Resolvable_Private_Address_Timeout_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? BLE_INIT_DONE : SET_RPA_TIMEOUT;
}


/****************************************************************/
/* Set_Event_Mask_Complete()      								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? SET_LE_EVENT_MASK : SET_EVENT_MASK;
}


/****************************************************************/
/* LE_Set_Event_Mask_Complete()      							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? LE_READ_BUFFER_SIZE : SET_LE_EVENT_MASK;
}


/****************************************************************/
/* LE_Read_Buffer_Size_Complete()      							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Buffer_Size_Complete( CONTROLLER_ERROR_CODES Status, uint16_t LE_ACL_Data_Packet_Length,
		uint8_t Total_Num_LE_ACL_Data_Packets )
{
	if( Status == COMMAND_SUCCESS )
	{
		LE_ACL_Data_Packet_Length_Supported = LE_ACL_Data_Packet_Length;
		Total_Num_LE_ACL_Data_Packets_Supported = Total_Num_LE_ACL_Data_Packets;
		if( ( !LE_ACL_Data_Packet_Length_Supported ) || ( !Total_Num_LE_ACL_Data_Packets_Supported ) )
		{
			BLEInitSteps = READ_BUFFER_SIZE;
		}else
		{
			BLEInitSteps = LE_LOCAL_SUPPORTED_FEATURES;
		}
	}else
	{
		BLEInitSteps = LE_READ_BUFFER_SIZE;
	}
}


/****************************************************************/
/* LE_Read_Local_Supported_Features_Complete()    		  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status, LE_SUPPORTED_FEATURES* LE_Features )
{
	if( Status == COMMAND_SUCCESS )
	{
		HCI_LE_Features = *LE_Features;
		BLEInitSteps = READ_BD_ADDRESS;
	}else
	{
		BLEInitSteps = LE_LOCAL_SUPPORTED_FEATURES;
	}
}


/****************************************************************/
/* Read_BD_ADDR_Complete()      								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_BD_ADDR_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* BD_ADDR )
{
	if( Status == COMMAND_SUCCESS )
	{
		if( memcmp( Get_Public_Device_Address( ).Ptr, BD_ADDR, sizeof(BD_ADDR_TYPE) ) == 0 )
		{
			BLEInitSteps = GENERATE_STATIC_RANDOM_ADDRESS;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
	}else
	{
		BLEInitSteps = READ_BD_ADDRESS;
	}
}


/****************************************************************/
/* Reset_Controller()        	        						*/
/* Location: 					 								*/
/* Purpose: Reset Link Layer at controller side.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Reset_Controller( void )
{
	uint8_t status = FALSE;

	if( Controller_Reset_Flag == BLE_TRUE )
	{
		status = TRUE;
	}else if( ( Controller_Reset_Flag != BLE_FALSE ) && ( Get_Config_Step() == CONFIG_FREE ) )
	{
		Controller_Reset_Flag = BLE_FALSE;
		Set_Config_Step( CONFIG_BLOCKED );
		HCI_Reset( &Reset_Complete, NULL );
	}

	return (status);
}


/****************************************************************/
/* Reset_Complete()        	       								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Reset_Complete( CONTROLLER_ERROR_CODES Status )
{
	Controller_Reset_Flag = ( Status == COMMAND_SUCCESS ) ? BLE_TRUE : BLE_ERROR;
}


/****************************************************************/
/* Read_Local_Version_Information_Complete() 					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Version_Information_Complete( CONTROLLER_ERROR_CODES Status,
		LOCAL_VERSION_INFORMATION* Local_Version_Information )
{
	if( Status == COMMAND_SUCCESS )
	{
		LocalInfo = *Local_Version_Information;
		BLEInitSteps = CLEAR_WHITE_LIST;
	}else
	{
		BLEInitSteps = READ_LOCAL_VERSION;
	}
}


/****************************************************************/
/* Hal_Device_Standby_Event()        	    	   				*/
/* Location: 					 								*/
/* Purpose: Called to indicate the status of standby command.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_Device_Standby_Event( CONTROLLER_ERROR_CODES Status )
{
	if( Status == COMMAND_SUCCESS )
	{
		Standby_Flag = BLE_TRUE;
	}else
	{
		Standby_Flag = BLE_ERROR;
	}
}


/****************************************************************/
/* Enter_StandBy_Mode()        	 								*/
/* Location: 					 								*/
/* Purpose: Put the controller in stand-by mode.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Enter_StandBy_Mode( void )
{
	switch( BLEState )
	{
	case ADVERTISING_STATE:
	case SCANNING_STATE:
	case INITIATING_STATE:
	case CONNECTION_STATE:
	case SYNCHRONIZATION_STATE:
	case ISOCHRONOUS_BROADCASTING_STATE:
		/* TODO: algumas funções tem máquinas de estado para transição, ou seja, estas funções
		 * devem ir do início ao fim, caso contrário a próxima chamada de função vai "zoar" com a mesma
		 * Por isso, evaliar quais são as funções e esperar as mesmas terminarem ou prover uma maneira
		 * de resetar estas funções por aqui antes de colocar o sistema em stand-by. */
		Standby_Flag = BLE_FALSE;
		Set_BLE_State( STANDBY_STATE );
		return (TRUE);
		break;

	default:
		break;

	}

	return (FALSE);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
