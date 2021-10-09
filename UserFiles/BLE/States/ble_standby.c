

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
	SEND_STANDBY_CMD,
	END_STANDBY_CONFIG,
	WAIT_OPERATION,
}STANDBY_CONFIG_STEPS;


typedef struct
{
	STANDBY_CONFIG_STEPS Actual;
	STANDBY_CONFIG_STEPS BaseStep;
}STANDBY_CONFIG;


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
int8_t Standby_Config( void );
void Standby( void );
static void LE_Set_Scan_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void Hal_Device_Standby_Event( CONTROLLER_ERROR_CODES Status );


/****************************************************************/
/* extern functions declaration                                 */
/****************************************************************/
extern void Set_BLE_State( BLE_STATES NewBLEState );
extern uint8_t Exit_Advertising_Mode( BLE_STATES CurrentState );
extern uint8_t Exit_Scanning_Mode( BLE_STATES CurrentState );
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
			break;

		case CONNECTION_STATE:
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
	//TODO: adicionar aqui código para desligar comandos scanning, advertising e initiating

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

	case SEND_STANDBY_CMD:
		StandbyConfigTimeout = 0;
		StandbyConfig.Actual = WAIT_OPERATION;
		ACI_Hal_Device_Standby( &Hal_Device_Standby_Event, NULL );
		break;

	case END_STANDBY_CONFIG:
		StandbyConfig.Actual = StandbyConfig.BaseStep;
		return (TRUE);
		break;

	case WAIT_OPERATION:
		if( TimeBase_DelayMs( &StandbyConfigTimeout, 500, TRUE ) )
		{
			HCI_COMMAND_OPCODE OpCode;

			Hosted_Functions_Enter_Standby( );

			OpCode.Val = HCI_LE_SET_SCAN_ENABLE;
			Clear_Command_CallBack( OpCode );

			OpCode.Val = HCI_LE_SET_ADVERTISING_ENABLE;
			Clear_Command_CallBack( OpCode );

			StandbyConfig.Actual = StandbyConfig.BaseStep;
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
		StandbyConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? SEND_STANDBY_CMD : StandbyConfig.BaseStep;
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
		StandbyConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? SEND_STANDBY_CMD : StandbyConfig.BaseStep;
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
	if( StandbyConfig.Actual == WAIT_OPERATION )
	{
		if( Status == COMMAND_SUCCESS )
		{
			/* Cancels any ongoing controller's function shared by the host */
			Hosted_Functions_Enter_Standby( );
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
