

#ifndef BLE_UTILS_H_
#define BLE_UTILS_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "Types.h"
#include "ble_states.h"
#include "gap.h"
#include "security_manager.h"
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
BD_ADDR_TYPE* Generate_Device_Address( SUPPORTED_COMMANDS* HCI_Sup_Cmd, RANDOM_ADDRESS_TYPE AddrType, IRK_TYPE* IRK, uint8_t Token );
uint8_t AES_128_Encrypt( SUPPORTED_COMMANDS* HCI_Sup_Cmd, uint8_t Key[16], uint8_t Plaintext_Data[16], EncryptCallBack CallBack );
uint8_t Resolve_Private_Address( SUPPORTED_COMMANDS* HCI_Sup_Cmd, BD_ADDR_TYPE* PrivateAddress, IRK_TYPE* IRK, StatusCallBack CallBack );
GET_BD_ADDR Get_Public_Device_Address( void );
GET_BD_ADDR Get_Static_Random_Device_Address( void );
IDENTITY_ADDRESS Get_Identity_Address( PEER_ADDR_TYPE Type );
LE_BD_ADDR_TYPE* Get_LE_Bluetooth_Device_Address( void );
uint8_t Get_Max_Advertising_Data_Length( void );
uint8_t Get_Max_Scan_Response_Data_Length( void );
uint8_t Check_Advertising_Parameters( ADVERTISING_PARAMETERS* AdvPar );
void Set_Advertising_HostData( ADVERTISING_PARAMETERS* AdvPar );
uint8_t Check_NULL_IRK( IRK_TYPE* IRKPtr );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLE_UTILS_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
