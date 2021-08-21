

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"
#include "hosted_functions.h"
#include "security_manager.h"
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
static uint8_t Check_Observer_Parameters( SCANNING_PARAMETERS* ScanPar );
static uint8_t Check_Central_Parameters( SCANNING_PARAMETERS* ScanPar );


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
static SCAN_CONFIG ScanConfig; //= { DISABLE_ADVERTISING, DISABLE_ADVERTISING, DISABLE_ADVERTISING };//TODO
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
	/* In privacy-enabled Peripheral, the Host shall set a timer equal to
	 * TGAP(private_addr_int). The Host shall generate a new resolvable
	 * private address or non-resolvable private address when the timer
	 * TGAP(private_addr_int) expires. */
	/* If the Controller has address resolution and that is enabled, the RPA is
	 * automatically generated given the timeout for the controller. However, in versions
	 * lower that 4.1, this functionality does not exist so we need to generate again.
	 * For non-resolvable private address (Broadcaster/non-connectable condition) we shall
	 * generate since the controller will not do it. */

	//TODO
//	if( ScanningParameters->Privacy && ( ( Get_Local_Version_Information()->HCI_Version <= CORE_SPEC_4_1 ) ||
//			( ( ScanningParameters->Original_Own_Address_Type == OWN_RANDOM_DEV_ADDR ) &&
//					( ScanningParameters->Original_Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE ) ) ) )
	{
//		if( TimeBase_DelayMs( &ScanningParameters->Counter, TGAP_PRIVATE_ADDR_INT, TRUE ) )
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
	if ( ( ScanPar->Own_Address_Type == OWN_RANDOM_DEV_ADDR ) && ( ScanPar->Own_Random_Address_Type == RESOLVABLE_PRIVATE ) )
	{
		/* We should only configure random as non-resolvable or static random address */
		return (FALSE);
	}

	switch( ScanPar->Role )
	{

	case OBSERVER:
		return( Check_Observer_Parameters( ScanPar ) );
		break;

	case CENTRAL:
		return( Check_Central_Parameters( ScanPar ) );
		break;

		/* Other roles are not allowed in scanning */
	default: break;

	}

	return (FALSE);
}


/****************************************************************/
/* Check_Observer_Parameters()      							*/
/* Location: 													*/
/* Purpose: Verify scanning parameters for observer role.		*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Observer_Parameters( SCANNING_PARAMETERS* ScanPar )
{

}


/****************************************************************/
/* Check_Central_Parameters()      								*/
/* Location: 													*/
/* Purpose: Verify scanning parameters for central role.		*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Central_Parameters( SCANNING_PARAMETERS* ScanPar )
{

}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
