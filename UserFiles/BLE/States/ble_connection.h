

#ifndef BLE_CONNECTION_H_
#define BLE_CONNECTION_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "link_layer.h"
#include "gap.h"
#include "security_manager.h"


/****************************************************************/
/* Type Defines					                                */
/****************************************************************/


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void Master_Connection_Complete( LEEnhancedConnectionComplete* ConnCpltData );
void Slave_Connection_Complete( LEEnhancedConnectionComplete* ConnCpltData );
void Master_Disconnection_Complete( DisconnectionComplete* DisConnCpltData );
void Slave_Disconnection_Complete( DisconnectionComplete* DisConnCpltData );
uint8_t Get_Max_Number_Of_Connections( void );
uint8_t Get_Number_Of_Active_Connections( void );


#endif /* BLE_CONNECTION_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
