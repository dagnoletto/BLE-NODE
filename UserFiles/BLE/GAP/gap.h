

#ifndef GAP_H_
#define GAP_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "appearance_values.h"


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
/* Assigned numbers and GAP */
#define FLAGS_TYPE 						   				0x01 /* Flags */
#define INCOMPLT_LIST_16_BIT_SVC_CLASS_UUIDS_TYPE 		0x02 /* Incomplete List of 16-bit Service Class UUIDs */
#define COMPLETE_LIST_16_BIT_SVC_CLASS_UUIDS_TYPE 		0x03 /* Complete List of 16-bit Service Class UUIDs */
#define INCOMPLT_LIST_32_BIT_SVC_CLASS_UUIDS_TYPE		0x04 /* Incomplete List of 32-bit Service Class UUIDs */
#define COMPLETE_LIST_32_BIT_SVC_CLASS_UUIDS_TYPE 		0x05 /* Complete List of 32-bit Service Class UUIDs */
#define INCOMPLT_LIST_128_BIT_SVC_CLASS_UUIDS_TYPE   	0x06 /* Incomplete List of 128-bit Service Class UUIDs */
#define COMPLETE_LIST_128_BIT_SVC_CLASS_UUIDS_TYPE   	0x07 /* Complete List of 128-bit Service Class UUIDs */
#define SHORTENED_LOCAL_NAME_TYPE						0x08 /* Shortened Local Name */
#define COMPLETE_LOCAL_NAME_TYPE 						0x09 /* Complete Local Name */
#define TX_POWER_LEVEL_TYPE 							0x0A /* Tx Power Level */

#define CLASS_OF_DEVICE_TYPE 							0x0D /* Class of Device */
#define SIMPLE_PAIRING_HASH_C_TYPE 						0x0E /* Simple Pairing Hash C */
#define SIMPLE_PAIRING_HASH_C_192_TYPE 					0x0E /* Simple Pairing Hash C-192 */
#define SIMPLE_PAIRING_RANDOMIZER_R_TYPE 				0x0F /* Simple Pairing Randomizer R */
#define SIMPLE_PAIRING_RANDOMIZER_R_192_TYPE 			0x0F /* Simple Pairing Randomizer R-192 */
#define DEVICE_ID_TYPE 									0x10 /* Device ID */
#define SECURITY_MANAGER_TK_VALUE_TYPE 					0x10 /* Security Manager TK Value */
#define SECURITY_MANAGER_OUT_OF_BAND_FLAGS_TYPE 		0x11 /* Security Manager Out of Band Flags */
#define SLAVE_CONNECTION_INTERVAL_RANGE_TYPE			0x12 /* Slave Connection Interval Range */

#define LIST_OF_16_BIT_SVC_SOLICITATION_UUIDS_TYPE 		0x14 /* List of 16-bit Service Solicitation UUIDs */
#define LIST_OF_128_BIT_SVC_SOLICITATION_UUIDS_TYPE 	0x15 /* List of 128-bit Service Solicitation UUIDs */
#define SERVICE_DATA_TYPE 								0x16 /* Service Data */
#define SERVICE_DATA_16_BIT_UUID_TYPE 					0x16 /* Service Data - 16-bit UUID */
#define PUBLIC_TARGET_ADDRESS_TYPE 						0x17 /* Public Target Address */
#define RANDOM_TARGET_ADDRESS_TYPE 						0x18 /* Random Target Address */
#define APPEARANCE_TYPE 								0x19 /* Appearance */
#define ADVERTISING_INTERVAL_TYPE 						0x1A /* Advertising Interval */
#define LE_BLUETOOTH_DEVICE_ADDRESS_TYPE 				0x1B /* LE Bluetooth Device Address */
#define LE_ROLE_TYPE 									0x1C /* LE Role */
#define SIMPLE_PAIRING_HASH_C_256_TYPE 					0x1D /* Simple Pairing Hash C-256 */
#define SIMPLE_PAIRING_RANDOMIZER_R_256_TYPE 			0x1E /* Simple Pairing Randomizer R-256 */
#define LIST_OF_32_BIT_SVC_SOLICITATION_UUIDS_TYPE 		0x1F /* List of 32-bit Service Solicitation UUIDs */
#define SERVICE_DATA_32_BIT_UUID_TYPE 					0x20 /* Service Data - 32-bit UUID */
#define SERVICE_DATA_128_BIT_UUID_TYPE 					0x21 /* Service Data - 128-bit UUID */
#define LE_SECURE_CONN_CONFIRMATION_VALUE_TYPE 			0x22/* LE Secure Connections Confirmation Value */
#define LE_SECURE_CONN_RANDOM_VALUE_TYPE 				0x23 /* LE Secure Connections Random Value */
#define URI_TYPE 										0x24 /* URI */
#define INDOOR_POSITIONING_TYPE 						0x25 /* Indoor Positioning */
#define TRANSPORT_DISCOVERY_DATA_TYPE 					0x26 /* Transport Discovery Data */
#define LE_SUPPORTED_FEATURES_TYPE 						0x27 /* LE Supported Features */
#define CHANNEL_MAP_UPDATE_INDICATION_TYPE 				0x28 /* Channel Map Update Indication */
#define PB_ADV_TYPE										0x29 /* PB-ADV */
#define MESH_MESSAGE_TYPE 								0x2A /* Mesh Message */
#define MESH_BEACON_TYPE 								0x2B /* Mesh Beacon */
#define BIGINFO_TYPE 									0x2C /* BIGInfo */
#define BROADCAST_CODE_TYPE 							0x2D /* Broadcast_Code */

#define THREE_D_INFORMATION_DATA_TYPE 					0x3D /* 3D Information Data */

#define MANUFACTURER_SPECIFIC_DATA_TYPE 				0xFF /* Manufacturer Specific Data */


/****************************************************************/
/* Type Defines 				               		            */
/****************************************************************/
typedef enum
{
	BROADCASTER,
	OBSERVER,
	PERIPHERAL,
	CENTRAL,
}GAP_LE_ROLE;


typedef enum
{
	NON_DISCOVERABLE_MODE,
	LIMITED_DISCOVERABLE_MODE,
	GENERAL_DISCOVERABLE_MODE
}GAP_DISCOVERY_MODE;


#define NO_SPECIFIC_MINIMUM 0xFFFF /* Used in Slave_Conn_Interval_Range_Type, connIntervalmin */
#define NO_SPECIFIC_MAXIMUM 0xFFFF /* Used in Slave_Conn_Interval_Range_Type, connIntervalmax */


/* Data Types according to Supplement to the Bluetooth Core Specification CSS_v9 2019-12-31 */
typedef struct
{
	uint8_t length; /* size of type + size of Flags */
	uint8_t type;   /* FLAGS_TYPE */
	union
	{
		struct Flags_Bits
		{
			uint8_t LE_Limited_Discoverable_Mode				:1;
			uint8_t LE_General_Discoverable_Mode				:1;
			uint8_t BR_EDR_Not_Supported						:1;	/* BR/EDR Not Supported. Bit 37 of LMP Feature Mask Definitions (Page 0). */
			uint8_t Simul_LE_BR_EDR_Same_Dev_Capable_Controller :1; /* Simultaneous LE and BR/EDR to Same Device Capable (Controller).
		 	 	 	 	 	 	 	 	 	 	 	 	 	 	   Bit 49 of LMP Feature Mask Definitions (Page 0). */
			uint8_t Simul_LE_BR_EDR_Same_Dev_Capable_Host		:1; /* Simultaneous LE and BR/EDR to Same Device Capable (Host).
															 	   Bit 66 of LMP Feature Mask Definitions (Page 1). */
			uint8_t Reserved :3;
		}__attribute__((packed)) Bits;
		uint8_t Val;
	}__attribute__((packed)) Flags;
}__attribute__((packed)) Flags_Type;


typedef struct
{
	uint8_t length; /* size of type + size of local_Name[] */
	uint8_t type; /* Can be SHORTENED_LOCAL_NAME_TYPE or COMPLETE_LOCAL_NAME_TYPE */
	uint8_t local_Name[]; /* Maximum of 248 bytes coded according to the UTF-8 standard */
}__attribute__((packed)) Local_Name_Type;


typedef struct
{
	uint8_t length; /* size of type + size of Tx_Power_Level */
	uint8_t type; /* TX_POWER_LEVEL_TYPE */
	int8_t Tx_Power_Level;
}__attribute__((packed)) Tx_Power_Level_Type;


typedef struct
{
	uint8_t length; /* size of type + size of connIntervalmin + size of connIntervalmax */
	uint8_t type; /* SLAVE_CONNECTION_INTERVAL_RANGE_TYPE */
	uint16_t connIntervalmin; /* Conn_Interval_Min * 1.25 ms. Value of 0xFFFF (NO_SPECIFIC_MINIMUM) indicates no specific minimum. */
	uint16_t connIntervalmax; /* Conn_Interval_Max * 1.25 ms. Value of 0xFFFF (NO_SPECIFIC_MAXIMUM) indicates no specific maximum. */
}__attribute__((packed)) Slave_Conn_Interval_Range_Type;


typedef struct
{
	uint8_t length; /* size of type + size of Public_Target_Address[] */
	uint8_t type;   /* PUBLIC_TARGET_ADDRESS_TYPE */
	BD_ADDR_TYPE Public_Target_Address[]; /* One or more addresses can be loaded here */
}__attribute__((packed)) Public_Target_Address_Type;


typedef struct
{
	uint8_t length; /* size of type + size of Random_Target_Address[] */
	uint8_t type;   /* RANDOM_TARGET_ADDRESS_TYPE */
	BD_ADDR_TYPE Random_Target_Address[]; /* One or more addresses can be loaded here */
}__attribute__((packed)) Random_Target_Address_Type;


typedef struct
{
	uint8_t length; /* size of type + size of Appearance */
	uint8_t type;   /* APPEARANCE_TYPE */
	Appearance_Value Appearance;
}__attribute__((packed)) Appearance_Type;


typedef struct
{
	uint8_t length; /* size of type + size of LE_BD_Address */
	uint8_t type;   /* LE_BLUETOOTH_DEVICE_ADDRESS_TYPE */
	LE_BD_ADDR_TYPE LE_BD_Address;
}__attribute__((packed)) LE_BD_Address_Type;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t Load_Flags( Flags_Type* Ptr, int16_t ArraySize,
		GAP_LE_ROLE Role, GAP_DISCOVERY_MODE DiscoveryMode );
uint8_t Load_Local_Name( Local_Name_Type* Ptr, int16_t ArraySize );
uint8_t Load_Tx_Power_Level( Tx_Power_Level_Type* Ptr, int16_t ArraySize, int8_t Tx_Power_Level );
uint8_t Load_Slave_Conn_Interval_Range( Slave_Conn_Interval_Range_Type* Ptr, int16_t ArraySize,
		uint16_t connIntervalmin, uint16_t connIntervalmax );
uint8_t Load_Public_Target_Address( Public_Target_Address_Type* Ptr, int16_t ArraySize,
		BD_ADDR_TYPE BDAddress[], uint8_t NumberOfAddresses );
uint8_t Load_Random_Target_Address( Random_Target_Address_Type* Ptr, int16_t ArraySize,
		BD_ADDR_TYPE BDAddress[], uint8_t NumberOfAddresses );
uint8_t Load_Appearance( Appearance_Type* Ptr, int16_t ArraySize, GAP_APPEARANCE Appearance );
uint8_t Load_LE_Bluetooth_Device_Address( LE_BD_Address_Type* Ptr, int16_t ArraySize );
void* Get_AD_Type_Ptr( uint8_t AD_Type, uint8_t Ad_or_Scan_Ptr[], int16_t SizeOfData );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* GAP_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
