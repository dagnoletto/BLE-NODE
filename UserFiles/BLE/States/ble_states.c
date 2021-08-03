

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
	ADD_IDENTITY_ADDRESS,
	SET_RPA_TIMEOUT,
	CLEAR_TIMER,
	WAIT_STATUS,
	BLE_INIT_DONE
}BLE_INIT_STEPS;


typedef enum
{
	DISABLE_ADVERTISING,
	VERIFY_ADDRESS,
	GENERATE_NON_RESOLVABLE_ADDRESS,
	SET_RANDOM_ADDRESS,
	SET_ADV_PARAMETERS,
	LOAD_ADV_DATA,
	READ_ADV_POWER,
	SET_ADV_POWER,
	SET_ADV_DATA,
	SET_SCAN_RSP_DATA,
	ENABLE_ADVERTISING,
	END_ADV_CONFIG,
	FAILED_ADV_CONFIG,
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
static int8_t Advertising_Config( void );
static ADV_CONFIG Update_Random_Address( void );
static ADV_CONFIG Check_Local_IRK( RESOLVING_RECORD* ResolvingRecord, uint8_t RPAInController );
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address );
static void Advertising( void );
static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Advertising_Parameters_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( CONTROLLER_ERROR_CODES Status, int8_t TX_Power_Level );
static void LE_Set_Data_Complete( CONTROLLER_ERROR_CODES Status );
static void Vendor_Specific_Init_CallBack( void* ConfigData );
static void Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status );
static void Clear_White_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Read_Resolving_List_Size_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Resolving_List_Size );
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Resolvable_Private_Address_Timeout_Complete( CONTROLLER_ERROR_CODES Status );
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
static ADV_CONFIG AdvConfig = { DISABLE_ADVERTISING, DISABLE_ADVERTISING };
static uint8_t ControllerResolvingListSize;
static uint16_t SM_Resolving_List_Index;
static BD_ADDR_TYPE RandomAddress;


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
	if( Get_BLE_State( ) == STANDBY_STATE )
	{
		if( Check_Advertising_Parameters( AdvPar )  )
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
			if( Generate_Device_Address( &HCI_Supported_Commands, STATIC_DEVICE_ADDRESS, NULL ) != NULL )
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
		SM_Resolving_List_Index = 0;
		break;

	case ADD_IDENTITY_ADDRESS:
		/* Check if we have bonded devices to add to the resolving list */
		if ( ( ControllerResolvingListSize > SM_Resolving_List_Index ) && ( SM_Resolving_List_Index < Get_Number_Of_Resolving_Records() ) )
		{
			DEVICE_IDENTITY* DevId = Get_Record_From_Index( SM_Resolving_List_Index );
			BLEInitSteps = HCI_LE_Add_Device_To_Resolving_List( DevId->Peer_Identity_Address.Type, DevId->Peer_Identity_Address.Address,
					&(DevId->Peer_IRK), &(DevId->Local_IRK), &LE_Add_Device_To_Resolving_List_Complete, NULL ) ? CLEAR_TIMER : ADD_IDENTITY_ADDRESS;
		}else
		{
			BLEInitSteps = SET_RPA_TIMEOUT;
		}
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
/* Advertising_Config()        	   								*/
/* Location: 					 								*/
/* Purpose: Configure advertising type							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static int8_t Advertising_Config( void )
{
	static uint32_t AdvConfigTimeout = 0;
	IDENTITY_ADDRESS PeerId;
	RESOLVING_RECORD* Ptr;

	switch( AdvConfig.Actual )
	{
	case DISABLE_ADVERTISING:
		AdvConfigTimeout = 0;
		AdvertisingParameters->Counter = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Advertising_Enable )
		{
			AdvConfig.Actual = HCI_LE_Set_Advertising_Enable( FALSE, &LE_Set_Advertising_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADVERTISING;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? VERIFY_ADDRESS : AdvConfig.Actual;
			AdvConfig.Prev = DISABLE_ADVERTISING;
		}
		break;

	case VERIFY_ADDRESS:
	{
		switch ( AdvertisingParameters->Own_Address_Type )
		{
		case OWN_RANDOM_DEV_ADDR:
			AdvConfig.Actual = Update_Random_Address( ).Actual;
			break;

		case OWN_RESOL_OR_PUBLIC_ADDR:
		case OWN_RESOL_OR_RANDOM_ADDR:
			PeerId.Type = AdvertisingParameters->Peer_Address_Type;
			PeerId.Address = AdvertisingParameters->Peer_Address;
			Ptr = Get_Record_From_Peer_Identity( &PeerId );

			if ( Get_Local_Version_Information()->HCI_Version > CORE_SPEC_4_1 )
			{
				/* Above version 4.1, the controller generates the addresses.
				 * However, we still have to check if local IRK is not null because controller does'nt have
				 * the local identity to do it. */
				AdvConfig.Actual = ( Ptr != NULL ) ? Check_Local_IRK( Ptr, TRUE ).Actual : SET_ADV_PARAMETERS;
				/* TODO: here we assume the controller automatically loads the address based on the list and
				 * we don't need to retrieve the address using HCI_LE_Read_Local_Resolvable_Address command. */
			}else
			{
				/* For lower versions, the host must generate the resolvable address */
				/* Check if this device identity is in the Host resolving list */
				if ( Ptr != NULL )
				{
					AdvConfig.Actual = Check_Local_IRK( Ptr, FALSE ).Actual;
				}else
				{
					if( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR )
					{
						/* It is NOT on the list, we shall use the public address. */
						AdvertisingParameters->Own_Address_Type = OWN_PUBLIC_DEV_ADDR;
						AdvConfig.Actual = SET_ADV_PARAMETERS;
					}else
					{
						/* It is NOT on the list, we shall use the random address from LE_Set_Random_Address. */
						AdvertisingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
						AdvConfig.Actual = Update_Random_Address( ).Actual;
					}
				}
			}
			break;

			/* The default is public device address */
		case OWN_PUBLIC_DEV_ADDR:
		default:
			AdvConfig.Actual = SET_ADV_PARAMETERS;
			break;
		}
	}
	break;

	case GENERATE_NON_RESOLVABLE_ADDRESS:
	{
		BD_ADDR_TYPE* Ptr = Generate_Device_Address( &HCI_Supported_Commands, NON_RESOLVABLE_PRIVATE, NULL );
		if ( Ptr != NULL )
		{
			RandomAddress = *Ptr;
			AdvConfig.Actual = SET_RANDOM_ADDRESS;
		}
	}
	break;

	case SET_RANDOM_ADDRESS:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Random_Address )
		{
			AdvConfig.Actual = HCI_LE_Set_Random_Address( RandomAddress, &LE_Set_Random_Address_Complete, NULL ) ? WAIT_OPERATION : SET_RANDOM_ADDRESS;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? SET_ADV_PARAMETERS : AdvConfig.Actual;
		}
		break;

		//TODO: inserir um novo estado aqui para ajustar o peer address quando necessário!!!

	case SET_ADV_PARAMETERS:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Advertising_Parameters )
		{
			AdvConfig.Actual = HCI_LE_Set_Advertising_Parameters( AdvertisingParameters->Advertising_Interval_Min, AdvertisingParameters->Advertising_Interval_Max, AdvertisingParameters->Advertising_Type,
					AdvertisingParameters->Own_Address_Type, AdvertisingParameters->Peer_Address_Type, AdvertisingParameters->Peer_Address,
					AdvertisingParameters->Advertising_Channel_Map, AdvertisingParameters->Advertising_Filter_Policy, &LE_Set_Advertising_Parameters_Complete, NULL ) ? WAIT_OPERATION : SET_ADV_PARAMETERS;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? LOAD_ADV_DATA : AdvConfig.Actual;
		}
		break;

	case LOAD_ADV_DATA:
		Set_Advertising_HostData( AdvertisingParameters );
		if( ( AdvertisingParameters->HostData.Adv_Data_Length <= Get_Max_Advertising_Data_Length() )
				&& ( AdvertisingParameters->HostData.ScanRsp_Data_Length <= Get_Max_Scan_Response_Data_Length() ) )
		{
			AdvConfig.Actual = READ_ADV_POWER;
		}else
		{
			AdvConfig.Actual = FAILED_ADV_CONFIG;
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
			AdvConfig.Actual = HCI_LE_Set_Advertising_Data( AdvertisingParameters->HostData.Adv_Data_Length, AdvertisingParameters->HostData.Adv_Data_Ptr, &LE_Set_Data_Complete, NULL ) ? WAIT_OPERATION : SET_ADV_DATA;
			AdvConfig.Next = ( AdvConfig.Actual == WAIT_OPERATION ) ? SET_SCAN_RSP_DATA : AdvConfig.Actual;
			AdvConfig.Prev = SET_ADV_DATA;
		}
		break;

	case SET_SCAN_RSP_DATA:
		AdvConfigTimeout = 0;
		if( HCI_Supported_Commands.Bits.HCI_LE_Set_Scan_Response_Data )
		{
			AdvConfig.Actual = HCI_LE_Set_Scan_Response_Data( AdvertisingParameters->HostData.ScanRsp_Data_Length, AdvertisingParameters->HostData.Scan_Data_Ptr, &LE_Set_Data_Complete, NULL ) ? WAIT_OPERATION : SET_SCAN_RSP_DATA;
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

	case FAILED_ADV_CONFIG:
		AdvConfig.Actual = DISABLE_ADVERTISING;
		free(AdvertisingParameters);
		AdvertisingParameters = NULL;
		return (-1); /* Failed condition */
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
/* Update_Random_Address()        	   							*/
/* Location: 					 								*/
/* Purpose: Update random address based on advertising 			*/
/* parameters.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static ADV_CONFIG Update_Random_Address( void )
{
	ADV_CONFIG AdvStep;

	if( AdvertisingParameters->Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE )
	{
		AdvStep.Actual = GENERATE_NON_RESOLVABLE_ADDRESS;
	}else if( AdvertisingParameters->Own_Random_Address_Type == STATIC_DEVICE_ADDRESS )
	{
		RandomAddress = *( Get_Static_Random_Device_Address( ).Ptr );
		AdvStep.Actual = SET_RANDOM_ADDRESS;
	}else
	{
		AdvStep.Actual = FAILED_ADV_CONFIG;
	}

	return ( AdvStep );
}


/****************************************************************/
/* Check_Local_IRK()        	   								*/
/* Location: Page 3023 Core_v5.2								*/
/* Purpose: Test if the local IRK is null.			 			*/
/* parameters.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:	If the Host, when populating the resolving 		*/
/* list, sets a local IRK to all zeros, then any local address 	*/
/* used within an advertising physical channel PDU shall use 	*/
/* the local Identity Address, which is provided by the Host.	*/
/****************************************************************/
static ADV_CONFIG Check_Local_IRK( RESOLVING_RECORD* ResolvingRecord, uint8_t RPAInController )
{
	ADV_CONFIG AdvStep;
	uint8_t LocalIRKNull = TRUE; /* Consider null at first */

	for ( uint8_t i = 0; i < sizeof(ResolvingRecord->Peer.Local_IRK); i++ )
	{
		if ( ResolvingRecord->Peer.Local_IRK.Bytes[i] != 0 )
		{
			LocalIRKNull = FALSE; /* The IRK is not null */
			break;
		}
	}

	if ( LocalIRKNull )
	{
		/* The local IRK is null, we shall use the local identity instead. */
		if ( ResolvingRecord->Local_Identity_Address.Type == PEER_RANDOM_DEV_ADDR )
		{
			/* Random static local identity */
			AdvertisingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
			AdvertisingParameters->Own_Random_Address_Type = STATIC_DEVICE_ADDRESS;
			AdvStep.Actual = Update_Random_Address( ).Actual;
		}else
		{
			/* Public identity address */
			AdvertisingParameters->Own_Address_Type = OWN_PUBLIC_DEV_ADDR;
			AdvStep.Actual = SET_ADV_PARAMETERS;
		}
	}else if( !RPAInController )
	{
		/* Make it random for controller 4.1 compatibility */
		/* RPAInController - Resolvable Private Address in controller is not available, call this function to load
		 * the RPA. */
		AdvertisingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
		/* We should use RPA generated by the controller. */
		AdvStep.Actual = HCI_LE_Read_Local_Resolvable_Address( ResolvingRecord->Peer.Peer_Identity_Address.Type,
				ResolvingRecord->Peer.Peer_Identity_Address.Address, &Read_Local_Resolvable_Address_Complete, NULL ) ? WAIT_OPERATION : FAILED_ADV_CONFIG;
	}else
	{
		AdvStep.Actual = SET_ADV_PARAMETERS;
	}

	return (AdvStep);
}


/****************************************************************/
/* Read_Local_Resolvable_Address_Complete()        	   			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? SET_RANDOM_ADDRESS : FAILED_ADV_CONFIG;
	RandomAddress = *Local_Resolvable_Address;
}


/****************************************************************/
/* Advertising()        	   									*/
/* Location: 					 								*/
/* Purpose: Advertising											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Advertising( void )
{
	/* In privacy-enabled Peripheral, the Host shall set a timer equal to
	 * TGAP(private_addr_int). The Host shall generate a new resolvable
	 * private address or non-resolvable private address when the timer
	 * TGAP(private_addr_int) expires. */
	if( AdvertisingParameters->Privacy )
	{
		if( TimeBase_DelayMs( &AdvertisingParameters->Counter, TGAP_PRIVATE_ADDR_INT, TRUE ) )
		{
			Set_BLE_State( CONFIG_ADVERTISING );
		}
	}
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
/* LE_Set_Random_Address_Complete()        	 					*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? AdvConfig.Next : SET_RANDOM_ADDRESS;
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
		Tx_Power_Level_Type* Ptr;
		uint8_t* DataPtr = AdvertisingParameters->HostData.Adv_Data_Ptr;
		int16_t DataSize = AdvertisingParameters->HostData.Adv_Data_Length;


		do
		{
			Ptr = Get_AD_Type_Ptr( TX_POWER_LEVEL_TYPE, DataPtr, DataSize );
			if( Ptr != NULL )
			{
				Ptr->Tx_Power_Level = TX_Power_Level;
				DataPtr = (uint8_t*)( (uint32_t)( Ptr ) + (Ptr->length) + 1 ); /* Points to the next structure */
				DataSize = AdvertisingParameters->HostData.Adv_Data_Length - ( (uint32_t)( DataPtr ) - (uint32_t)( AdvertisingParameters->HostData.Adv_Data_Ptr ) );
				if( DataSize < sizeof( Tx_Power_Level_Type ) )
				{
					Ptr = NULL;
				}
			}
		}while( Ptr != NULL );


		DataPtr = AdvertisingParameters->HostData.Scan_Data_Ptr;
		DataSize = AdvertisingParameters->HostData.ScanRsp_Data_Length;


		do
		{
			Ptr = Get_AD_Type_Ptr( TX_POWER_LEVEL_TYPE, DataPtr, DataSize );
			if( Ptr != NULL )
			{
				Ptr->Tx_Power_Level = TX_Power_Level;
				DataPtr = (uint8_t*)( (uint32_t)( Ptr ) + (Ptr->length) + 1 ); /* Points to the next structure */
				DataSize = AdvertisingParameters->HostData.ScanRsp_Data_Length - ( (uint32_t)( DataPtr ) - (uint32_t)( AdvertisingParameters->HostData.Scan_Data_Ptr ) );
				if( DataSize < sizeof( Tx_Power_Level_Type ) )
				{
					Ptr = NULL;
				}
			}
		}while( Ptr != NULL );
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
	BLEInitSteps = ( Status == COMMAND_SUCCESS ) ? ADD_IDENTITY_ADDRESS : READ_RESOLVING_LIST_SIZE;
}


/****************************************************************/
/* LE_Add_Device_To_Resolving_List_Complete()      				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	if ( Status == COMMAND_SUCCESS )
	{
		SM_Resolving_List_Index++;
	}
	BLEInitSteps = ADD_IDENTITY_ADDRESS;
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
			ConfigDataPtr->Public_address = *( Get_Public_Device_Address( ).Ptr );
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
				if( memcmp( &ConfigDataPtr->Public_address, Get_Public_Device_Address( ).Ptr, sizeof(BD_ADDR_TYPE) ) == 0 )
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
