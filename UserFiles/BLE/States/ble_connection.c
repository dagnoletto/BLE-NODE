

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
static int16_t Search_Connection_Handle( uint16_t ConnHandle );
static int16_t Add_Connection_Handle( uint16_t ConnHandle );
static int16_t Remove_Connection_Handle( uint16_t ConnHandle );


/****************************************************************/
/* extern functions declaration                                 */
/****************************************************************/
extern void Set_BLE_State( BLE_STATES NewBLEState );
extern uint8_t Exit_Advertising_Mode( BLE_STATES CurrentState );
extern uint8_t Exit_Initiating_Mode( BLE_STATES CurrentState );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define MAX_NUMBER_OF_CONNECTIONS ( sizeof(Connection_Handle_List)/sizeof(typeof(Connection_Handle_List)) )


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static uint16_t Connection_Handle_List[] = {
		CONN_HANDLE_NULL, CONN_HANDLE_NULL,
		CONN_HANDLE_NULL, CONN_HANDLE_NULL
};


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

		if ( ( FunStat ) && ( state != CONNECTION_STATE ) )
		{
			Set_BLE_State( CONNECTION_STATE );
		}
	}else
	{
		Enter_Standby_Mode(  );
	}
}


/****************************************************************/
/* Get_Max_Number_Of_Connections()        						*/
/* Location: 					 								*/
/* Purpose: Check how many connections the device can support.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Get_Max_Number_Of_Connections( void )
{
	return ( MAX_NUMBER_OF_CONNECTIONS );
}


/****************************************************************/
/* Get_Number_Of_Active_Connections()        					*/
/* Location: 					 								*/
/* Purpose: Check how many connections the device is holding.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Get_Number_Of_Active_Connections( void )
{
	uint8_t ConnHandleCounter = 0;

	for( uint8_t i = 0; i < MAX_NUMBER_OF_CONNECTIONS; i++ )
	{
		if ( Connection_Handle_List[i] != CONN_HANDLE_NULL )
		{
			ConnHandleCounter++;
		}
	}

	return ( ConnHandleCounter );
}


/****************************************************************/
/* Get_Connection_Handle()        								*/
/* Location: 					 								*/
/* Purpose: Retrieve the connection handle for the passed 		*/
/* index.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint16_t Get_Connection_Handle( uint8_t Index )
{
	if( Index < MAX_NUMBER_OF_CONNECTIONS )
	{
		return ( Connection_Handle_List[Index] );
	}

	return ( CONN_HANDLE_NULL );
}


/****************************************************************/
/* Search_Connection_Handle()        							*/
/* Location: 					 								*/
/* Purpose: Verify if a certain connection handle exists.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static int16_t Search_Connection_Handle( uint16_t ConnHandle )
{
	if( ConnHandle != CONN_HANDLE_NULL )
	{
		for( uint8_t i = 0; i < MAX_NUMBER_OF_CONNECTIONS; i++ )
		{
			if ( Connection_Handle_List[i] == ConnHandle )
			{
				return ( i ); /* return the list index */
			}
		}
	}

	return ( -1 ); /* Not found */
}


/****************************************************************/
/* Add_Connection_Handle()        								*/
/* Location: 					 								*/
/* Purpose: Add connection handle to the list.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static int16_t Add_Connection_Handle( uint16_t ConnHandle )
{
	/* Check if the handle is already loaded */
	int16_t index = Search_Connection_Handle( ConnHandle );

	if( index < 0 )
	{
		/* There is no handler with this code */
		if( ConnHandle != CONN_HANDLE_NULL )
		{
			for( uint8_t i = 0; i < MAX_NUMBER_OF_CONNECTIONS; i++ )
			{
				/* Search for available index */
				if ( Connection_Handle_List[i] == CONN_HANDLE_NULL )
				{
					Connection_Handle_List[i] = ConnHandle;
					return ( i );
				}
			}
		}

		return ( -1 ); /* There is no room */
	}else
	{
		return ( index );
	}
}


/****************************************************************/
/* Remove_Connection_Handle()        							*/
/* Location: 					 								*/
/* Purpose: Remove connection handle from the list.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static int16_t Remove_Connection_Handle( uint16_t ConnHandle )
{
	/* Check if the handle is loaded */
	int16_t index = Search_Connection_Handle( ConnHandle );

	if( index >= 0 )
	{
		Connection_Handle_List[index] = CONN_HANDLE_NULL;
	}

	return ( index );
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
	Add_Connection_Handle( ConnCpltData->Connection_Handle );
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
