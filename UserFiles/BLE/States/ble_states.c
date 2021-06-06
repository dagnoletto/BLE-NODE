

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
	CLEAR_TIMER,
	WAIT_STATUS,
	BLE_INIT_DONE
}BLE_INIT_STEPS;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Reset_Controller( void );
static uint8_t Vendor_Specific_Init( void );
static uint8_t BLE_Init( void );
static void Vendor_Specific_Init_CallBack( void* ConfigData );


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
	switch( BLEState )
	{
	case RESET_CONTROLLER:
		if( Reset_Controller(  ) )
		{
			BLEState = VENDOR_SPECIFIC_INIT;
		}
		break;

	case VENDOR_SPECIFIC_INIT:
		if( Vendor_Specific_Init(  ) )
		{
			BLEState = BLE_INITIAL_SETUP;
		}
		break;

	case BLE_INITIAL_SETUP:
		if( BLE_Init(  ) )
		{
			Standby_Flag = BLE_FALSE;
			BLEState = STANDBY_STATE;
		}
		break;

	case STANDBY_STATE:
		if( Standby_Flag != BLE_TRUE )
		{
			if( ACI_Hal_Device_Standby(  ) )
			{
				Standby_Flag = BLE_TRUE;
			}
		}
		break;

	case ADVERTISING_STATE:
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
		BLEInitSteps = HCI_Read_Local_Supported_Commands( ) ? CLEAR_TIMER : LOCAL_SUPPORTED_COMMANDS;
		break;

	case LOCAL_SUPPORTED_FEATURES:
		memset( &HCI_LMP_Features, 0, sizeof(HCI_LMP_Features) );
		if( HCI_Supported_Commands.Bits.HCI_Read_Local_Supported_Features )
		{
			BLEInitSteps = HCI_Read_Local_Supported_Features( ) ? CLEAR_TIMER : LOCAL_SUPPORTED_FEATURES;
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

			BLEInitSteps = HCI_Set_Event_Mask( Event_Mask ) ? CLEAR_TIMER : SET_EVENT_MASK;
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

			BLEInitSteps = HCI_LE_Set_Event_Mask( LE_Event_Mask ) ? CLEAR_TIMER : SET_LE_EVENT_MASK;
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
			BLEInitSteps = HCI_LE_Read_Buffer_Size( ) ? CLEAR_TIMER : LE_READ_BUFFER_SIZE;
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
			BLEInitSteps = HCI_LE_Read_Local_Supported_Features( ) ? CLEAR_TIMER : LE_LOCAL_SUPPORTED_FEATURES;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case READ_BD_ADDRESS:
		if( HCI_Supported_Commands.Bits.HCI_Read_BD_ADDR )
		{
			BLEInitSteps = HCI_Read_BD_ADDR(  ) ? CLEAR_TIMER : READ_BD_ADDRESS;
		}else
		{
			BLEInitSteps = CLEAR_TIMER;
		}
		break;

	case GENERATE_RANDOM_ADDRESS:
		if( Generate_Device_Addresses( &HCI_Supported_Commands, TRUE ) )
		{
			BLEInitSteps = BLE_INIT_DONE;
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
/* HCI_Read_Local_Supported_Commands_Complete()      			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Read_Local_Supported_Commands_Complete( CONTROLLER_ERROR_CODES Status, SUPPORTED_COMMANDS* Supported_Commands )
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
/* HCI_Read_Local_Supported_Features_Complete()      			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status, SUPPORTED_FEATURES* LMP_Features )
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
/* HCI_Set_Event_Mask_Complete()      							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? SET_LE_EVENT_MASK : SET_EVENT_MASK;
}


/****************************************************************/
/* HCI_LE_Set_Event_Mask_Complete()      						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_LE_Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status )
{
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? LE_READ_BUFFER_SIZE : SET_LE_EVENT_MASK;
}


/****************************************************************/
/* HCI_LE_Read_Buffer_Size_Complete()      						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_LE_Read_Buffer_Size_Complete( CONTROLLER_ERROR_CODES Status, uint16_t LE_ACL_Data_Packet_Length,
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
/* HCI_LE_Read_Local_Supported_Features_Complete()      		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_LE_Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status, LE_SUPPORTED_FEATURES* LE_Features )
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
/* HCI_Read_BD_ADDR_Complete()      							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Read_BD_ADDR_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* BD_ADDR )
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
		HCI_Reset( );
	}

	return (status);
}


/****************************************************************/
/* HCI_Reset_Complete()        	       							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Reset_Complete( CONTROLLER_ERROR_CODES Status )
{
	Controller_Reset_Flag = ( Status == COMMAND_SUCCESS ) ? BLE_TRUE : BLE_ERROR;
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
/* ACI_Hal_Device_Standby_Event()        	       				*/
/* Location: 					 								*/
/* Purpose: Called to indicate the status of standby command.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void ACI_Hal_Device_Standby_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode )
{
	if( ( Event == COMMAND_COMPLETE ) && ( ErrorCode == COMMAND_SUCCESS ) )
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
