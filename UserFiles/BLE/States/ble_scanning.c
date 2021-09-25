

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
	VERIFY_OWN_ADDRESS,
	GENERATE_RANDOM_ADDRESS,
	WAIT_FOR_NEW_LOCAL_READ,
	SET_RANDOM_ADDRESS,
	ENABLE_ADDRESS_RESOLUTION,
	SET_SCAN_PARAMETERS,
	ENABLE_SCANNING,
	END_SCAN_CONFIG,
	FAILED_SCAN_CONFIG,
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
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address );
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Scan_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Scan_Parameters_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );
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
			ScanConfig.Actual = VERIFY_OWN_ADDRESS;
		}
		break;

	case ADD_TO_RESOLVING_LIST:
		/* Check if we have bonded devices to add to the resolving list */
		if ( SM_Resolving_List_Index < Get_Number_Of_Resolving_Records() )
		{
			DEVICE_IDENTITY* DevId = &( Get_Record_From_Index( SM_Resolving_List_Index )->Peer );
			/* Here we add a device to the controller's resolving list if it exists in the Host's list */
			ScanConfig.Actual = HCI_LE_Add_Device_To_Resolving_List( DevId->Peer_Identity_Address.Type, DevId->Peer_Identity_Address.Address,
					&DevId->Peer_IRK, &DevId->Local_IRK, &LE_Add_Device_To_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : ADD_TO_RESOLVING_LIST;
		}else
		{
			ScanConfig.Actual = VERIFY_OWN_ADDRESS;
		}
		break;

	case VERIFY_OWN_ADDRESS:
		switch( ScanningParameters->Own_Address_Type )
		{
		case OWN_PUBLIC_DEV_ADDR:
		case OWN_RESOL_OR_PUBLIC_ADDR:
			ScanConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
			break;

			/* For random address options, we shall populate the random address before setting scanning parameters */
		case OWN_RANDOM_DEV_ADDR:
		case OWN_RESOL_OR_RANDOM_ADDR:
		default:
			ScanConfig.Actual = GENERATE_RANDOM_ADDRESS;
			break;
		}
		break;

		case GENERATE_RANDOM_ADDRESS:
		{
			switch( ScanningParameters->Own_Random_Address_Type )
			{
			case NON_RESOLVABLE_PRIVATE:
			{
				BD_ADDR_TYPE* Ptr = Generate_Device_Address( Get_Supported_Commands(), NON_RESOLVABLE_PRIVATE, NULL, 7 );
				if ( Ptr != NULL )
				{
					RandomAddress = *Ptr;
					ScanConfig.Actual = SET_RANDOM_ADDRESS;
				}
			}
			break;

			case RESOLVABLE_PRIVATE:
				if( RecordPtr != NULL )
				{
					ScanConfigTimeout = 0;
					/* We should use RPA generated by the controller. */
					ScanConfig.Actual = HCI_LE_Read_Local_Resolvable_Address( RecordPtr->Peer.Peer_Identity_Address.Type,
							RecordPtr->Peer.Peer_Identity_Address.Address, &Read_Local_Resolvable_Address_Complete, NULL ) ? WAIT_OPERATION : WAIT_FOR_NEW_LOCAL_READ;
				}else
				{
					ScanConfig.Actual = FAILED_SCAN_CONFIG;
				}
				break;

			case STATIC_DEVICE_ADDRESS:
			default:
				RandomAddress = *( Get_Static_Random_Device_Address( ).Ptr );
				ScanConfig.Actual = SET_RANDOM_ADDRESS;
				break;
			}
		}
		break;

		case WAIT_FOR_NEW_LOCAL_READ:
			if( TimeBase_DelayMs( &ScanConfigTimeout, 500, TRUE ) )
			{
				ScanConfig.Actual = GENERATE_RANDOM_ADDRESS;
			}
			break;

		case SET_RANDOM_ADDRESS:
			ScanConfigTimeout = 0;
			ScanConfig.Next = ENABLE_ADDRESS_RESOLUTION;
			ScanConfig.Actual = HCI_LE_Set_Random_Address( RandomAddress, &LE_Set_Random_Address_Complete, NULL ) ? WAIT_OPERATION : SET_RANDOM_ADDRESS;
			break;

		case ENABLE_ADDRESS_RESOLUTION:
			if( ScanningParameters->Privacy )
			{
				ScanConfig.Next = SET_SCAN_PARAMETERS;
				ScanConfig.Prev = ENABLE_ADDRESS_RESOLUTION;
				ScanConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( TRUE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADDRESS_RESOLUTION;
			}else
			{
				ScanConfig.Actual = SET_SCAN_PARAMETERS;
			}
			break;

		case SET_SCAN_PARAMETERS:
			ScanConfigTimeout = 0;
			ScanConfig.Next = ENABLE_SCANNING;
			ScanConfig.Actual = HCI_LE_Set_Scan_Parameters( ScanningParameters->LE_Scan_Type, ScanningParameters->LE_Scan_Interval, ScanningParameters->LE_Scan_Window,
					ScanningParameters->Own_Address_Type, ScanningParameters->Scanning_Filter_Policy, &LE_Set_Scan_Parameters_Complete, NULL ) ? WAIT_OPERATION : SET_SCAN_PARAMETERS;
			break;

		case ENABLE_SCANNING:
			ScanConfigTimeout = 0;
			ScanConfig.Next = END_SCAN_CONFIG;
			ScanConfig.Prev = ENABLE_SCANNING;
			ScanConfig.Actual = HCI_LE_Set_Scan_Enable( TRUE, ScanningParameters->Filter_Duplicates, &LE_Set_Scan_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_SCANNING;
			break;

		case END_SCAN_CONFIG:
			ScanConfig.Actual = DISABLE_SCANNING;
			return (TRUE);
			break;

		case FAILED_SCAN_CONFIG:
			ScanConfig.Actual = DISABLE_SCANNING;
			free(ScanningParameters);
			ScanningParameters = NULL;
			return (-1); /* Failed condition */
			break;

		case WAIT_OPERATION:
			if( TimeBase_DelayMs( &ScanConfigTimeout, 500, TRUE ) )
			{
				ScanConfig.Actual = DISABLE_SCANNING;
				ScanConfig.Next = DISABLE_SCANNING;
			}
			break;

		default:
			break;
	}

	return (FALSE);
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
/* LE_Add_Device_To_Resolving_List_Complete()	      			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	if ( Status == COMMAND_SUCCESS )
	{
		ScanConfig.Actual = ADD_TO_RESOLVING_LIST;
		SM_Resolving_List_Index++;
	}else if( Status == MEM_CAPACITY_EXCEEDED )
	{
		ScanConfig.Actual = VERIFY_OWN_ADDRESS;
	}else
	{
		ScanConfig.Actual = ADD_TO_RESOLVING_LIST;
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
	ScanConfig.Actual = ( Status == COMMAND_SUCCESS ) ? ScanConfig.Next : SET_RANDOM_ADDRESS;
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
		ScanConfig.Actual = SET_RANDOM_ADDRESS;
		RandomAddress = *Local_Resolvable_Address;
	}else
	{
		ScanConfig.Actual = WAIT_FOR_NEW_LOCAL_READ;
	}
}


/****************************************************************/
/* LE_Set_Scan_Parameters_Complete()       			 			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Scan_Parameters_Complete( CONTROLLER_ERROR_CODES Status )
{
	ScanConfig.Actual = ( Status == COMMAND_SUCCESS ) ? ScanConfig.Next : SET_SCAN_PARAMETERS;
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
/* HCI_LE_Advertising_Report()                					*/
/* Location: 2382 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Advertising_Report event indicates that 	*/
/* one or more Bluetooth devices have responded to an active 	*/
/* scan or have broadcast advertisements that were received 	*/
/* during a passive scan. The Controller may queue these 		*/
/* advertising reports and send information from multiple 		*/
/* devices in one HCI_LE_Advertising_Report event. This event 	*/
/* shall only be generated if scanning was enabled using the	*/
/* HCI_LE_Set_Scan_Enable command. It only reports advertising  */
/* events that used legacy advertising PDUs.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
//TODO: callback implemented in higher layers for now. In the future this function may fill-up a list and the higher layers
//could retrieve devices from this list
//void HCI_LE_Advertising_Report( uint8_t Subevent_Code, uint8_t Num_Reports, uint8_t Event_Type[], uint8_t Address_Type[], BD_ADDR_TYPE Address[],
//		uint8_t Data_Length[], uint8_t Data[], int8_t RSSI[] )
//{
//	/* Sub event code for the HCI_LE_Advertising_Report event: page 2382 Core_v5.2 */
//	if( Subevent_Code == 0x02 )
//	{
//		ADVERTISING_REPORT Report;
//		uint16_t Number_Of_Data_Bytes = 0;
//		static uint8_t AdvData[40];
//
//		for( uint8_t i = 0; i < Num_Reports; i++ )
//		{
//			Report.Event_Type = Event_Type[i];
//			Report.Address_Type = Address_Type[i];
//			Report.Address = Address[i];
//			Report.Data_Length = Data_Length[i];
//			Report.RSSI = RSSI[i];
//			Report.DataPtr = &AdvData[0];
//
//			memcpy( Report.DataPtr, &Data[Number_Of_Data_Bytes], Report.Data_Length );
//
//			Number_Of_Data_Bytes += Report.Data_Length;
//
//			//TODO: add device to scanned device list
//		}
//	}
//}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
