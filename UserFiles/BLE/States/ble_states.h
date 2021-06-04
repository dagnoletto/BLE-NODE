

#ifndef BLE_STATES_H_
#define BLE_STATES_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "vendor_specific.h"
#include "link_layer.h"


/****************************************************************/
/* Type Defines (Packet structure)                              */
/****************************************************************/
typedef enum
{
	RESET_CONTROLLER,
	VENDOR_SPECIFIC_INIT,
	BLE_INITIAL_SETUP,
	STANDBY_STATE,
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


#endif /* BLE_STATES_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
