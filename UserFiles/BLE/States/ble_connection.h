

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
uint8_t Get_Max_Number_Of_Connections( void );
uint8_t Get_Number_Of_Active_Connections( void );


#endif /* BLE_CONNECTION_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
