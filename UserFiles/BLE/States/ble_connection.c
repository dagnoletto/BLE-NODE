

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
/* HCI_LE_Enhanced_Connection_Complete()               			*/
/* Location: 2394 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Enhanced_Connection_Complete event 		*/
/* indicates to both of the Hosts forming the connection that a */
/* new connection has been created. Upon the creation of the 	*/
/* connection a Connection_Handle shall be assigned by the		*/
/* Controller, and passed to the Host in this event. If the 	*/
/* connection creation fails, this event shall be provided to 	*/
/* the Host that had issued the HCI_LE_Create_Connection or 	*/
/* HCI_LE_Extended_Create_Connection command. If this event is 	*/
/* unmasked and the HCI_LE_Connection_Complete event is 		*/
/* unmasked, only the HCI_LE_Enhanced_Connection_Complete event */
/* is sent when a new connection has been created. This event 	*/
/* indicates to the Host that issued an 						*/
/* HCI_LE_Create_Connection or 									*/
/* HCI_LE_Extended_Create_Connection command and received an	*/
/* HCI_Command_Status event if the connection creation failed 	*/
/* or was successful. The Peer_Address, 						*/
/* Peer_Resolvable_Private_Address, and							*/
/* Local_Resolvable_Private_Address shall always reflect the 	*/
/* most recent packet sent and received on air. The 			*/
/* Master_Clock_Accuracy parameter is only valid for a slave. 	*/
/* On a master, this parameter shall be set to 0x00.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_LE_Enhanced_Connection_Complete( LEEnhancedConnectionComplete* ConnCpltData )
{
	if( ConnCpltData->Role == MASTER )
	{
		Master_Connection_Complete( ConnCpltData );
	}else if( ConnCpltData->Role == SLAVE )
	{
		Slave_Connection_Complete( ConnCpltData );
	}
}


/****************************************************************/
/* Master_Connection_Complete()     	    					*/
/* Location: 					 								*/
/* Purpose: Informs the master a new connection was established */
/* with a slave													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void Master_Connection_Complete( LEEnhancedConnectionComplete* ConnCpltData )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* Slave_Connection_Complete()     	    						*/
/* Location: 					 								*/
/* Purpose: Informs the slave a new connection was established  */
/* with a master												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void Slave_Connection_Complete( LEEnhancedConnectionComplete* ConnCpltData )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
