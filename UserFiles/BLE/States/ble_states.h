

#ifndef BLE_STATES_H_
#define BLE_STATES_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "vendor_specific.h"
#include "link_layer.h"


/****************************************************************/
/* Type Defines (Packet structure)                              */
/****************************************************************/
typedef enum
{
	RESET_CONTROLLER,
	VENDOR_SPECIFIC_INIT,
	BLE_INITIAL_SETUP,
	STANDBY_STATE,
	CONFIG_ADVERTISING,
	ADVERTISING_STATE,
	SCANNING_STATE,
	INITIATING_STATE,
	CONNECTION_STATE,
	SYNCHRONIZATION_STATE,
	ISOCHRONOUS_BROADCASTING_STATE
}BLE_STATES;


typedef struct
{
	HCI_VERSION HCI_Version;
	uint16_t HCI_Revision;
	uint8_t LMP_PAL_Version;
	uint16_t Manufacturer_Name;
	uint16_t LMP_PAL_Subversion;
}BLE_VERSION_INFO;


typedef struct
{
	uint16_t Advertising_Interval_Min;
	uint16_t Advertising_Interval_Max;
	ADVERTISING_TYPE Advertising_Type;
	ADDRESS_TYPE Own_Address_Type;
	ADDRESS_TYPE Peer_Address_Type;
	BD_ADDR_TYPE Peer_Address;
	ADV_CHANNEL_MAP Advertising_Channel_Map;
	uint8_t Advertising_Filter_Policy;
	uint8_t Adv_Data_Length;
	uint8_t* Adv_Data_Ptr;
	uint8_t ScanRsp_Data_Length;
	uint8_t* Scan_Data_Ptr;
}ADVERTISING_PARAMETERS;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void Run_BLE( void );
BLE_STATES Get_BLE_State( void );
SUPPORTED_COMMANDS* Get_Supported_Commands( void );
SUPPORTED_FEATURES* Get_Supported_Features( void );
BLE_VERSION_INFO* Get_Local_Version_Information( void );
uint8_t Enter_Advertising_Mode( ADVERTISING_PARAMETERS* AdvPar );


#endif /* BLE_STATES_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
