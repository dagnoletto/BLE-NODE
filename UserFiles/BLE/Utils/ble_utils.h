

#ifndef BLE_UTILS_H_
#define BLE_UTILS_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "Types.h"
#include "gap.h"
#include "security_manager.h"
#include "main.h"


/****************************************************************/
/* Defines 					                            		*/
/****************************************************************/


/****************************************************************/
/* Type Defines 					                            */
/****************************************************************/
typedef void (*EncryptCallBack)(uint8_t EncryptedData[16], CONTROLLER_ERROR_CODES status);
typedef void (*StatusCallBack)(uint8_t comparisonStatus, CONTROLLER_ERROR_CODES status);


typedef struct
{
	uint8_t Status;
	BD_ADDR_TYPE* AddrPtr;
}GET_BD_ADDR;


typedef enum
{
	LOCAL_PUBLIC_DEV_ADDR    	  = 0x00, /* Public Device Address */
	LOCAL_RANDOM_DEV_ADDR	   	  = 0x01, /* Random Device Address */
	LOCAL_PUBLIC_IDENTITY_ADDR 	  = 0x02, /* Public Identity Address */
	LOCAL_RANDOM_IDENTITY_ADDR 	  = 0x03, /* Random (static) Identity Address */
	LOCAL_RPA_PUBLIC_IDENTITY	  = 0x04, /* Resolvable Private Address based on Public Identity Address */
	LOCAL_RPA_RANDOM_IDENTITY	  = 0x05  /* Resolvable Private Address based on Random (static) Identity Address */
}LOCAL_ADDRESS_TYPE;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
BD_ADDR_TYPE* Generate_Device_Address( SUPPORTED_COMMANDS* HCI_Sup_Cmd, RANDOM_ADDRESS_TYPE AddrType, IRK_TYPE* IRK, uint8_t Token );
void Cancel_Device_Address_Generation( void );
uint8_t Resolve_Private_Address( SUPPORTED_COMMANDS* HCI_Sup_Cmd, BD_ADDR_TYPE* PrivateAddress, IRK_TYPE* IRK, uint8_t Token, StatusCallBack CallBack );
void Cancel_Private_Address_Resolution( void );
uint8_t AES_128_Encrypt( SUPPORTED_COMMANDS* HCI_Sup_Cmd, uint8_t Key[16], uint8_t Plaintext_Data[16], EncryptCallBack CallBack );
GET_BD_ADDR Get_Public_Device_Address( void );
GET_BD_ADDR Get_Static_Random_Device_Address( void );
IDENTITY_ADDRESS Get_Identity_Address( PEER_ADDR_TYPE Type );
LE_BD_ADDR_TYPE* Get_LE_Bluetooth_Device_Address( void );
uint8_t Check_NULL_IRK( IRK_TYPE* IRKPtr );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLE_UTILS_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
