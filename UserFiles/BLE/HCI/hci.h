

#ifndef HCI_H_
#define HCI_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


/****************************************************************/
/* Type Defines (Packet structure)                              */
/****************************************************************/
typedef union
{
	struct
	{
		uint16_t OCF :10; /* OpCode Command Field */
		uint16_t OGF : 6; /* OpCode Group Field   */
	};
	uint16_t Val;
}HCI_COMMAND_OPCODE;


typedef struct
{
	HCI_COMMAND_OPCODE OpCode;
	uint8_t Parameter_Total_Length;
	uint8_t Parameter[];
}__attribute__((packed)) HCI_COMMAND_PCKT;


typedef struct
{
	uint16_t BC_Flag : 2; /* Broadcast_Flag */
	uint16_t PB_Flag : 2; /* Packet_Boundary_Flag */
	uint16_t Handle  :12; /* Connection_Handle */
	uint16_t Data_Total_Length;
	uint8_t Data[];
}__attribute__((packed)) HCI_ACL_DATA_PCKT;


typedef struct
{
	uint16_t RFU 				: 2; /* Reserved for future use */
	uint16_t Packet_Status_Flag : 2;
	uint16_t Connection_Handle  :12; /* Connection_Handle */
	uint8_t Data_Total_Length;
	uint8_t Data[];
}__attribute__((packed)) HCI_SYNCHRONOUS_DATA_PCKT;


typedef struct
{
	uint8_t Event_Code;
	uint8_t Parameter_Total_Length;
	uint8_t Event_Parameter[];
}__attribute__((packed)) HCI_EVENT_PCKT;


typedef struct
{
	uint16_t RFU 				  : 1; /* Reserved for future use */
	uint16_t TS_Flag 			  : 1;
	uint16_t PB_Flag			  : 2;
	uint16_t Connection_Handle    :12; /* Connection_Handle */
	uint16_t RFU2 				  : 2; /* Reserved for future use */
	uint16_t ISO_Data_Load_Length :14;
	uint8_t ISO_Data_Load[];
}__attribute__((packed)) HCI_ISO_DATA_PCKT;


/* OpCode Group Field (OGF) for HCI command packets */
typedef enum
{
	LINK_CTRL_CMD 			= 0x01,
	LINK_POLICY_CMD  		= 0x02,
	CTRL_AND_BASEBAND_CMD  	= 0x03,
	INFO_PARAMETERS_CMD  	= 0x04,
	STATUS_PARAMETERS_CMD  	= 0x05,
	TESTING_CMD  			= 0x06,

	LE_CONTROLLER_CMD  		= 0x08,
	VENDOR_SPECIFIC_CMD  	= 0x3F,
}OPCODE_GROUP_FIELD;


#define PARSE_OPCODE(OCF,OGF) ( ((OGF) << 10) | ((OCF) & 0x3FF) )


/* Low Energy (LE) commands */
typedef enum
{
	HCI_READ_LOCAL_VER_INFO_OPCODE 	= (PARSE_OPCODE( 0x0001, LE_CONTROLLER_CMD )),
	HCI_LE_SET_ADV_DATA_OPCODE 		= (PARSE_OPCODE( 0x0008, LE_CONTROLLER_CMD )),
}COMMAND_OPCODE;


/* Event codes for HCI event packets */
typedef enum
{
	COMMAND_COMPLETE = 0x0E,
	COMMAND_STATUS 	 = 0x0F,
	VENDOR_SPECIFIC  = 0xFF
}EVENT_CODE;


typedef enum //todo: finalizar
{
	COMMAND_SUCCESS 	= 0x00,
	UNKNOWN_HCI_COMMAND = 0x01
}CONTROLLER_ERROR_CODES;


/****************************************************************/
/* VENDOR SPECIFIC DATA				                            */
/****************************************************************/
typedef enum
{
	REASON_RESERVED			  = 0x00, /* Reserved */
	FIRMWARE_STARTED_PROPERLY = 0x01, /* Firmware started properly */
	UPDATER_MODE_ACI_START	  = 0x02, /* Updater mode entered because of Aci_Updater_Start command */
	UPDATER_MODE_BLUE_FLAG	  = 0x03, /* Updater mode entered because of a bad BLUE flag */
	UPDATER_MODE_IRQ_PIN	  = 0x04, /* Updater mode entered with IRQ pin */
	RESET_BY_WATCHDOG		  = 0x05, /* Reset caused by watch dog */
	RESET_BY_LOCKUP			  = 0x06, /* Reset due to lockup */
	BROWNOUT_RESET			  = 0x07, /* Brownout reset */
	RESET_BY_CRASH			  = 0x08, /* Reset caused by a crash (NMI or Hard Fault) */
	RESET_ECC_ERROR			  = 0x09  /* Reset caused by an ECC error */
}REASON_CODE;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t HCI_Read_Local_Version_Information( void );
uint8_t HCI_LE_Set_Advertising_Data( uint8_t Advertising_Data_Length, uint8_t Advertising_Data[] );
void HCI_LE_Set_Advertising_Data_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );
void HCI_Read_Local_Version_Information_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );

/* Vendor Specific Events */
void HCI_VS_Blue_Initialized_Event( REASON_CODE Code );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* HCI_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
