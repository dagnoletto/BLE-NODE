

#ifndef BLE_STATES_H_
#define BLE_STATES_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "vendor_specific.h"


/****************************************************************/
/* Type Defines (Packet structure)                              */
/****************************************************************/
typedef enum
{
	VENDOR_SPECIFIC_INIT 			= 0x0,
	BLE_INITIAL_SETUP	 			= 0x1,
	STANDBY_STATE		 			= 0x2,
	ADVERTISING_STATE	 			= 0x3,
	SCANNING_STATE		 			= 0x4,
	INITIATING_STATE	 			= 0x5,
	CONNECTION_STATE	 			= 0x6,
	SYNCHRONIZATION_STATE 			= 0x7,
	ISOCHRONOUS_BROADCASTING_STATE 	= 0x8
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
