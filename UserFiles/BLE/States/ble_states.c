

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "vendor_specific.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t BLE_Init( void );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static BLE_STATES BLEState = VENDOR_SPECIFIC_INIT;


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
	case VENDOR_SPECIFIC_INIT:
		if( Vendor_Specific_Init(  ) )
		{
			BLEState = BLE_INITIAL_SET;
		}
		break;

	case BLE_INITIAL_SET:
		if( BLE_Init(  ) )
		{
			BLEState = BLE_STANDBY;
		}
		break;

	case BLE_STANDBY:
		break;
	}
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

}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
