

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"
#include "hosted_functions.h"
#include "ble_scanning.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	DISABLE_SCANNING,
	DISABLE_ADDRESS_RESOLUTION,
	CLEAR_RESOLVING_LIST,
	ADD_TO_RESOLVING_LIST,
	VERIFY_ADDRESS,
	//	WAIT_FOR_NEW_LOCAL_READ,
	//	GENERATE_NON_RESOLVABLE_ADDRESS,
	//	SET_RANDOM_ADDRESS,
	//	SET_PEER_ADDRESS,
	//	WAIT_FOR_NEW_PEER_READ,
	//	ENABLE_ADDRESS_RESOLUTION,
	//	SET_ADV_PARAMETERS,
	//	LOAD_ADV_DATA,
	//	READ_ADV_POWER,
	//	SET_ADV_POWER,
	//	SET_ADV_DATA,
	//	SET_SCAN_RSP_DATA,
	//	ENABLE_ADVERTISING,
	//	END_ADV_CONFIG,
	//	FAILED_ADV_CONFIG,
	WAIT_OPERATION,
}SCAN_CONFIG_STEPS;


typedef struct
{
	SCAN_CONFIG_STEPS Actual;
	SCAN_CONFIG_STEPS Next;
	SCAN_CONFIG_STEPS Prev;
}SCAN_CONFIG;


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
int8_t Scanning_Config( void );
void Scanning( void );
//static ADV_CONFIG Update_Random_Address( void );
//static ADV_CONFIG Check_Local_IRK( RESOLVING_RECORD* ResolvingRecord, uint8_t RPAInController );
//static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address );
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
//static void Read_Peer_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Peer_Resolvable_Address );
static void LE_Set_Scan_Enable_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Set_Advertising_Parameters_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( CONTROLLER_ERROR_CODES Status, int8_t TX_Power_Level );
//static void LE_Set_Data_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static uint8_t Check_Scanner_Parameters( SCANNING_PARAMETERS* ScanPar );
static uint8_t Check_Random_Address_For_Scanning( SCANNING_PARAMETERS* ScanPar );
static uint8_t Check_Local_Resolvable_Private_Address( IDENTITY_ADDRESS* Peer_Identity_Address );


/****************************************************************/
/* extern functions declaration                                 */
/****************************************************************/
extern void Set_BLE_State( BLE_STATES NewBLEState );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static SCANNING_PARAMETERS* ScanningParameters = NULL;
static SCAN_CONFIG ScanConfig = { DISABLE_SCANNING, DISABLE_SCANNING, DISABLE_SCANNING };
static BD_ADDR_TYPE RandomAddress;
static uint16_t SM_Resolving_List_Index;


/****************************************************************/
/* Enter_Scanning_Mode()        	 							*/
/* Location: 					 								*/
/* Purpose: Put the controller in scanning mode.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Enter_Scanning_Mode( SCANNING_PARAMETERS* ScanPar )
{
	if( Get_BLE_State( ) == STANDBY_STATE )
	{
		if( Check_Scanning_Parameters( ScanPar )  )
		{
			if( ScanningParameters != NULL )
			{
				free(ScanningParameters);
				ScanningParameters = NULL;
			}

			ScanningParameters = malloc( sizeof(SCANNING_PARAMETERS) );

			if( ScanningParameters != NULL )
			{
				*ScanningParameters = *ScanPar;
				Set_BLE_State( CONFIG_SCANNING );
				return (TRUE);
			}
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Scanning_Config()        	   								*/
/* Location: 					 								*/
/* Purpose: Configure scanning type.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
int8_t Scanning_Config( void )
{
	static uint32_t ScanConfigTimeout = 0;
	static RESOLVING_RECORD* RecordPtr;

	switch( ScanConfig.Actual )
	{
	case DISABLE_SCANNING:
		RecordPtr = NULL;
		ScanConfigTimeout = 0;
		ScanningParameters->Counter = 0;
		ScanConfig.Next = DISABLE_ADDRESS_RESOLUTION;
		ScanConfig.Prev = DISABLE_SCANNING;
		ScanConfig.Actual = HCI_LE_Set_Scan_Enable( FALSE, FALSE, &LE_Set_Scan_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_SCANNING;
		break;

	case DISABLE_ADDRESS_RESOLUTION:
		ScanConfig.Next = CLEAR_RESOLVING_LIST;
		ScanConfig.Prev = DISABLE_ADDRESS_RESOLUTION;
		ScanConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( FALSE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADDRESS_RESOLUTION;
		break;

	case CLEAR_RESOLVING_LIST:
		if( ( ScanningParameters->Privacy ) || ( ScanningParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) ||
				( ScanningParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) ||
				( ( ScanningParameters->Own_Address_Type == OWN_RANDOM_DEV_ADDR ) && ( ScanningParameters->Own_Random_Address_Type == RESOLVABLE_PRIVATE ) ) )
		{
			/* Check if this peer device is in the resolving list */
			RecordPtr = Get_Record_From_Peer_Identity( &ScanningParameters->PeerId );

			SM_Resolving_List_Index = 0;
			ScanConfig.Actual = HCI_LE_Clear_Resolving_List( &LE_Clear_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : CLEAR_RESOLVING_LIST;
		}else
		{
			ScanConfig.Actual = VERIFY_ADDRESS;
		}
		break;

	default:
		break;
	}

	return (FALSE);





	//	switch( AdvConfig.Actual )
	//		{

	/*
//		case DISABLE_ADVERTISING:
//			Ptr = NULL;
//			AdvertisingParameters->Own_Address_Type = AdvertisingParameters->Original_Own_Address_Type;
//			AdvertisingParameters->Own_Random_Address_Type = AdvertisingParameters->Original_Own_Random_Address_Type;
//			AdvertisingParameters->Peer_Address = AdvertisingParameters->Original_Peer_Address;
//			AdvConfigTimeout = 0;
//			AdvertisingParameters->Counter = 0;
//			AdvConfig.Next = DISABLE_ADDRESS_RESOLUTION;
//			AdvConfig.Prev = DISABLE_ADVERTISING;
//			AdvConfig.Actual = HCI_LE_Set_Advertising_Enable( FALSE, &LE_Set_Advertising_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADVERTISING;
//			break;
	 */

	/*
//
//		case DISABLE_ADDRESS_RESOLUTION:
//			AdvConfig.Next = CLEAR_RESOLVING_LIST;
//			AdvConfig.Prev = DISABLE_ADDRESS_RESOLUTION;
//			AdvConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( FALSE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADDRESS_RESOLUTION;
//			break;
	 */

	/*
//		case CLEAR_RESOLVING_LIST:
//			if( ( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) || ( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) )
//			{
//				IDENTITY_ADDRESS PeerId;
//
//				PeerId.Type = AdvertisingParameters->Peer_Address_Type;
//				PeerId.Address = AdvertisingParameters->Peer_Address;
//				Ptr = Get_Record_From_Peer_Identity( &PeerId );
//				if( Ptr != NULL )
//				{
//
//					SM_Resolving_List_Index = 0;
//					AdvConfig.Actual = HCI_LE_Clear_Resolving_List( &LE_Clear_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : CLEAR_RESOLVING_LIST;
//				}else
//				{
//
//					AdvConfig.Actual = VERIFY_ADDRESS;
//				}
//			}else
//			{
//				AdvConfig.Actual = VERIFY_ADDRESS;
//			}
//			break;
	 */

	//		case ADD_TO_RESOLVING_LIST:
	//			/* Check if we have bonded devices to add to the resolving list */
	//			if ( SM_Resolving_List_Index < Get_Number_Of_Resolving_Records() )
	//			{
	//				DEVICE_IDENTITY* DevId = &( Get_Record_From_Index( SM_Resolving_List_Index )->Peer );
	//				/* Here we add a device to the controller's resolving list if it exists in the Host's list */
	//				AdvConfig.Actual = HCI_LE_Add_Device_To_Resolving_List( DevId->Peer_Identity_Address.Type, DevId->Peer_Identity_Address.Address,
	//						&DevId->Peer_IRK, &DevId->Local_IRK, &LE_Add_Device_To_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : ADD_TO_RESOLVING_LIST;
	//			}else
	//			{
	//				AdvConfig.Actual = VERIFY_ADDRESS;
	//			}
	//			break;
	//
	//		case VERIFY_ADDRESS:
	//		{
	//			AdvConfigTimeout = 0;
	//			switch ( AdvertisingParameters->Own_Address_Type )
	//			{
	//			case OWN_RANDOM_DEV_ADDR:
	//				AdvConfig.Actual = Update_Random_Address( ).Actual;
	//				break;
	//
	//			case OWN_RESOL_OR_PUBLIC_ADDR:
	//			case OWN_RESOL_OR_RANDOM_ADDR:
	//				if ( Get_Local_Version_Information()->HCI_Version > CORE_SPEC_4_1 )
	//				{
	//					/* Above version 4.1, the controller generates the addresses.
	//					 * However, we still have to check if local IRK is not null because controller does'nt have
	//					 * the local identity to do it. */
	//					if( Ptr != NULL )
	//					{
	//						AdvConfig.Actual = Check_Local_IRK( Ptr, TRUE ).Actual;
	//					}else if( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR )
	//					{
	//						AdvConfig.Actual = SET_PEER_ADDRESS;
	//					}else
	//					{
	//						AdvConfig.Actual = Update_Random_Address( ).Actual;
	//					}
	//					/* TODO: here we assume the controller automatically loads the address based on the list and
	//					 * we don't need to retrieve the address using HCI_LE_Read_Local_Resolvable_Address command. */
	//				}else
	//				{
	//					/* For lower versions, the host must generate the resolvable address */
	//					/* Check if this device identity is in the Host resolving list */
	//					if ( Ptr != NULL )
	//					{
	//						AdvConfig.Actual = Check_Local_IRK( Ptr, FALSE ).Actual;
	//					}else
	//					{
	//						if( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR )
	//						{
	//							/* It is NOT on the list, we shall use the public address. */
	//							AdvertisingParameters->Own_Address_Type = OWN_PUBLIC_DEV_ADDR;
	//							AdvConfig.Actual = SET_PEER_ADDRESS;
	//						}else
	//						{
	//							/* It is NOT on the list, we shall use the random address from LE_Set_Random_Address. */
	//							AdvertisingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
	//							AdvConfig.Actual = Update_Random_Address( ).Actual;
	//						}
	//					}
	//				}
	//				break;
	//
	//				/* The default is public device address */
	//			case OWN_PUBLIC_DEV_ADDR:
	//			default:
	//				AdvConfig.Actual = SET_PEER_ADDRESS;
	//				break;
	//			}
	//		}
	//		break;
	//
	//		case WAIT_FOR_NEW_LOCAL_READ:
	//			AdvertisingParameters->Own_Address_Type = AdvertisingParameters->Original_Own_Address_Type;
	//			AdvertisingParameters->Peer_Address = AdvertisingParameters->Original_Peer_Address;
	//			if( TimeBase_DelayMs( &AdvConfigTimeout, 500, TRUE ) )
	//			{
	//				AdvConfig.Actual = VERIFY_ADDRESS;
	//			}
	//			break;
	//
	//		case GENERATE_NON_RESOLVABLE_ADDRESS:
	//		{
	//			BD_ADDR_TYPE* Ptr = Generate_Device_Address( Get_Supported_Commands(), NON_RESOLVABLE_PRIVATE, NULL, 2 );
	//			if ( Ptr != NULL )
	//			{
	//				RandomAddress = *Ptr;
	//				AdvConfig.Actual = SET_RANDOM_ADDRESS;
	//			}
	//		}
	//		break;
	//
	//		case SET_RANDOM_ADDRESS:
	//			AdvConfigTimeout = 0;
	//			AdvConfig.Next = SET_PEER_ADDRESS;
	//			AdvConfig.Actual = HCI_LE_Set_Random_Address( RandomAddress, &LE_Set_Random_Address_Complete, NULL ) ? WAIT_OPERATION : SET_RANDOM_ADDRESS;
	//			break;
	//
	//		case SET_PEER_ADDRESS:
	//			AdvConfigTimeout = 0;
	//			if( ( Get_Local_Version_Information()->HCI_Version > CORE_SPEC_4_1 ) && ( AdvertisingParameters->Own_Address_Type == AdvertisingParameters->Original_Own_Address_Type ) )
	//			{
	//				/* The Peer_Address_Type and Peer_Address already contains the peer's device identity
	//				 * that is going to be used by the controller to generate and use the peer's Resolvable Private
	//				 * Address. So, everything should be fine. No modification of the original own address was done */
	//				/* TODO: Here we assume the controller will generate a peer address equals to peer's device identity in case the peerIRK is
	//				 * set to all zeros. According to Page 3023 Core_v5.2:
	//				 * If the Host, when populating the resolving list, sets a peer IRK to all zeros, then the peer address used within an
	//				 * advertising physical channel PDU shall use the peer’s Identity Address, which is provided by the Host. */
	//				AdvConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
	//			}else if( ( AdvertisingParameters->Advertising_Type == ADV_DIRECT_IND_HIGH_DUTY || AdvertisingParameters->Advertising_Type == ADV_DIRECT_IND_LOW_DUTY ) &&
	//					( AdvertisingParameters->Original_Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR || AdvertisingParameters->Original_Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) )
	//			{
	//				/* Direct advertising is used, which means the peer address may be resolvable if the peer's identity is in the resolving list */
	//				if( Ptr != NULL )
	//				{
	//					/* Our hosted simulated function already deals with the condition stated in Page 3023 Core_v5.2:
	//					 * If the Host, when populating the resolving list, sets a peer IRK to all zeros, then the peer address used within an
	//					 * advertising physical channel PDU shall use the peer’s Identity Address, which is provided by the Host. TODO: However, we are
	//					 * not sure if versions of the controller above 4.1 do the same. We assume yes. */
	//					AdvConfig.Actual = HCI_LE_Read_Peer_Resolvable_Address( Ptr->Peer.Peer_Identity_Address.Type,
	//							Ptr->Peer.Peer_Identity_Address.Address, &Read_Peer_Resolvable_Address_Complete, NULL ) ? WAIT_OPERATION : WAIT_FOR_NEW_PEER_READ;
	//				}else
	//				{
	//					/* It is not on the list, so proceed with whatever peer address loaded */
	//					AdvConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
	//				}
	//			}else
	//			{
	//				AdvConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
	//			}
	//			break;
	//
	//		case WAIT_FOR_NEW_PEER_READ:
	//			if( TimeBase_DelayMs( &AdvConfigTimeout, 500, TRUE ) )
	//			{
	//				AdvConfig.Actual = SET_PEER_ADDRESS;
	//			}
	//			break;
	//
	//		case ENABLE_ADDRESS_RESOLUTION:
	//			if( ( Ptr != NULL ) && ( AdvertisingParameters->Privacy ) )
	//			{
	//				AdvConfig.Next = SET_ADV_PARAMETERS;
	//				AdvConfig.Prev = ENABLE_ADDRESS_RESOLUTION;
	//				AdvConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( TRUE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADDRESS_RESOLUTION;
	//			}else
	//			{
	//				AdvConfig.Actual = SET_ADV_PARAMETERS;
	//			}
	//			break;
	//
	//		case SET_ADV_PARAMETERS:
	//			AdvConfigTimeout = 0;
	//			AdvConfig.Next = LOAD_ADV_DATA;
	//			AdvConfig.Actual = HCI_LE_Set_Advertising_Parameters( AdvertisingParameters->Advertising_Interval_Min, AdvertisingParameters->Advertising_Interval_Max, AdvertisingParameters->Advertising_Type,
	//					AdvertisingParameters->Own_Address_Type, AdvertisingParameters->Peer_Address_Type, AdvertisingParameters->Peer_Address,
	//					AdvertisingParameters->Advertising_Channel_Map, AdvertisingParameters->Advertising_Filter_Policy, &LE_Set_Advertising_Parameters_Complete, NULL ) ? WAIT_OPERATION : SET_ADV_PARAMETERS;
	//			break;
	//
	//		case LOAD_ADV_DATA:
	//			Set_Advertising_HostData( AdvertisingParameters );
	//			if( ( AdvertisingParameters->HostData.Adv_Data_Length <= Get_Max_Advertising_Data_Length() )
	//					&& ( AdvertisingParameters->HostData.ScanRsp_Data_Length <= Get_Max_Scan_Response_Data_Length() ) )
	//			{
	//				AdvConfig.Actual = READ_ADV_POWER;
	//			}else
	//			{
	//				AdvConfig.Actual = FAILED_ADV_CONFIG;
	//			}
	//			break;
	//
	//		case READ_ADV_POWER:
	//			AdvConfigTimeout = 0;
	//			if( Get_Supported_Commands()->Bits.HCI_LE_Read_Advertising_Physical_Channel_Tx_Power )
	//			{
	//				AdvConfig.Next = SET_ADV_POWER;
	//				AdvConfig.Actual = HCI_LE_Read_Advertising_Physical_Channel_Tx_Power( &LE_Read_Advertising_Physical_Channel_Tx_Power_Complete, NULL ) ? WAIT_OPERATION : READ_ADV_POWER;
	//			}else
	//			{
	//				AdvConfig.Actual = SET_ADV_DATA;
	//			}
	//			break;
	//
	//		case SET_ADV_POWER:
	//			/* So far there is no HCI standard command to adjust specifically the advertising power, this step is for the future */
	//			/* We can used the ACI_Hal_Set_Tx_Power_Level from vendor_specific_hci, but is for all channels, not only advertising */
	//			AdvConfig.Actual = SET_ADV_DATA;
	//			break;
	//
	//		case SET_ADV_DATA:
	//			AdvConfigTimeout = 0;
	//			AdvConfig.Next = SET_SCAN_RSP_DATA;
	//			AdvConfig.Prev = SET_ADV_DATA;
	//			AdvConfig.Actual = HCI_LE_Set_Advertising_Data( AdvertisingParameters->HostData.Adv_Data_Length, AdvertisingParameters->HostData.Adv_Data_Ptr, &LE_Set_Data_Complete, NULL ) ? WAIT_OPERATION : SET_ADV_DATA;
	//			break;
	//
	//		case SET_SCAN_RSP_DATA:
	//			AdvConfigTimeout = 0;
	//			AdvConfig.Next = ENABLE_ADVERTISING;
	//			AdvConfig.Prev = SET_SCAN_RSP_DATA;
	//			AdvConfig.Actual = HCI_LE_Set_Scan_Response_Data( AdvertisingParameters->HostData.ScanRsp_Data_Length, AdvertisingParameters->HostData.Scan_Data_Ptr, &LE_Set_Data_Complete, NULL ) ? WAIT_OPERATION : SET_SCAN_RSP_DATA;
	//			break;
	//
	//		case ENABLE_ADVERTISING:
	//			AdvConfigTimeout = 0;
	//			AdvConfig.Next = END_ADV_CONFIG;
	//			AdvConfig.Prev = ENABLE_ADVERTISING;
	//			AdvConfig.Actual = HCI_LE_Set_Advertising_Enable( TRUE, &LE_Set_Advertising_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADVERTISING;
	//			break;
	//
	//		case END_ADV_CONFIG:
	//			AdvConfig.Actual = DISABLE_ADVERTISING;
	//			return (TRUE);
	//			break;
	//
	//		case FAILED_ADV_CONFIG:
	//			AdvConfig.Actual = DISABLE_ADVERTISING;
	//			free(AdvertisingParameters);
	//			AdvertisingParameters = NULL;
	//			return (-1); /* Failed condition */
	//			break;
	//
	//		case WAIT_OPERATION:
	//			if( TimeBase_DelayMs( &AdvConfigTimeout, 500, TRUE ) )
	//			{
	//				AdvConfig.Actual = DISABLE_ADVERTISING;
	//				AdvConfig.Next = DISABLE_ADVERTISING;
	//			}
	//			break;
	//
	//		default:
	//			break;
	//		}
	//
	//		return (FALSE);
}


/****************************************************************/
/* LE_Set_Scan_Enable_Complete()        	   					*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Scan_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	ScanConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? ScanConfig.Next : ScanConfig.Prev;
}


/****************************************************************/
/* LE_Set_Address_Resolution_Enable_Complete()    				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	ScanConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? ScanConfig.Next : ScanConfig.Prev;
}


/****************************************************************/
/* LE_Clear_Resolving_List_Complete()      						*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	ScanConfig.Actual = ADD_TO_RESOLVING_LIST;
}


/****************************************************************/
/* Update_Random_Address()        	   							*/
/* Location: 					 								*/
/* Purpose: Update random address based on scanning 			*/
/* parameters.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static SCAN_CONFIG Update_Random_Address( void )
{
	//TODO
	//	SCAN_CONFIG ScanStep;
	//
	//	if( ScanningParameters->Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE )
	//	{
	//		ScanStep.Actual = GENERATE_NON_RESOLVABLE_ADDRESS;
	//	}else if( ScanningParameters->Own_Random_Address_Type == STATIC_DEVICE_ADDRESS )
	//	{
	//		RandomAddress = *( Get_Static_Random_Device_Address( ).Ptr );
	//		ScanStep.Actual = SET_RANDOM_ADDRESS;
	//	}else
	//	{
	//		ScanStep.Actual = FAILED_ADV_CONFIG;
	//	}
	//
	//	return ( ScanStep );
}


/****************************************************************/
/* Scanning()        	   										*/
/* Location: 					 								*/
/* Purpose: Scanning											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Scanning( void )
{
	/* Privacy feature in a Central with Host-based privacy: The Host shall generate a resolvable
	 * private address using the ‘resolvable private address generation procedure’ as defined in
	 * Section 10.8.2.2 or non-resolvable private address procedure as defined in Section 10.8.2.1.
	 * The Host shall set a timer equal to TGAP(private_addr_int). The Host shall generate a
	 * new resolvable private address or non-resolvable private address when the
	 * timer TGAP(private_addr_int) expires. Note: TGAP(private_addr_int) timer need not be run
	 * if a Central is not scanning or connected. (Page 1389 Core_v5.2).
	 * During active scanning, a privacy enabled Central shall use a non-resolvable
	 * or resolvable private address. */

	/* Privacy feature in an Observer: During active scanning, a privacy enabled Observer shall use
	 * either a resolvable private address or non-resolvable private address.
	 * If Address Resolution is not supported or disabled in the Controller, the
	 * following applies to the Host: The Host shall generate a resolvable private
	 * address using the ‘resolvable private address generation procedure’ as defined
	 * in Section 10.8.2.2 or non-resolvable private address procedure as defined in
	 * Section 10.8.2.1. The Host shall set a timer equal to TGAP(private_addr_int).
	 * The Host shall generate a new resolvable private address or non-resolvable
	 * private address when the timer TGAP(private_addr_int) expires. The value of
	 * TGAP(private_addr_int) shall not be greater than 1 hour. Note: TGAP(private_addr_int)
	 * timer need not be run if an Observer is not scanning. (Page 1389 Core_v5.2). */

	/* If the Controller has address resolution and that is enabled, the RPA is
	 * automatically generated given the timeout for the controller. However, in versions
	 * lower that 4.1, this functionality does not exist so we need to generate again.
	 * For non-resolvable private address we shall generate since the controller will not do it. */

	/* TODO: Here we consider the address resolution is always enabled in scanning.
	 * Even for versions higher than 4.1, this function should be called if address resolution
	 * is disabled. We lay on the fact address resolution is always enabled in the controller
	 * for versions above 4.1. */

	if( ScanningParameters->Privacy && ( ScanningParameters->LE_Scan_Type == ACTIVE_SCANNING ) &&
			( ( ScanningParameters->Own_Address_Type == OWN_RANDOM_DEV_ADDR || ScanningParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) &&
					( ScanningParameters->Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE || ScanningParameters->Own_Random_Address_Type == RESOLVABLE_PRIVATE ) ) )
	{
		if( TimeBase_DelayMs( &ScanningParameters->Counter, TGAP_PRIVATE_ADDR_INT, TRUE ) )
		{
			Set_BLE_State( CONFIG_SCANNING );
		}
	}
}


/****************************************************************/
/* Check_Scanning_Parameters()      							*/
/* Location: 													*/
/* Purpose: Verify scanning parameters.					 		*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
uint8_t Check_Scanning_Parameters( SCANNING_PARAMETERS* ScanPar )
{
	if( ( ScanPar->LE_Scan_Window > ScanPar->LE_Scan_Interval ) || ( ScanPar->Filter_Duplicates > 1 ) )
	{
		/* LE_Scan_Window shall be less than or equal to LE_Scan_Interval */
		/* Filter_Duplicates shall be 0 or 1. */
		return (FALSE);
	}else if( Get_Local_Version_Information()->HCI_Version <= CORE_SPEC_4_1 )
	{
		/* For Core version 4.1 and lower, the concept of resolving private addresses in the
		 * controller is not present so the controller can not resolve the private addresses
		 * in the controller, making this configuration unavailable. That means if the controller
		 * receives an advertising address that matches the resolving list, the controller will not
		 * generate its own RPA to request the scan to the advertiser. Although this configuration only
		 * matters for active scanning, we avoid going further if the device does not support the
		 * parameter range. */
		if( ( ScanPar->Own_Address_Type > OWN_RANDOM_DEV_ADDR) || ( ScanPar->Scanning_Filter_Policy > 1 ) )
		{
			return (FALSE);
		}
	}

	switch( ScanPar->Role )
	{

	case OBSERVER:
	case CENTRAL:
		return( Check_Scanner_Parameters( ScanPar ) );
		break;

		/* Other roles are not allowed in scanning */
	default: break;

	}

	return (FALSE);
}


/****************************************************************/
/* Check_Scanner_Parameters()      								*/
/* Location: 													*/
/* Purpose: Verify scanning parameters for scanner.				*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Scanner_Parameters( SCANNING_PARAMETERS* ScanPar )
{
	switch( ScanPar->Own_Address_Type )
	{
	case OWN_PUBLIC_DEV_ADDR:
		/* During active scanning, a privacy enabled Central shall use a non-resolvable
		or resolvable private address. */
		return ( ( ( ScanPar->LE_Scan_Type == ACTIVE_SCANNING ) && ScanPar->Privacy ) ? FALSE : TRUE );
		break;

	case OWN_RANDOM_DEV_ADDR:
		return ( Check_Random_Address_For_Scanning( ScanPar ) );
		break;

	case OWN_RESOL_OR_PUBLIC_ADDR:
		/* Due to the fact the public address may be used for active scanning, no privacy is
		 * guaranteed under this condition. */
		return ( ( ( ScanPar->LE_Scan_Type == ACTIVE_SCANNING ) && ScanPar->Privacy ) ? FALSE : TRUE );
		break;

	case OWN_RESOL_OR_RANDOM_ADDR:
		/* If we are scanning actively with privacy, the privacy depends pretty much of how
		 * random part is configured. */
		if( ( ScanPar->LE_Scan_Type == ACTIVE_SCANNING ) && ScanPar->Privacy )
		{
			/* Check random part */
			return ( Check_Random_Address_For_Scanning( ScanPar ) );
		}else if( ScanPar->Own_Random_Address_Type == RESOLVABLE_PRIVATE )
		{
			/* Check resolvable private address is possible to generate */
			return ( Check_Local_Resolvable_Private_Address( &ScanPar->PeerId ) );
		}else
		{
			return (TRUE);
		}
		break;
	}
	return (FALSE);
}


/****************************************************************/
/* Check_Random_Address_For_Scanning()   						*/
/* Location: 													*/
/* Purpose: Check if random address for scanning.				*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Random_Address_For_Scanning( SCANNING_PARAMETERS* ScanPar )
{
	switch( ScanPar->Own_Random_Address_Type )
	{
	case STATIC_DEVICE_ADDRESS:
		/* During active scanning, a privacy enabled Central shall use a non-resolvable
		or resolvable private address. */
		return ( ( ( ScanPar->LE_Scan_Type == ACTIVE_SCANNING ) && ScanPar->Privacy ) ? FALSE : TRUE );
		break;

	case RESOLVABLE_PRIVATE:
		/* Permitted in privacy mode */
		/* But we need to know if we have record for the peer identity. */
		return ( Check_Local_Resolvable_Private_Address( &ScanPar->PeerId ) );
		break;

	case NON_RESOLVABLE_PRIVATE:
		/* Permitted in privacy mode */
		return (TRUE);
		break;
	}
	return (FALSE);
}


/****************************************************************/
/* Check_Local_Resolvable_Private_Address()   					*/
/* Location: 													*/
/* Purpose: Check if local RPA can be generated.				*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Local_Resolvable_Private_Address( IDENTITY_ADDRESS* Peer_Identity_Address )
{
	RESOLVING_RECORD* RecordPtr = Get_Record_From_Peer_Identity( Peer_Identity_Address );
	if( RecordPtr != NULL )
	{
		/* The local IRK must be valid since local identity would be used instead of
		 * Resolvable private address. */
		if( !Check_NULL_IRK( &RecordPtr->Peer.Local_IRK ) )
		{
			return (TRUE);
		}
	}
	return (FALSE);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
