

#ifndef BLE_INITIATING_H_
#define BLE_INITIATING_H_


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
typedef struct
{
	uint16_t LE_Scan_Interval;
	uint16_t LE_Scan_Window;
	uint8_t Initiator_Filter_Policy;
	ADDRESS_TYPE Peer_Address_Type;
	BD_ADDR_TYPE Peer_Address;
	OWN_ADDR_TYPE Own_Address_Type;
	RANDOM_ADDRESS_TYPE Own_Random_Address_Type; /* Type of random address when Own_Address_Type == OWN_RANDOM_DEV_ADDR */
	uint16_t Connection_Interval_Min;
	uint16_t Connection_Interval_Max;
	uint16_t Connection_Latency;
	uint16_t Supervision_Timeout;
	uint16_t Min_CE_Length;
	uint16_t Max_CE_Length;
	uint8_t Privacy; /* TRUE / FALSE for initiator privacy */
	uint32_t Counter;
	OWN_ADDR_TYPE Original_Own_Address_Type;
}INITIATING_PARAMETERS;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t Check_Initiating_Parameters( INITIATING_PARAMETERS* InitPar );


#endif /* BLE_INITIATING_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
