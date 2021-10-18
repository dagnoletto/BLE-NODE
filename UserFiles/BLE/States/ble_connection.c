

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"
#include "hosted_functions.h"
#include "ble_connection.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
void Connection( void );


/****************************************************************/
/* extern functions declaration                                 */
/****************************************************************/
extern void Set_BLE_State( BLE_STATES NewBLEState );
extern uint8_t Exit_Advertising_Mode( BLE_STATES CurrentState );
extern uint8_t Exit_Initiating_Mode( BLE_STATES CurrentState );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/


/****************************************************************/
/* Enter_Connection_Mode()        								*/
/* Location: 					 								*/
/* Purpose: Informs the host it is in connection mode.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Enter_Connection_Mode( CONTROLLER_ERROR_CODES Status )
{
	BLE_STATES state = Get_BLE_State( );

	if( ( ( state == ADVERTISING_STATE ) || ( state == INITIATING_STATE ) ) && ( Status == COMMAND_SUCCESS ) )
	{
		uint8_t FunStat = FALSE;

		switch( state )
		{
		case ADVERTISING_STATE:
			FunStat = Exit_Advertising_Mode( state );
			break;

		case INITIATING_STATE:
			FunStat = Exit_Initiating_Mode( state );
			break;

		default: break;
		}

		if ( FunStat )
		{
			Set_BLE_State( CONNECTION_STATE );
		}
	}else
	{
		Enter_Standby_Mode(  );
	}
}


/****************************************************************/
/* Connection()        	   										*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Connection( void )
{

}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
