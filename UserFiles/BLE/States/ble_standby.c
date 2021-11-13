

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"
#include "hosted_functions.h"
#include "ble_standby.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	DISABLE_ADVERTISING,
	DISABLE_SCANNING,
	DISABLE_INITIATING,
	DISABLE_ALL_CONNECTIONS,
	SEND_RESET_CMD,
	RECONFIG_VENDOR_SPECIFC,
	REINIT_BLE,
	SEND_STANDBY_CMD,
	END_STANDBY_CONFIG,
	WAIT_OPERATION,
	WAIT_OPERATION_STBY,
}STANDBY_CONFIG_STEPS;


typedef struct
{
	STANDBY_CONFIG_STEPS Actual;
	STANDBY_CONFIG_STEPS BaseStep;
	uint8_t ConnHandleCounter;
}STANDBY_CONFIG;


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
int8_t Standby_Config( void );
void Standby( void );
static void LE_Create_Connection_Cancel_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Scan_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void Disconnect_Status( CONTROLLER_ERROR_CODES Status );
static void Hal_Device_Standby_Event( CONTROLLER_ERROR_CODES Status );
void HCI_Reset_Complete( CONTROLLER_ERROR_CODES Status );


/****************************************************************/
/* extern functions declaration                                 */
/****************************************************************/
extern void Set_BLE_State( BLE_STATES NewBLEState );
extern uint8_t Exit_Advertising_Mode( BLE_STATES CurrentState );
extern uint8_t Exit_Scanning_Mode( BLE_STATES CurrentState );
extern uint8_t Exit_Initiating_Mode( BLE_STATES CurrentState );
extern CONNECTION_HANDLE* Get_Connection_Handle( uint8_t Index );
extern void Remove_Connection_Index( uint8_t Index );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
//#define RECONFIG_DURING_STAND_BY


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static STANDBY_CONFIG StandbyConfig;
static uint32_t StandbyConfigTimeout = 0;


/****************************************************************/
/* Enter_Standby_Mode()        	 								*/
/* Location: 					 								*/
/* Purpose: Put the controller in stand-by mode.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Enter_Standby_Mode( void )
{
	BLE_STATES state = Get_BLE_State( );

	if( state != STANDBY_STATE )
	{
		uint8_t FunStat = FALSE;

		switch( state )
		{
		case BLE_INITIAL_SETUP_DONE:
			StandbyConfig.BaseStep = DISABLE_ADVERTISING;
			FunStat = TRUE;
			break;

		case CONFIG_ADVERTISING:
		case ADVERTISING_STATE:
			FunStat = Exit_Advertising_Mode( state );
			if ( FunStat )
			{
				StandbyConfig.BaseStep = DISABLE_ADVERTISING;
			}
			break;

		case CONFIG_SCANNING:
		case SCANNING_STATE:
			FunStat = Exit_Scanning_Mode( state );
			if ( FunStat )
			{
				StandbyConfig.BaseStep = DISABLE_SCANNING;
			}
			break;

		case CONFIG_INITIATING:
		case INITIATING_STATE:
			FunStat = Exit_Initiating_Mode( state );
			if ( FunStat )
			{
				StandbyConfig.BaseStep = DISABLE_INITIATING;
			}
			break;

		case CONFIG_STANDBY:
			/* Already in stand-by configuration, do not restart */
			FunStat = FALSE;

			/* Except if we are receiving a call back from a Disconnections complete */
			if( ( StandbyConfig.BaseStep == DISABLE_ALL_CONNECTIONS ) &&
					( StandbyConfig.Actual == WAIT_OPERATION ) )
			{
				CONNECTION_HANDLE* ConnHandlePtr = Get_Connection_Handle( StandbyConfig.ConnHandleCounter );

				/* The last disconnection was successful, so a new disconnection can take place */
				if( ConnHandlePtr != NULL )
				{
					if( ConnHandlePtr->Status == CONN_HANDLE_FREE )
					{
						FunStat = TRUE;
					}else if( ConnHandlePtr->Status == CONN_HANDLE_FAILED )
					{
						FunStat = TRUE;
						StandbyConfig.ConnHandleCounter++;
						ConnHandlePtr->Status = CONN_HANDLE_FULL;
					}
				}
			}
			break;

		case CONNECTION_STATE:
			FunStat = TRUE;
			StandbyConfig.ConnHandleCounter = Get_Max_Number_Of_Connections( );
			StandbyConfig.BaseStep = DISABLE_ALL_CONNECTIONS;
			break;

		case SYNCHRONIZATION_STATE:
		case ISOCHRONOUS_BROADCASTING_STATE:
			/* TODO: to be implemented */
			FunStat = FALSE;
			break;

		default: break;
		}

		if ( FunStat )
		{
			StandbyConfig.Actual = StandbyConfig.BaseStep;
			Set_BLE_State( CONFIG_STANDBY );
		}
	}
}


/****************************************************************/
/* Standby_Config()        	 									*/
/* Location: 					 								*/
/* Purpose: Do the housekeeping to enter standby mode.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
int8_t Standby_Config( void )
{
	switch( StandbyConfig.Actual )
	{
	case DISABLE_ADVERTISING:
		StandbyConfigTimeout = 0;
		StandbyConfig.Actual = WAIT_OPERATION;
		HCI_LE_Set_Advertising_Enable( FALSE, &LE_Set_Advertising_Enable_Complete, NULL );
		break;

	case DISABLE_SCANNING:
		StandbyConfigTimeout = 0;
		StandbyConfig.Actual = WAIT_OPERATION;
		HCI_LE_Set_Scan_Enable( FALSE, FALSE, &LE_Set_Scan_Enable_Complete, NULL );
		break;

	case DISABLE_INITIATING:
		StandbyConfigTimeout = 0;
		StandbyConfig.Actual = WAIT_OPERATION;
		HCI_LE_Create_Connection_Cancel( &LE_Create_Connection_Cancel_Complete, NULL );
		break;

	case DISABLE_ALL_CONNECTIONS:
	{
		CONNECTION_HANDLE* ConnHandle;
		while ( StandbyConfig.ConnHandleCounter )
		{
			StandbyConfig.ConnHandleCounter--;
			ConnHandle = Get_Connection_Handle( StandbyConfig.ConnHandleCounter );
			if( ( ConnHandle != NULL ) && ( ConnHandle->Status != CONN_HANDLE_FREE ) )
			{
				StandbyConfigTimeout = 0;
				StandbyConfig.Actual = WAIT_OPERATION;
				HCI_Disconnect( ConnHandle->Handle, REMOTE_USER_TERMINATED_CONNECTION, &Disconnect_Status );
				return (FALSE);
			}
		}

		/* Here we assume all disconnections were done */
		for( uint8_t i = Get_Max_Number_Of_Connections(); i > 0; i-- )
		{
			Remove_Connection_Index( i - 1 );
		}
		StandbyConfig.BaseStep = SEND_RESET_CMD;
		StandbyConfig.Actual = SEND_RESET_CMD;
	}
	break;

	case SEND_RESET_CMD:
		StandbyConfigTimeout = 0;
		StandbyConfig.BaseStep = SEND_RESET_CMD;
#ifdef RECONFIG_DURING_STAND_BY
		StandbyConfig.Actual = WAIT_OPERATION;
		HCI_Reset( &HCI_Reset_Complete, NULL );
#else
		StandbyConfig.Actual = SEND_STANDBY_CMD;
#endif
		break;

	case RECONFIG_VENDOR_SPECIFC:
		if( Vendor_Specific_Init() )
		{
			StandbyConfig.Actual = REINIT_BLE;
		}
		break;

	case REINIT_BLE:
		if( BLE_Init(  ) )
		{
			StandbyConfig.Actual = SEND_STANDBY_CMD;
		}
		break;

	case SEND_STANDBY_CMD:
		StandbyConfigTimeout = 0;
		StandbyConfig.Actual = WAIT_OPERATION_STBY;
		ACI_Hal_Device_Standby( &Hal_Device_Standby_Event, NULL );
		break;

	case END_STANDBY_CONFIG:
		/* Cancels any ongoing controller's function shared by the host */
		Hosted_Functions_Enter_Standby( );
		HCI_Reset_Transport_Layer( );
		return (TRUE);
		break;

	case WAIT_OPERATION:
		if( TimeBase_DelayMs( &StandbyConfigTimeout, 1500, TRUE ) )
		{
			Hosted_Functions_Enter_Standby( );

			StandbyConfig.Actual = StandbyConfig.BaseStep;
		}
		break;

	case WAIT_OPERATION_STBY:
		if( TimeBase_DelayMs( &StandbyConfigTimeout, 1500, TRUE ) )
		{
			/* We should finish anyway because the controller may fail to respond after
			 * he entered stand-by */
			StandbyConfig.Actual = END_STANDBY_CONFIG;
		}
		break;

	default:
		break;
	}

	return (FALSE);
}


/****************************************************************/
/* Standby()        	   										*/
/* Location: 					 								*/
/* Purpose: Standing by											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Standby( void )
{

}


/****************************************************************/
/* LE_Create_Connection_Cancel_Complete()    					*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Create_Connection_Cancel_Complete( CONTROLLER_ERROR_CODES Status )
{
	if( StandbyConfig.Actual == WAIT_OPERATION )
	{
		StandbyConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? SEND_RESET_CMD : StandbyConfig.BaseStep;
	}
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
	if( StandbyConfig.Actual == WAIT_OPERATION )
	{
		StandbyConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? SEND_RESET_CMD : StandbyConfig.BaseStep;
	}
}


/****************************************************************/
/* LE_Set_Advertising_Enable_Complete()        	   				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	if( StandbyConfig.Actual == WAIT_OPERATION )
	{
		StandbyConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? SEND_RESET_CMD : StandbyConfig.BaseStep;
	}
}


/****************************************************************/
/* Disconnect_Status()        	    			   				*/
/* Location: 					 								*/
/* Purpose: Status callback.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Disconnect_Status( CONTROLLER_ERROR_CODES Status )
{
	if( ( StandbyConfig.Actual == WAIT_OPERATION ) && ( Status != COMMAND_SUCCESS ) )
	{
		StandbyConfig.Actual = StandbyConfig.BaseStep;
	}
}


/****************************************************************/
/* HCI_Reset_Complete()        	    			   				*/
/* Location: 					 								*/
/* Purpose: Complete callback.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Reset_Complete( CONTROLLER_ERROR_CODES Status )
{
	if( StandbyConfig.Actual == WAIT_OPERATION )
	{
		StandbyConfig.Actual = ( Status == COMMAND_SUCCESS ) ? RECONFIG_VENDOR_SPECIFC : StandbyConfig.BaseStep;
	}
}


/****************************************************************/
/* Hal_Device_Standby_Event()        	    	   				*/
/* Location: 					 								*/
/* Purpose: Called to indicate the status of standby command.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_Device_Standby_Event( CONTROLLER_ERROR_CODES Status )
{
	if( StandbyConfig.Actual == WAIT_OPERATION_STBY )
	{
		if( Status == COMMAND_SUCCESS )
		{
			StandbyConfig.Actual = END_STANDBY_CONFIG;
		}else
		{
			StandbyConfig.Actual = SEND_STANDBY_CMD;
		}
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
