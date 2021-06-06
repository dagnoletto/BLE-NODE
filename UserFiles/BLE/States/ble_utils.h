

#ifndef BLE_UTILS_H_
#define BLE_UTILS_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "Types.h"
#include "main.h"


/****************************************************************/
/* Defines 					                            		*/
/****************************************************************/
#define DEFAULT_PUBLIC_ADDRESS { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 }


/****************************************************************/
/* Type Defines 					                            */
/****************************************************************/


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t Generate_Device_Addresses( SUPPORTED_COMMANDS* HCI_Sup_Cmd, uint8_t Update_Static_ADDR );
uint8_t Generate_Private_Device_Addresses( void );
BD_ADDR_TYPE* Get_Public_Device_Address( void );
BD_ADDR_TYPE* Get_Static_Device_Address( void );
BD_ADDR_TYPE* Get_Private_Device_Address( uint8_t resolvable );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLE_UTILS_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
