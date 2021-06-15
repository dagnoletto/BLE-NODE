

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
typedef struct
{
	uint8_t Status;
	BD_ADDR_TYPE* Ptr;
}GET_BD_ADDR;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t Generate_Device_Addresses( SUPPORTED_COMMANDS* HCI_Sup_Cmd, IRK_TYPE* IRK );
uint8_t AES_128_Encrypt( SUPPORTED_COMMANDS* HCI_Sup_Cmd, uint8_t Key[16], uint8_t Plaintext_Data[16], EncryptCallBack CallBack );
uint8_t Resolve_Private_Address( SUPPORTED_COMMANDS* HCI_Sup_Cmd, BD_ADDR_TYPE* PrivateAddress, IRK_TYPE* IRK, StatusCallBack CallBack );
GET_BD_ADDR Get_Public_Device_Address( void );
BD_ADDR_TYPE* Get_Private_Device_Address( uint8_t resolvable );
LE_BD_ADDR_TYPE* Get_LE_Bluetooth_Device_Address( void );
IRK_TYPE* Get_Default_IRK( void );
uint8_t Get_Max_Advertising_Data_Length( void );
uint8_t Get_Max_Scan_Response_Data_Length( void );
uint8_t LE_Write_Address( ADDRESS_TYPE AddressType, uint8_t Data[7] );
uint8_t* LE_Read_Address( ADDRESS_TYPE AddressType );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLE_UTILS_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
