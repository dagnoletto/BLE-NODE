

#ifndef MASTER_H_
#define MASTER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "hci.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef struct
{
	ADDRESS_TYPE Address_Type;
	BD_ADDR_TYPE Address;
	int8_t RSSI;
	struct
	{
		uint8_t Size;
		uint8_t Bytes[MAX_ADVERTISING_DATA_LENGTH];
	}AdvData;
	struct
	{
		uint8_t Size;
		uint8_t Bytes[MAX_SCAN_RESPONSE_DATA_LENGTH];
	}ScanRspData;
}SLAVE_ADV_INFO;


typedef struct
{
	SLAVE_ADV_INFO Adv;
	uint16_t Connection_Handle;
	LE_SUPPORTED_FEATURES SupFeatures;
	REMOTE_VERSION_INFORMATION Version;
}SLAVE_INFO;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void MasterNode( void );
void Client( void );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/
SLAVE_INFO SlaveInfo;


#endif /* MASTER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
