

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


/****************************************************************/
/* Type Defines 					                            */
/****************************************************************/
typedef void (*EncryptCallBack)(uint8_t EncryptedData[16], uint8_t status);
typedef void (*StatusCallBack)(uint8_t status);


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t Generate_Device_Addresses( SUPPORTED_COMMANDS* HCI_Sup_Cmd, IRK_TYPE* IRK, uint8_t Update_Static_ADDR );
uint8_t AES_128_Encrypt( SUPPORTED_COMMANDS* HCI_Sup_Cmd, uint8_t Key[16], uint8_t Plaintext_Data[16], EncryptCallBack CallBack );
uint8_t Resolve_Private_Address( SUPPORTED_COMMANDS* HCI_Sup_Cmd, BD_ADDR_TYPE* PrivateAddress, IRK_TYPE* IRK, StatusCallBack CallBack );
BD_ADDR_TYPE* Get_Public_Device_Address( void );
BD_ADDR_TYPE* Get_Static_Device_Address( void );
BD_ADDR_TYPE* Get_Private_Device_Address( uint8_t resolvable );
IRK_TYPE* Get_Default_IRK( void );
uint8_t Clear_White_List( StatusCallBack CallBack );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLE_UTILS_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
