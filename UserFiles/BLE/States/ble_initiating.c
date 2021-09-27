

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"
#include "hosted_functions.h"
#include "ble_initiating.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	CANCEL_INITIATING,
	DISABLE_ADDRESS_RESOLUTION,
	CLEAR_RESOLVING_LIST,
	ADD_TO_RESOLVING_LIST,
	VERIFY_OWN_ADDRESS,
	GENERATE_RANDOM_ADDRESS,
	WAIT_FOR_NEW_LOCAL_READ,
	SET_RANDOM_ADDRESS,
	ENABLE_ADDRESS_RESOLUTION,
	SET_INIT_PARAMETERS,
	END_INIT_CONFIG,
	FAILED_INIT_CONFIG,
	WAIT_OPERATION,
}INIT_CONFIG_STEPS;


typedef struct
{
	INIT_CONFIG_STEPS Actual;
	INIT_CONFIG_STEPS Next;
	INIT_CONFIG_STEPS Prev;
}INIT_CONFIG;


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
int8_t Initiating_Config( void );
void Initiating( void );
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address );
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static uint8_t Check_Initiator_Parameters( INITIATING_PARAMETERS* InitPar );
static uint8_t Check_Random_Address_For_Initiating( INITIATING_PARAMETERS* InitPar );
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
static INITIATING_PARAMETERS* InitiatingParameters = NULL;
static INIT_CONFIG InitConfig = { CANCEL_INITIATING, CANCEL_INITIATING, CANCEL_INITIATING };
static BD_ADDR_TYPE RandomAddress;
static uint16_t SM_Resolving_List_Index;


/****************************************************************/
/* Enter_Initiating_Mode()        	 							*/
/* Location: 					 								*/
/* Purpose: Put the controller in initiating mode.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Enter_Initiating_Mode( INITIATING_PARAMETERS* InitPar )
{
	if( Get_BLE_State( ) == STANDBY_STATE )
	{
		if( Check_Initiating_Parameters( InitPar )  )
		{
			if( InitiatingParameters != NULL )
			{
				free(InitiatingParameters);
				InitiatingParameters = NULL;
			}

			InitiatingParameters = malloc( sizeof(INITIATING_PARAMETERS) );

			if( InitiatingParameters != NULL )
			{
				*InitiatingParameters = *InitPar;
				Set_BLE_State( CONFIG_INITIATING );
				return (TRUE);
			}
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Initiating_Config()        	   								*/
/* Location: 					 								*/
/* Purpose: Configure initiating type.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
int8_t Initiating_Config( void )
{
	static uint32_t InitConfigTimeout = 0;
	static RESOLVING_RECORD* RecordPtr;

	switch( InitConfig.Actual )
	{
	case CANCEL_INITIATING:
		RecordPtr = NULL;
		InitConfigTimeout = 0;
		InitiatingParameters->Counter = 0;
		InitConfig.Next = DISABLE_ADDRESS_RESOLUTION;
		InitConfig.Prev = CANCEL_INITIATING;
		//InitConfig.Actual = HCI_LE_Set_Scan_Enable( FALSE, FALSE, &LE_Set_Scan_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_INITIATING;
		break;

	case DISABLE_ADDRESS_RESOLUTION:
		InitConfig.Next = CLEAR_RESOLVING_LIST;
		InitConfig.Prev = DISABLE_ADDRESS_RESOLUTION;
		InitConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( FALSE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADDRESS_RESOLUTION;
		break;

//	case CLEAR_RESOLVING_LIST:
//		if( ( ScanningParameters->Privacy ) || ( ScanningParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) ||
//				( ScanningParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) ||
//				( ( ScanningParameters->Own_Address_Type == OWN_RANDOM_DEV_ADDR ) && ( ScanningParameters->Own_Random_Address_Type == RESOLVABLE_PRIVATE ) ) )
//		{
//			/* Check if this peer device is in the resolving list */
//			RecordPtr = Get_Record_From_Peer_Identity( &ScanningParameters->PeerId );
//
//			SM_Resolving_List_Index = 0;
//			ScanConfig.Actual = HCI_LE_Clear_Resolving_List( &LE_Clear_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : CLEAR_RESOLVING_LIST;
//		}else
//		{
//			ScanConfig.Actual = VERIFY_OWN_ADDRESS;
//		}
//		break;
//
//	case ADD_TO_RESOLVING_LIST:
//		/* Check if we have bonded devices to add to the resolving list */
//		if ( SM_Resolving_List_Index < Get_Number_Of_Resolving_Records() )
//		{
//			DEVICE_IDENTITY* DevId = &( Get_Record_From_Index( SM_Resolving_List_Index )->Peer );
//			/* Here we add a device to the controller's resolving list if it exists in the Host's list */
//			ScanConfig.Actual = HCI_LE_Add_Device_To_Resolving_List( DevId->Peer_Identity_Address.Type, DevId->Peer_Identity_Address.Address,
//					&DevId->Peer_IRK, &DevId->Local_IRK, &LE_Add_Device_To_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : ADD_TO_RESOLVING_LIST;
//		}else
//		{
//			ScanConfig.Actual = VERIFY_OWN_ADDRESS;
//		}
//		break;
//
//	case VERIFY_OWN_ADDRESS:
//		switch( ScanningParameters->Own_Address_Type )
//		{
//		case OWN_PUBLIC_DEV_ADDR:
//		case OWN_RESOL_OR_PUBLIC_ADDR:
//			ScanConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
//			break;
//
//			/* For random address options, we shall populate the random address before setting scanning parameters */
//		case OWN_RANDOM_DEV_ADDR:
//		case OWN_RESOL_OR_RANDOM_ADDR:
//		default:
//			ScanConfig.Actual = GENERATE_RANDOM_ADDRESS;
//			break;
//		}
//		break;
//
//		case GENERATE_RANDOM_ADDRESS:
//		{
//			switch( ScanningParameters->Own_Random_Address_Type )
//			{
//			case NON_RESOLVABLE_PRIVATE:
//			{
//				BD_ADDR_TYPE* Ptr = Generate_Device_Address( Get_Supported_Commands(), NON_RESOLVABLE_PRIVATE, NULL, 7 );
//				if ( Ptr != NULL )
//				{
//					RandomAddress = *Ptr;
//					ScanConfig.Actual = SET_RANDOM_ADDRESS;
//				}
//			}
//			break;
//
//			case RESOLVABLE_PRIVATE:
//				if( RecordPtr != NULL )
//				{
//					ScanConfigTimeout = 0;
//					/* We should use RPA generated by the controller. */
//					ScanConfig.Actual = HCI_LE_Read_Local_Resolvable_Address( RecordPtr->Peer.Peer_Identity_Address.Type,
//							RecordPtr->Peer.Peer_Identity_Address.Address, &Read_Local_Resolvable_Address_Complete, NULL ) ? WAIT_OPERATION : WAIT_FOR_NEW_LOCAL_READ;
//				}else
//				{
//					ScanConfig.Actual = FAILED_SCAN_CONFIG;
//				}
//				break;
//
//			case STATIC_DEVICE_ADDRESS:
//			default:
//				RandomAddress = *( Get_Static_Random_Device_Address( ).Ptr );
//				ScanConfig.Actual = SET_RANDOM_ADDRESS;
//				break;
//			}
//		}
//		break;
//
//		case WAIT_FOR_NEW_LOCAL_READ:
//			if( TimeBase_DelayMs( &ScanConfigTimeout, 500, TRUE ) )
//			{
//				ScanConfig.Actual = GENERATE_RANDOM_ADDRESS;
//			}
//			break;
//
//		case SET_RANDOM_ADDRESS:
//			ScanConfigTimeout = 0;
//			ScanConfig.Next = ENABLE_ADDRESS_RESOLUTION;
//			ScanConfig.Actual = HCI_LE_Set_Random_Address( RandomAddress, &LE_Set_Random_Address_Complete, NULL ) ? WAIT_OPERATION : SET_RANDOM_ADDRESS;
//			break;
//
//		case ENABLE_ADDRESS_RESOLUTION:
//			if( ScanningParameters->Privacy )
//			{
//				ScanConfig.Next = SET_SCAN_PARAMETERS;
//				ScanConfig.Prev = ENABLE_ADDRESS_RESOLUTION;
//				ScanConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( TRUE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADDRESS_RESOLUTION;
//			}else
//			{
//				ScanConfig.Actual = SET_SCAN_PARAMETERS;
//			}
//			break;
//
//		case SET_SCAN_PARAMETERS:
//			ScanConfigTimeout = 0;
//			ScanConfig.Next = ENABLE_SCANNING;
//			ScanConfig.Actual = HCI_LE_Set_Scan_Parameters( ScanningParameters->LE_Scan_Type, ScanningParameters->LE_Scan_Interval, ScanningParameters->LE_Scan_Window,
//					ScanningParameters->Own_Address_Type, ScanningParameters->Scanning_Filter_Policy, &LE_Set_Scan_Parameters_Complete, NULL ) ? WAIT_OPERATION : SET_SCAN_PARAMETERS;
//			break;
//
//		case ENABLE_SCANNING:
//			ScanConfigTimeout = 0;
//			ScanConfig.Next = END_INIT_CONFIG;
//			ScanConfig.Prev = ENABLE_SCANNING;
//			ScanConfig.Actual = HCI_LE_Set_Scan_Enable( TRUE, ScanningParameters->Filter_Duplicates, &LE_Set_Scan_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_SCANNING;
//			break;

		case END_INIT_CONFIG:
			InitConfig.Actual = CANCEL_INITIATING;
			return (TRUE);
			break;

		case FAILED_INIT_CONFIG:
			InitConfig.Actual = CANCEL_INITIATING;
			free(InitiatingParameters);
			InitiatingParameters = NULL;
			return (-1); /* Failed condition */
			break;

		case WAIT_OPERATION:
			if( TimeBase_DelayMs( &InitConfigTimeout, 500, TRUE ) )
			{
				InitConfig.Actual = CANCEL_INITIATING;
				InitConfig.Next = CANCEL_INITIATING;
			}
			break;

		default:
			break;
	}

	return (FALSE);
}


/****************************************************************/
/* LE_Set_Address_Resolution_Enable_Complete()    				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	InitConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? InitConfig.Next : InitConfig.Prev;
}


/****************************************************************/
/* LE_Clear_Resolving_List_Complete()      						*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	InitConfig.Actual = ADD_TO_RESOLVING_LIST;
}


/****************************************************************/
/* LE_Add_Device_To_Resolving_List_Complete()	      			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	if ( Status == COMMAND_SUCCESS )
	{
		InitConfig.Actual = ADD_TO_RESOLVING_LIST;
		SM_Resolving_List_Index++;
	}else if( Status == MEM_CAPACITY_EXCEEDED )
	{
		InitConfig.Actual = VERIFY_OWN_ADDRESS;
	}else
	{
		InitConfig.Actual = ADD_TO_RESOLVING_LIST;
	}
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
	InitConfig.Actual = ( Status == COMMAND_SUCCESS ) ? InitConfig.Next : SET_RANDOM_ADDRESS;
}


/****************************************************************/
/* Read_Local_Resolvable_Address_Complete()        	   			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address )
{
	if( Status == COMMAND_SUCCESS )
	{
		InitConfig.Actual = SET_RANDOM_ADDRESS;
		RandomAddress = *Local_Resolvable_Address;
	}else
	{
		InitConfig.Actual = WAIT_FOR_NEW_LOCAL_READ;
	}
}


/****************************************************************/
/* Initiating()        	   										*/
/* Location: 					 								*/
/* Purpose: Initiating											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Initiating( void )
{
	//TODO: verificar se alguma coisa deve ser feita aqui
//	/* Privacy feature in a Central with Host-based privacy: The Host shall generate a resolvable
//	 * private address using the ‘resolvable private address generation procedure’ as defined in
//	 * Section 10.8.2.2 or non-resolvable private address procedure as defined in Section 10.8.2.1.
//	 * The Host shall set a timer equal to TGAP(private_addr_int). The Host shall generate a
//	 * new resolvable private address or non-resolvable private address when the
//	 * timer TGAP(private_addr_int) expires. Note: TGAP(private_addr_int) timer need not be run
//	 * if a Central is not scanning or connected. (Page 1389 Core_v5.2).
//	 * During active scanning, a privacy enabled Central shall use a non-resolvable
//	 * or resolvable private address. */
//
//	/* Privacy feature in an Observer: During active scanning, a privacy enabled Observer shall use
//	 * either a resolvable private address or non-resolvable private address.
//	 * If Address Resolution is not supported or disabled in the Controller, the
//	 * following applies to the Host: The Host shall generate a resolvable private
//	 * address using the ‘resolvable private address generation procedure’ as defined
//	 * in Section 10.8.2.2 or non-resolvable private address procedure as defined in
//	 * Section 10.8.2.1. The Host shall set a timer equal to TGAP(private_addr_int).
//	 * The Host shall generate a new resolvable private address or non-resolvable
//	 * private address when the timer TGAP(private_addr_int) expires. The value of
//	 * TGAP(private_addr_int) shall not be greater than 1 hour. Note: TGAP(private_addr_int)
//	 * timer need not be run if an Observer is not scanning. (Page 1389 Core_v5.2). */
//
//	/* If the Controller has address resolution and that is enabled, the RPA is
//	 * automatically generated given the timeout for the controller. However, in versions
//	 * lower that 4.1, this functionality does not exist so we need to generate again.
//	 * For non-resolvable private address we shall generate since the controller will not do it. */
//
//	/* TODO: Here we consider the address resolution is always enabled in scanning.
//	 * Even for versions higher than 4.1, this function should be called if address resolution
//	 * is disabled. We lay on the fact address resolution is always enabled in the controller
//	 * for versions above 4.1. */

//	if( ScanningParameters->Privacy && ( ScanningParameters->LE_Scan_Type == ACTIVE_SCANNING ) &&
//			( ( ScanningParameters->Own_Address_Type == OWN_RANDOM_DEV_ADDR || ScanningParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) &&
//					( ScanningParameters->Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE || ScanningParameters->Own_Random_Address_Type == RESOLVABLE_PRIVATE ) ) )
//	{
//		if( TimeBase_DelayMs( &ScanningParameters->Counter, TGAP_PRIVATE_ADDR_INT, TRUE ) )
//		{
//			Set_BLE_State( CONFIG_SCANNING );
//		}
//	}
}


/****************************************************************/
/* Check_Initiating_Parameters()      							*/
/* Location: 													*/
/* Purpose: Verify initiating parameters.				 		*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
uint8_t Check_Initiating_Parameters( INITIATING_PARAMETERS* InitPar )
{
	if( InitPar->LE_Scan_Window > InitPar->LE_Scan_Interval )
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
		if( ( InitPar->Own_Address_Type > OWN_RANDOM_DEV_ADDR) || ( InitPar->Initiator_Filter_Policy > 1 ) )
		{
			return (FALSE);
		}
	}

	return( Check_Initiator_Parameters( InitPar ) );
}


/****************************************************************/
/* Check_Initiator_Parameters()      							*/
/* Location: 													*/
/* Purpose: Verify initiating parameters for initiator.			*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Initiator_Parameters( INITIATING_PARAMETERS* InitPar )
{
	switch( InitPar->Own_Address_Type )
	{
	case OWN_PUBLIC_DEV_ADDR:
		/* During active scanning, a privacy enabled Central shall use a non-resolvable
		or resolvable private address. */
		return ( ( ( /* InitPar->LE_Scan_Type == ACTIVE_SCANNING */0 /* TODO */ ) && InitPar->Privacy ) ? FALSE : TRUE );
		break;

	case OWN_RANDOM_DEV_ADDR:
		return ( Check_Random_Address_For_Initiating( InitPar ) );
		break;

	case OWN_RESOL_OR_PUBLIC_ADDR:
		/* Due to the fact the public address may be used for active scanning, no privacy is
		 * guaranteed under this condition. */
		return ( ( ( /* InitPar->LE_Scan_Type == ACTIVE_SCANNING */0 /* TODO */ ) && InitPar->Privacy ) ? FALSE : TRUE );
		break;

	case OWN_RESOL_OR_RANDOM_ADDR:
		/* If we are scanning actively with privacy, the privacy depends pretty much of how
		 * random part is configured. */
		if( ( /* InitPar->LE_Scan_Type == ACTIVE_SCANNING */0 /* TODO */ ) && InitPar->Privacy )
		{
			/* Check random part */
			return ( Check_Random_Address_For_Initiating( InitPar ) );
		}else if( /* InitPar->Own_Random_Address_Type == RESOLVABLE_PRIVATE */0 /* TODO */ )
		{
			/* Check resolvable private address is possible to generate */
			return ( /* Check_Local_Resolvable_Private_Address( &InitPar->PeerId ) */0 /* TODO */ );
		}else
		{
			return (TRUE);
		}
		break;
	}
	return (FALSE);
}


/****************************************************************/
/* Check_Random_Address_For_Initiating()   						*/
/* Location: 													*/
/* Purpose: Check random address for initiating.				*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Random_Address_For_Initiating( INITIATING_PARAMETERS* InitPar )
{
	switch( /* InitPar->Own_Random_Address_Type */0 /* TODO */ )
	{
	case STATIC_DEVICE_ADDRESS:
		/* During active scanning, a privacy enabled Central shall use a non-resolvable
		or resolvable private address. */
		return ( ( ( /* InitPar->LE_Scan_Type == ACTIVE_SCANNING */0 /* TODO */ ) && InitPar->Privacy ) ? FALSE : TRUE );
		break;

	case RESOLVABLE_PRIVATE:
		/* Permitted in privacy mode */
		/* But we need to know if we have record for the peer identity. */
		return ( /* Check_Local_Resolvable_Private_Address( &InitPar->PeerId ) */0 /* TODO */ );
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
