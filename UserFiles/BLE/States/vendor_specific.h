

#ifndef VENDOR_SPECIFIC_H_
#define VENDOR_SPECIFIC_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "vendor_specific_hci.h"
#include "Types.h"


/****************************************************************/
/* Defines 					                            		*/
/****************************************************************/
/* Link Layer Mode */
#define LL_WITH_HOST 0x0 /* The Bluenrg operates with Link Layer plus internal Host */
#define LL_ONLY		 0x1 /* The Bluenrg operates in Link Layer only mode */


/* Role Mode: BlueNRG roles and mode configuration */
#define SLAVE_AND_MASTER_6KB  		0x1 /* Slave and master Only one connection 6 kB of RAM retention */
#define SLAVE_AND_MASTER_12KB 		0x2 /* Slave and master Only one connection 12 kB of RAM retention */
#define MASTER_AND_SLAVE_8CON 		0x3 /* Master and slave Up to 8 connections 12 kB of RAM retention */
#define MASTER_AND_SLAVE_4CON 		0x4 /* Master and slave Up to 4 connections Simultaneous advertising and scanning */


/****************************************************************/
/* Type Defines 					                            */
/****************************************************************/
typedef void (*VS_Callback)(void* Data);


typedef struct
{
	BD_ADDR_TYPE Public_address; /* Bluetooth public address */
	uint8_t DIV[2];				 /* DIV used to derive CSRK */
	uint8_t ER[16];				 /* Encryption root key used to derive LTK and CSRK */
	uint8_t IR[16];				 /* Identity root key used to derive LTK and CSRK */
	uint8_t LLWithoutHost;		 /* Switch on/off Link Layer only mode */
	uint8_t Role;				 /* Select the BlueNRG-MS roles and mode configuration */
}CONFIG_DATA;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
/* TODO: more Vendor Specific config functions can be added, but for now Read_Config_Data and Write_Config_Data
 * offers access to all config fields while Read_Public_Address and Write_Public_Address accesses only public address */
BLE_STATUS Read_Config_Data( CONFIG_DATA* ConfigData, VS_Callback CallBackFun );
BLE_STATUS Write_Config_Data( CONFIG_DATA* ConfigData, VS_Callback CallBackFun );
BLE_STATUS Read_Public_Address( BD_ADDR_TYPE* Public_Address, VS_Callback CallBackFun );
BLE_STATUS Write_Public_Address( BD_ADDR_TYPE* Public_Address, VS_Callback CallBackFun );

void Vendor_Specific_Process( void );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* VENDOR_SPECIFIC_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
