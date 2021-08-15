

#ifndef BLE_STATES_H_
#define BLE_STATES_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "vendor_specific.h"
#include "link_layer.h"
#include "gap.h"
#include "ble_advertising.h"


/****************************************************************/
/* Type Defines 					                            */
/****************************************************************/
typedef enum
{
	RESET_CONTROLLER,
	VENDOR_SPECIFIC_INIT,
	BLE_INITIAL_SETUP,
	STANDBY_STATE,
	CONFIG_ADVERTISING,
	ADVERTISING_STATE,
	SCANNING_STATE,
	INITIATING_STATE,
	CONNECTION_STATE,
	SYNCHRONIZATION_STATE,
	ISOCHRONOUS_BROADCASTING_STATE
}BLE_STATES;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void Run_BLE( void );
BLE_STATES Get_BLE_State( void );
SUPPORTED_COMMANDS* Get_Supported_Commands( void );
SUPPORTED_FEATURES* Get_Supported_Features( void );
LOCAL_VERSION_INFORMATION* Get_Local_Version_Information( void );
uint8_t Enter_StandBy_Mode( void );
uint8_t Enter_Advertising_Mode( ADVERTISING_PARAMETERS* AdvPar );


#endif /* BLE_STATES_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
