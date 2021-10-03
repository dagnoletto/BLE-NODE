

#ifndef APP_H_
#define APP_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "TimeFunctions.h"
#include "Bluenrg.h"
#include "ble_states.h"
#include "ble_utils.h"
#include "gap.h"
#include "security_manager.h"
#include "Master.h"
#include "Slave.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	WAIT_BLE_STANDBY,
	CONFIG_ADVERTISER,
	CONFIG_SCANNER,
	CONFIG_INITIATOR,
	RUN_ADVERTISER,
	RUN_SCANNER,
	RUN_INITIATOR,
	STOP_ADVERTISER,
	STOP_SCANNER,
	STOP_INITIATOR
}NODE_STATE;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void App_Init(void);
void App_Run(void);


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define BLE_SLAVE   0
#define BLE_MASTER  1
#define BLE_NODE BLE_SLAVE


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/
extern const BD_ADDR_TYPE MasterPublicAddress;
extern const BD_ADDR_TYPE SlavePublicAddress;


#endif /* APP_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
