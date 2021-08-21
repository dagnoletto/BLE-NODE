

#ifndef BLE_SCANNING_H_
#define BLE_SCANNING_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "link_layer.h"
#include "gap.h"


/****************************************************************/
/* Type Defines					                                */
/****************************************************************/
typedef struct
{
	uint16_t LE_Scan_Interval;
	uint16_t LE_Scan_Window;
	LE_SCAN_TYPE LE_Scan_Type;
	OWN_ADDR_TYPE Own_Address_Type;
	RANDOM_ADDRESS_TYPE Own_Random_Address_Type; /* Type of random address when Own_Address_Type == OWN_RANDOM_DEV_ADDR */
	uint8_t Scanning_Filter_Policy;
	uint8_t Privacy; /* TRUE / FALSE for peripheral privacy */
	uint32_t Counter;
	GAP_LE_ROLE Role;
}SCANNING_PARAMETERS;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t Check_Scanning_Parameters( SCANNING_PARAMETERS* ScanPar );


#endif /* BLE_SCANNING_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
