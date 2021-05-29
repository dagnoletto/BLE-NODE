

#ifndef VENDOR_SPECIFIC_H_
#define VENDOR_SPECIFIC_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "vendor_specific_hci.h"
#include "Types.h"


/****************************************************************/
/* Type Defines 					                            */
/****************************************************************/
typedef struct
{
	BD_ADDR_TYPE Public_address; /* Bluetooth public address */
	uint8_t DIV[2];				 /* DIV used to derive CSRK */
	uint8_t ER[16];				 /* Encryption root key used to derive LTK and CSRK */
	uint8_t IR[16];				 /* Identity root key used to derive LTK and CSRK */
	uint8_t LLWithoutHost;		 /* Switch on/off Link Layer only mode */
	uint8_t Role;				 /* Select the BlueNRG-MS roles and mode configuration */
}__attribute__((packed)) CONFIG_DATA;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
BLE_STATUS Vendor_Specific_Init( void );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* VENDOR_SPECIFIC_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
