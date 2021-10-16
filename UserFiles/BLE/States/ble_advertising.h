

#ifndef BLE_ADVERTISING_H_
#define BLE_ADVERTISING_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "link_layer.h"
#include "gap.h"
#include "ble_utils.h"


/****************************************************************/
/* Type Defines					                                */
/****************************************************************/
typedef struct
{
	uint8_t Adv_Data_Length;
	uint8_t* Adv_Data_Ptr;
	uint8_t ScanRsp_Data_Length;
	uint8_t* Scan_Data_Ptr;
}ADV_SCAN_DATA;


typedef struct
{
	uint16_t Advertising_Interval_Min;
	uint16_t Advertising_Interval_Max;
	ADVERTISING_TYPE Advertising_Type;
	OWN_ADDR_TYPE Own_Address_Type;
	RANDOM_ADDRESS_TYPE Own_Random_Address_Type; /* Type of random address when Own_Address_Type == OWN_RANDOM_DEV_ADDR */
	PEER_ADDR_TYPE Peer_Address_Type;
	BD_ADDR_TYPE Peer_Address;
	ADV_CHANNEL_MAP Advertising_Channel_Map;
	uint8_t Advertising_Filter_Policy;
	uint16_t connIntervalmin; /* Only useful in connectable advertising: Conn_Interval_Min * 1.25 ms. Value of 0xFFFF indicates no specific minimum. */
	uint16_t connIntervalmax; /* Only useful in connectable advertising: Conn_Interval_Max * 1.25 ms. Value of 0xFFFF indicates no specific maximum. */
	uint8_t Privacy; /* TRUE / FALSE for peripheral privacy */
	uint32_t Counter;
	GAP_LE_ROLE Role;
	GAP_DISCOVERY_MODE DiscoveryMode;
	ADV_SCAN_DATA HostData;
	OWN_ADDR_TYPE Original_Own_Address_Type;
	RANDOM_ADDRESS_TYPE Original_Own_Random_Address_Type;
	BD_ADDR_TYPE Original_Peer_Address;
}ADVERTISING_PARAMETERS;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t Get_Max_Advertising_Data_Length( void );
uint8_t Get_Max_Scan_Response_Data_Length( void );
uint8_t Check_Advertising_Parameters( ADVERTISING_PARAMETERS* AdvPar );
void Set_Advertising_HostData( ADVERTISING_PARAMETERS* AdvPar );
uint8_t Get_Advertiser_Address( LOCAL_ADDRESS_TYPE* Type, BD_ADDR_TYPE* AdvA );


#endif /* BLE_ADVERTISING_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
