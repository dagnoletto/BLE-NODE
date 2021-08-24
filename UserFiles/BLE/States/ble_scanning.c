

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
	//	DISABLE_ADDRESS_RESOLUTION,
	//	CLEAR_RESOLVING_LIST,
	//	ADD_TO_RESOLVING_LIST,
	//	VERIFY_ADDRESS,
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
	//	WAIT_OPERATION,
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
//static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
//static void Read_Peer_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Peer_Resolvable_Address );
//static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Set_Advertising_Parameters_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( CONTROLLER_ERROR_CODES Status, int8_t TX_Power_Level );
//static void LE_Set_Data_Complete( CONTROLLER_ERROR_CODES Status );
//static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status );
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
//static uint16_t SM_Resolving_List_Index;


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
	static RESOLVING_RECORD* Ptr;

	switch( ScanConfig.Actual )
	{
	case DISABLE_SCANNING:
		break;

	default:
		break;
	}

	return (FALSE);
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
	if( ScanPar->LE_Scan_Window > ScanPar->LE_Scan_Interval )
	{
		/* LE_Scan_Window shall be less than or equal to LE_Scan_Interval */
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
