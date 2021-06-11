

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"


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
	GENERATE_RANDOM_ADDRESS,
	READ_LOCAL_VERSION,
	CLEAR_WHITE_LIST,
	CLEAR_TIMER,
	WAIT_STATUS,
	BLE_INIT_DONE
}BLE_INIT_STEPS;


typedef enum
{
	DISABLE_ADVERTISING,
	SET_ADV_PARAMETERS,
	READ_ADV_POWER,
	SET_ADV_POWER,
	SET_ADV_DATA,
	SET_SCAN_RSP_DATA,
	ENABLE_ADVERTISING,
	END_ADV_CONFIG,
	WAIT_OPERATION,
}ADV_CONFIG_STEPS;


typedef struct
{
	ADV_CONFIG_STEPS Actual;
	ADV_CONFIG_STEPS Next;
	ADV_CONFIG_STEPS Prev;
}ADV_CONFIG;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static void Set_BLE_State( BLE_STATES NewBLEState );
static uint8_t Reset_Controller( void );
static void Reset_Complete( CONTROLLER_ERROR_CODES Status );
static uint8_t Vendor_Specific_Init( void );
static uint8_t BLE_Init( void );
static uint8_t Advertising_Config( void );
static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Advertising_Parameters_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( CONTROLLER_ERROR_CODES Status, int8_t TX_Power_Level );
static void LE_Set_Data_Complete( CONTROLLER_ERROR_CODES Status );
static void Vendor_Specific_Init_CallBack( void* ConfigData );
static void Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status );
static void Clear_White_List_Complete( CONTROLLER_ERROR_CODES Status );
static void Read_Local_Version_Information_Complete( CONTROLLER_ERROR_CODES Status,
		HCI_VERSION HCI_Version, uint16_t HCI_Revision,
		uint8_t LMP_PAL_Version, uint16_t Manufacturer_Name,
		uint16_t LMP_PAL_Subversion);
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
static BLE_STATUS VS_Init_Done_Flag;
static BLE_STATUS Controller_Reset_Flag = BLE_ERROR;
static BLE_STATUS Standby_Flag = BLE_FALSE;
static SUPPORTED_COMMANDS HCI_Supported_Commands;
static SUPPORTED_FEATURES HCI_LMP_Features;
static LE_SUPPORTED_FEATURES HCI_LE_Features;
static uint16_t LE_ACL_Data_Packet_Length_Supported;
static uint8_t Total_Num_LE_ACL_Data_Packets_Supported;
static BLE_VERSION_INFO LocalInfo;
static ADVERTISING_PARAMETERS* AdvertisingParameters = NULL;
static int8_t Adv_TX_Power_Level;
static ADV_CONFIG AdvConfig = { DISABLE_ADVERTISING, DISABLE_ADVERTISING };


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
		if( Advertising_Config(  ) )
		{
			Set_BLE_State( ADVERTISING_STATE );
		}
		break;

	case ADVERTISING_STATE:
		Set_BLE_State( ADVERTISING_STATE ); //TODO: teste
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
static void Set_BLE_State( BLE_STATES NewBLEState )
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
BLE_VERSION_INFO* Get_Local_Version_Information( void )
{
	return ( &LocalInfo );
}


/****************************************************************/
/* Enter_Advertising_Mode()        	 							*/
/* Location: 					 								*/
/* Purpose: Put the controller in advertising mode or update	*/
/* parameters if already in advertising.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Enter_Advertising_Mode( ADVERTISING_PARAMETERS* AdvPar )
{
	BLE_STATES State = Get_BLE_State( );
	if( ( State == STANDBY_STATE ) || ( State == ADVERTISING_STATE ) )
	{
		if( ( AdvPar->Adv_Data_Length <= Get_Max_Advertising_Data_Length() )
				&& ( AdvPar->ScanRsp_Data_Length <= Get_Max_Scan_Response_Data_Length() ) )
		{
			if( AdvertisingParameters != NULL )
			{
				free(AdvertisingParameters);
				AdvertisingParameters = NULL;
			}

			AdvertisingParameters = malloc( sizeof(ADVERTISING_PARAMETERS) );

			if( AdvertisingParameters != NULL )
			{
				*AdvertisingParameters = *AdvPar;
				Set_BLE_State( CONFIG_ADVERTISING );
				return (TRUE);
			}
		}
	}

	return (FALSE);
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
			BLEInitSteps = HCI_Read_BD_ADDR( &Read_BD_ADDR_Complete, NULL ) ? CLEAR_TIMER : READ_BD_ADDRESS;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case GENERATE_RANDOM_ADDRESS:
		if( Generate_Device_Addresses( &HCI_Supported_Commands, Get_Default_IRK(), TRUE ) )
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
			BLEInitSteps = HCI_LE_Clear_White_List( &Clear_White_List_Complete, NULL ) ? CLEAR_TIMER: CLEAR_WHITE_LIST;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
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
/* Advertising_Config()        	   								*/
/* Location: 					 								*/
/* Purpose: Configure advertising type							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Advertising_Config( void )
{
	static uint32_t AdvConfigTimeout = 0;

	switch( AdvConfig.Actual )
	{
	case DISABLE_ADVERTISING:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Advertising_Enable )
		{
			AdvConfig.Actual = HCI_LE_Set_Advertising_Enable( FALSE, &LE_Set_Advertising_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADVERTISING;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? SET_ADV_PARAMETERS : AdvConfig.Actual;
			AdvConfig.Prev = DISABLE_ADVERTISING;
		}
		break;

	case SET_ADV_PARAMETERS:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Advertising_Parameters )
		{
			AdvConfig.Actual = HCI_LE_Set_Advertising_Parameters( AdvertisingParameters->Advertising_Interval_Min, AdvertisingParameters->Advertising_Interval_Max, AdvertisingParameters->Advertising_Type,
					AdvertisingParameters->Own_Address_Type, AdvertisingParameters->Peer_Address_Type, AdvertisingParameters->Peer_Address,
					AdvertisingParameters->Advertising_Channel_Map, AdvertisingParameters->Advertising_Filter_Policy, &LE_Set_Advertising_Parameters_Complete, NULL ) ? WAIT_OPERATION : SET_ADV_PARAMETERS;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? READ_ADV_POWER : AdvConfig.Actual;
		}
		break;

	case READ_ADV_POWER:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Read_Advertising_Physical_Channel_Tx_Power )
		{
			AdvConfig.Actual = HCI_LE_Read_Advertising_Physical_Channel_Tx_Power( &LE_Read_Advertising_Physical_Channel_Tx_Power_Complete, NULL ) ? WAIT_OPERATION : READ_ADV_POWER;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? SET_ADV_POWER : AdvConfig.Actual;
		}else
		{
			AdvConfig.Actual = SET_ADV_DATA;
		}
		break;

	case SET_ADV_POWER:
		/* So far there is no HCI standard command to adjust specifically the advertising power, this step is for the future */
		/* We can used the ACI_Hal_Set_Tx_Power_Level from vendor_specific_hci, but is for all channels, not only advertising */
		AdvConfig.Actual = SET_ADV_DATA;
		break;

	case SET_ADV_DATA:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Advertising_Data )
		{
			uint8_t AdvData[ AdvertisingParameters->Adv_Data_Length + 2 ];

			uint8_t AdvDataLen = Format_AD_Structure( &AdvData[0], sizeof(AdvData), AdvertisingParameters->Advertising_Type, AdvertisingParameters->Adv_Data_Ptr, AdvertisingParameters->Adv_Data_Length );

			AdvConfig.Actual = HCI_LE_Set_Advertising_Data( AdvDataLen, &AdvData[0], &LE_Set_Data_Complete, NULL ) ? WAIT_OPERATION : SET_ADV_DATA;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? SET_SCAN_RSP_DATA : AdvConfig.Actual;
			AdvConfig.Prev = SET_ADV_DATA;
		}
		break;

	case SET_SCAN_RSP_DATA:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Scan_Response_Data )
		{
			uint8_t ScanRspData[ AdvertisingParameters->ScanRsp_Data_Length + 2 ];

			uint8_t ScanRspDataLen = Format_AD_Structure( &ScanRspData[0], sizeof(ScanRspData), AdvertisingParameters->Advertising_Type, AdvertisingParameters->Scan_Data_Ptr, AdvertisingParameters->ScanRsp_Data_Length );

			AdvConfig.Actual = HCI_LE_Set_Scan_Response_Data( ScanRspDataLen, &ScanRspData[0], &LE_Set_Data_Complete, NULL ) ? WAIT_OPERATION : SET_SCAN_RSP_DATA;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? ENABLE_ADVERTISING : AdvConfig.Actual;
			AdvConfig.Prev = SET_SCAN_RSP_DATA;
		}
		break;

	case ENABLE_ADVERTISING:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Advertising_Enable )
		{
			AdvConfig.Actual = HCI_LE_Set_Advertising_Enable( TRUE, &LE_Set_Advertising_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADVERTISING;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? END_ADV_CONFIG : AdvConfig.Actual;
			AdvConfig.Prev = ENABLE_ADVERTISING;
		}
		break;

	case END_ADV_CONFIG:
		AdvConfig.Actual = DISABLE_ADVERTISING;
		return (TRUE);
		break;

	case WAIT_OPERATION:
		if( TimeBase_DelayMs( &AdvConfigTimeout, 500, TRUE ) )
		{
			AdvConfig.Actual = DISABLE_ADVERTISING;
			AdvConfig.Next = DISABLE_ADVERTISING;
		}
		break;

	default:
		break;
	}

	return (FALSE);
}


/****************************************************************/
/* LE_Set_Advertising_Enable_Complete()        	   				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? AdvConfig.Next : AdvConfig.Prev;
}


/****************************************************************/
/* LE_Set_Advertising_Parameters_Complete()        	 			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Advertising_Parameters_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? AdvConfig.Next : SET_ADV_PARAMETERS;
}


/****************************************************************/
/* LE_Read_Advertising_Physical_Channel_Tx_Power_Complete()    	*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( CONTROLLER_ERROR_CODES Status, int8_t TX_Power_Level )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? AdvConfig.Next : READ_ADV_POWER;
	if( Status == COMMAND_SUCCESS )
	{
		Adv_TX_Power_Level = TX_Power_Level;
	}
}


/****************************************************************/
/* LE_Set_Data_Complete()   					 				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Data_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? AdvConfig.Next : AdvConfig.Prev;
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
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Clear_White_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? BLE_INIT_DONE : CLEAR_WHITE_LIST;
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
		if( memcmp( Get_Public_Device_Address(), BD_ADDR, sizeof(BD_ADDR_TYPE) ) == 0 )
		{
			BLEInitSteps = GENERATE_RANDOM_ADDRESS;
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
		HCI_VERSION HCI_Version, uint16_t HCI_Revision,
		uint8_t LMP_PAL_Version, uint16_t Manufacturer_Name,
		uint16_t LMP_PAL_Subversion)
{
	if( Status == COMMAND_SUCCESS )
	{
		LocalInfo.HCI_Version = HCI_Version;
		LocalInfo.HCI_Revision = HCI_Revision;
		LocalInfo.LMP_PAL_Version = LMP_PAL_Version;
		LocalInfo.Manufacturer_Name = Manufacturer_Name;
		LocalInfo.LMP_PAL_Subversion = LMP_PAL_Subversion;

		BLEInitSteps = CLEAR_WHITE_LIST;
	}else
	{
		BLEInitSteps = READ_LOCAL_VERSION;
	}
}


/****************************************************************/
/* Vendor_Specific_Init()        	        					*/
/* Location: 					 								*/
/* Purpose: Await for vendor specific events and do vendor 		*/
/* specific configuration.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Vendor_Specific_Init( void )
{
	typedef enum
	{
		WRITE_CONFIG_DATA  = 0x00,
		VERIFY_CONFIG_DATA = 0x01,
		WAIT_CONFIG_END	   = 0x02,
		END_CONFIG_MODE	   = 0x03
	}INIT_STEPS;

	static uint8_t Result = FALSE;
	static INIT_STEPS InitSteps = WRITE_CONFIG_DATA;
	static CONFIG_DATA* ConfigDataPtr = NULL;

	switch ( InitSteps )
	{
	case WRITE_CONFIG_DATA:

		VS_Init_Done_Flag = BLE_FALSE;
		if( ConfigDataPtr != NULL )
		{
			free( ConfigDataPtr );
			ConfigDataPtr = NULL;
		}
		ConfigDataPtr = malloc( sizeof(CONFIG_DATA) );

		if( ConfigDataPtr != NULL )
		{
			ConfigDataPtr->Public_address = *( Get_Public_Device_Address() );
			ConfigDataPtr->LLWithoutHost = LL_ONLY;
			ConfigDataPtr->Role = SLAVE_AND_MASTER_12KB;

			if( Write_Config_Data( ConfigDataPtr, &Vendor_Specific_Init_CallBack ) == BLE_TRUE )
			{
				InitSteps = VERIFY_CONFIG_DATA;
			}else
			{
				free( ConfigDataPtr );
				ConfigDataPtr = NULL;
			}
		}
		break;

	case VERIFY_CONFIG_DATA:
		if( VS_Init_Done_Flag != BLE_FALSE )
		{
			if( VS_Init_Done_Flag == BLE_TRUE )
			{
				VS_Init_Done_Flag = BLE_FALSE;
				memset( ConfigDataPtr, 0, sizeof(CONFIG_DATA) ); /* Clear bytes */
				if( Read_Config_Data( ConfigDataPtr, &Vendor_Specific_Init_CallBack ) == BLE_TRUE )
				{
					InitSteps = WAIT_CONFIG_END;
				}else
				{
					if( ConfigDataPtr != NULL )
					{
						free( ConfigDataPtr );
						ConfigDataPtr = NULL;
					}
					InitSteps = WRITE_CONFIG_DATA;
				}
			}else
			{
				if( ConfigDataPtr != NULL )
				{
					free( ConfigDataPtr );
					ConfigDataPtr = NULL;
				}
				InitSteps = WRITE_CONFIG_DATA;
			}
		}
		break;

	case WAIT_CONFIG_END:
		if( VS_Init_Done_Flag != BLE_FALSE )
		{
			Result = FALSE;
			if( ConfigDataPtr != NULL )
			{
				if( memcmp( &ConfigDataPtr->Public_address, Get_Public_Device_Address(), sizeof(BD_ADDR_TYPE) ) == 0 )
				{
					/* TODO: If the public address was updated, we assume all other fields were updated as well */
					Result = TRUE;
				}
				free( ConfigDataPtr );
				ConfigDataPtr = NULL;
			}
			InitSteps = END_CONFIG_MODE;
		}
		break;

	case END_CONFIG_MODE:
		if ( Get_Config_Step() == CONFIG_FREE )
		{
			InitSteps = WRITE_CONFIG_DATA;
			return ( Result );
		}
		break;
	}

	return ( FALSE );
}


/****************************************************************/
/* Vendor_Specific_Init_CallBack()        	       				*/
/* Location: 					 								*/
/* Purpose: Called to indicate the status of configuration.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Vendor_Specific_Init_CallBack( void* ConfigData )
{
	if( ConfigData != NULL )
	{
		VS_Init_Done_Flag = BLE_TRUE;
	}else
	{
		VS_Init_Done_Flag = BLE_ERROR;
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
/* End of file	                                                */
/****************************************************************/
