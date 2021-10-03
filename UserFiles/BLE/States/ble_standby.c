

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
	SEND_STANDBY_CMD,
	END_STANDBY_CONFIG,
	FAILED_STANDBY_CONFIG,
	WAIT_OPERATION,
}STANDBY_CONFIG_STEPS;


typedef struct
{
	STANDBY_CONFIG_STEPS Actual;
	BLE_STATES ExitingState;
}STANDBY_CONFIG;


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
int8_t Standby_Config( void );
void Standby( void );
static void Hal_Device_Standby_Event( CONTROLLER_ERROR_CODES Status );


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
static STANDBY_CONFIG StandbyConfig;


/****************************************************************/
/* Enter_Standby_Mode()        	 								*/
/* Location: 					 								*/
/* Purpose: Put the controller in stand-by mode.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Enter_Standby_Mode( void )
{
	StandbyConfig.ExitingState = Get_BLE_State( );
	StandbyConfig.Actual = SEND_STANDBY_CMD;

	switch( StandbyConfig.ExitingState )
	{
	case ADVERTISING_STATE:
	case SCANNING_STATE:
	case INITIATING_STATE:
	case CONNECTION_STATE:
	case SYNCHRONIZATION_STATE:
	case ISOCHRONOUS_BROADCASTING_STATE:
		/* TODO: algumas funções tem máquinas de estado para transição, ou seja, estas funções
		 * devem ir do início ao fim, caso contrário a próxima chamada de função vai "zoar" com a mesma
		 * Por isso, avaliar quais são as funções e esperar as mesmas terminarem ou prover uma maneira
		 * de resetar estas funções por aqui antes de colocar o sistema em stand-by. */
		/* TODO: liberar memória alocada das funções de advertisement, scanning e etc? */
		Set_BLE_State( CONFIG_STANDBY );
		break;

	default:
		Set_BLE_State( CONFIG_STANDBY );
		break;

	}

	return (TRUE);
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
	static uint32_t StandbyConfigTimeout = 0;
	//TODO: adicionar aqui código para desligar comandos scanning, advertising e initiating

	switch( StandbyConfig.Actual )
	{
	case SEND_STANDBY_CMD:
		StandbyConfigTimeout = 0;
		StandbyConfig.Actual = ACI_Hal_Device_Standby( &Hal_Device_Standby_Event, NULL ) ? WAIT_OPERATION : SEND_STANDBY_CMD;
		break;

	case END_STANDBY_CONFIG:
		StandbyConfig.Actual = SEND_STANDBY_CMD;
		return (TRUE);
		break;

	case FAILED_STANDBY_CONFIG:
		return (-1); /* Failed condition */
		break;

	case WAIT_OPERATION:
		if( TimeBase_DelayMs( &StandbyConfigTimeout, 500, TRUE ) )
		{
			StandbyConfig.Actual = FAILED_STANDBY_CONFIG;
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
