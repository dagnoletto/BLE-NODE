

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
static CONNECTION_HANDLE* Search_Connection_Handle( uint16_t ConnHandle );
static CONNECTION_HANDLE* Add_Connection_Handle( uint16_t ConnHandle, BLE_ROLE Role );
static void Change_Connection_Handle_Status( CONNECTION_HANDLE* HandlePtr, CONN_HANDLE_STATUS Status );
void Remove_Connection_Index( uint8_t Index );


/****************************************************************/
/* extern functions declaration                                 */
/****************************************************************/
extern void Set_BLE_State( BLE_STATES NewBLEState );
extern uint8_t Exit_Advertising_Mode( BLE_STATES CurrentState );
extern uint8_t Exit_Initiating_Mode( BLE_STATES CurrentState );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define MAX_NUMBER_OF_CONNECTIONS 4


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static CONNECTION_HANDLE Connection_Handle_List[MAX_NUMBER_OF_CONNECTIONS];


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
		if ( Connection_Handle_List[i].Status != CONN_HANDLE_FREE )
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
CONNECTION_HANDLE* Get_Connection_Handle( uint8_t Index )
{
	if( Index < MAX_NUMBER_OF_CONNECTIONS )
	{
		return ( &Connection_Handle_List[Index] );
	}

	return ( NULL );
}


/****************************************************************/
/* Search_Connection_Handle()        							*/
/* Location: 					 								*/
/* Purpose: Verify if a certain connection handle exists.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static CONNECTION_HANDLE* Search_Connection_Handle( uint16_t ConnHandle )
{
	for( uint8_t i = 0; i < MAX_NUMBER_OF_CONNECTIONS; i++ )
	{
		if ( Connection_Handle_List[i].Handle == ConnHandle )
		{
			return ( &Connection_Handle_List[i] ); /* return the list index */
		}
	}

	return ( NULL );
}


/****************************************************************/
/* Add_Connection_Handle()        								*/
/* Location: 					 								*/
/* Purpose: Add connection handle to the list.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static CONNECTION_HANDLE* Add_Connection_Handle( uint16_t ConnHandle, BLE_ROLE Role )
{
	if( ConnHandle <= MAX_CONNECTION_HANDLE )
	{
		/* Check if the handle is already loaded */
		CONNECTION_HANDLE* HandlePtr = Search_Connection_Handle( ConnHandle );

		if( HandlePtr != NULL ) /* There is already a handler with this code */
		{
			if( HandlePtr->Status == CONN_HANDLE_FREE )
			{
				HandlePtr->Handle = ConnHandle;
				HandlePtr->Role = Role;
				HandlePtr->Status = CONN_HANDLE_FULL;
				return ( HandlePtr );
			}
		}else
		{
			/* There is no handler with this code */
			for( uint8_t i = 0; i < MAX_NUMBER_OF_CONNECTIONS; i++ )
			{
				/* Search for available room */
				if ( Connection_Handle_List[i].Status == CONN_HANDLE_FREE )
				{
					Connection_Handle_List[i].Handle = ConnHandle;
					Connection_Handle_List[i].Role = Role;
					Connection_Handle_List[i].Status = CONN_HANDLE_FULL;
					return ( &Connection_Handle_List[i] );
				}
			}
		}
	}
	return ( NULL );
}


/****************************************************************/
/* Change_Connection_Handle_Status()   							*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Change_Connection_Handle_Status( CONNECTION_HANDLE* HandlePtr, CONN_HANDLE_STATUS Status )
{
	if( HandlePtr != NULL )
	{
		HandlePtr->Status = Status;
	}
}


/****************************************************************/
/* Remove_Connection_Index()        							*/
/* Location: 					 								*/
/* Purpose: Remove connection index from the list.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Remove_Connection_Index( uint8_t Index )
{
	if( Index < MAX_NUMBER_OF_CONNECTIONS )
	{
		Connection_Handle_List[Index].Status = CONN_HANDLE_FREE;
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
	Add_Connection_Handle( ConnCpltData->Connection_Handle, ConnCpltData->Role );
	if( ConnCpltData->Role == MASTER )
	{
		Master_Connection_Complete( ConnCpltData );
	}else if( ConnCpltData->Role == SLAVE )
	{
		Slave_Connection_Complete( ConnCpltData );
	}
}


/****************************************************************/
/* HCI_Disconnection_Complete()        							*/
/* Location: Page 2296 Core_v5.2								*/
/* Purpose: The HCI_Disconnection_Complete event occurs when a 	*/
/* connection is terminated. The status parameter indicates if	*/
/* the disconnection was successful or not. The reason parameter*/
/* indicates the reason for the disconnection if the 			*/
/* disconnection was successful. If the disconnection was not 	*/
/* successful, the value of the reason parameter shall be 		*/
/* ignored by the Host. For example, this can be the case if 	*/
/* the Host has issued the HCI_Disconnect command and there was */
/* a parameter error, or the command was not presently allowed, */
/* or a Connection_Handle that didn’t correspond to a 			*/
/* connection was given. Note: When a physical link fails, one 	*/
/* HCI_Disconnection_Complete event will be returned for each 	*/
/* logical channel on the physical link with the corresponding	*/
/* Connection_Handle as a parameter.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Disconnection_Complete( DisconnectionComplete* DisConnCpltData )
{
	BLE_STATES state = Get_BLE_State();
	CONN_HANDLE_STATUS newstatus = ( DisConnCpltData->Status == COMMAND_SUCCESS ) ? CONN_HANDLE_FREE : CONN_HANDLE_FAILED;
	CONNECTION_HANDLE* HandlePtr = Search_Connection_Handle( DisConnCpltData->Connection_Handle );

	if( state == CONNECTION_STATE )
	{
		if( ( newstatus == CONN_HANDLE_FREE ) && ( Get_Number_Of_Active_Connections() == 1 ) )
		{
			Change_Connection_Handle_Status( HandlePtr, newstatus );
			Enter_Standby_Mode();
		}else
		{
			Change_Connection_Handle_Status( HandlePtr, newstatus );
		}
	}else if( state == CONFIG_STANDBY )
	{
		Change_Connection_Handle_Status( HandlePtr, newstatus );
		Enter_Standby_Mode();
	}else
	{
		Change_Connection_Handle_Status( HandlePtr, newstatus );
	}

	if( HandlePtr != NULL )
	{
		if( HandlePtr->Role == MASTER )
		{
			Master_Disconnection_Complete( DisConnCpltData );
		}else if( HandlePtr->Role == SLAVE )
		{
			Slave_Disconnection_Complete( DisConnCpltData );
		}
	}
}


/****************************************************************/
/* HCI_Number_Of_Completed_Packets()                			*/
/* Location: 2315 Core_v5.2		 								*/
/* Purpose: The HCI_Number_Of_Completed_Packets event is used 	*/
/* by the Controller to indicate to the Host how many HCI Data 	*/
/* packets have been completed (transmitted or flushed) for each*/
/* Connection_Handle since the previous 						*/
/* HCI_Number_Of_Completed_Packets event was sent to the Host. 	*/
/* This means that the corresponding buffer space has been 		*/
/* freed in the Controller. Based on this information, and the 	*/
/* Total_Num_ACL_Data_Packets and 								*/
/* Total_Num_Synchronous_Data_Packets return parameter of the	*/
/* HCI_Read_Buffer_Size command, the Host can determine for 	*/
/* which Connection_Handles the following HCI Data packets 		*/
/* should be sent to the Controller. The 						*/
/* HCI_Number_Of_Completed_Packets event shall not specify a 	*/
/* given Connection_Handle before the HCI_Connection_Complete 	*/
/* event for the corresponding connection or after an event 	*/
/* indicating disconnection of the corresponding connection. 	*/
/* While the Controller has HCI Data packets in its buffer, it 	*/
/* shall keep sending the HCI_Number_Of_Completed_Packets event */
/* to the Host at least periodically, until it finally reports 	*/
/* that all the pending ACL Data packets have been transmitted 	*/
/* or flushed. The rate with which this event is sent is 		*/
/* manufacturer specific. Note: HCI_Number_Of_Completed_Packets */
/* events will not report on synchronous Connection_Handles if  */
/* synchronous Flow Control is disabled.	(See Section 7.3.36 */
/* and Section 7.3.37.)											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Number_Of_Completed_Packets( uint8_t Num_Handles, uint16_t Connection_Handle[], uint16_t Num_Completed_Packets[] )
{
	volatile uint8_t teste = 0;
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
/* Master_Disconnection_Complete()     	    					*/
/* Location: 					 								*/
/* Purpose: Informs the master a connection was terminated		*/
/* with a slave													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void Master_Disconnection_Complete( DisconnectionComplete* DisConnCpltData )
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
/* Slave_Disconnection_Complete()     	    					*/
/* Location: 					 								*/
/* Purpose: Informs the slave a connection was terminated		*/
/* with the master												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void Slave_Disconnection_Complete( DisconnectionComplete* DisConnCpltData )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
