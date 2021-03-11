

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
	HCI_READ_LOCAL_VERSION_INFORMATION 	= (PARSE_OPCODE( 0x0001, LE_CONTROLLER_CMD )),
	HCI_LE_SET_ADVERTISING_DATA 		= (PARSE_OPCODE( 0x0008, LE_CONTROLLER_CMD )),
}COMMAND_OPCODE;


/* Event codes for HCI event packets */
typedef enum
{
	HARDWARE_ERROR	 = 0x10,
	COMMAND_COMPLETE = 0x0E,
	COMMAND_STATUS 	 = 0x0F,
	VENDOR_SPECIFIC  = 0xFF
}EVENT_CODE;


typedef enum /* List of possible error codes, page 364 Core_v5.2 */
{
	COMMAND_SUCCESS 	   				 			= 0x00, /* Success */
	UNKNOWN_HCI_COMMAND    				 			= 0x01, /* Unknown HCI Command */
	UNKNOWN_CONNECTION_ID  				 			= 0x02, /* Unknown Connection Identifier */
	HARDWARE_FAILURE	   				 			= 0x03, /* Hardware Failure */
	PAGE_TIMEOUT		   				 			= 0x04,	/* Page Timeout */
	AUTHENTICATION_FAILURE 				 			= 0x05,	/* Authentication Failure */
	PIN_OR_KEY_MISSING     							= 0x06,	/* PIN or Key Missing */
	MEM_CAPACITY_EXCEEDED  				 			= 0x07,	/* Memory Capacity Exceeded */
	CONNECTION_TIMEOUT	   				 			= 0x08,	/* Connection Timeout */
	CONNECTION_LIMIT_EXCEEDED 			 			= 0x09, /* Connection Limit Exceeded */
	SYNC_CONN_LIMIT_TO_A_DEV_EXCEEDED 	 			= 0x0A, /* Synchronous Connection Limit To A Device Exceeded */
	CONNECTION_ALREADY_EXISTS 			 			= 0x0B, /* Connection Already Exists */
	COMMAND_DISALLOWED					 			= 0x0C, /* Command Disallowed */
	CONN_REJECTED_DUE_TO_LIM_RESOURCES   			= 0x0D, /* Connection Rejected due to Limited Resources */
	CONN_REJECTED_DUE_TO_SECURITY 		 			= 0x0E, /* Connection Rejected Due To Security Reasons */
	CONN_REJECTED_DUE_TO_UNACC_BD_ADDR   			= 0x0F, /* Connection Rejected due to Unacceptable BD_ADDR */
	CONNECTION_ACCEPT_TIMEOUT_EXCEEDED 	 			= 0x10, /* Connection Accept Timeout Exceeded */
	UNSUPPORTED_FEATURE_OR_PAR_VALUE     			= 0x11, /* Unsupported Feature or Parameter Value */
	INVALID_HCI_COMMAND_PARAMETERS 		 			= 0x12, /* Invalid HCI Command Parameters */
	REMOTE_USER_TERMINATED_CONNECTION	 			= 0x13, /* Remote User Terminated Connection */
	REM_DEV_TERM_CONN_LOW_RESOURCES 	 			= 0x14, /* Remote Device Terminated Connection due to Low Resources */
	REM_DEV_TERM_CONN_POWER_OFF 		 			= 0x15, /* Remote Device Terminated Connection due to Power Off */
	CONNECTION_TERMINATED_BY_LOCAL_HOST  			= 0x16, /* Connection Terminated By Local Host */
	REPEATED_ATTEMPTS					 			= 0x17, /* Repeated Attempts */
	PAIRING_NOT_ALLOWED 				 			= 0x18, /* Pairing Not Allowed */
	UNKNOWN_LMP_PDU 					 			= 0x19, /* Unknown LMP PDU */
	UNSUPPORTED_REMOTE_OR_LMP_FEATURE    			= 0x1A, /* Unsupported Remote Feature / Unsupported LMP Feature */
	SCO_OFFSET_REJECTED					 			= 0x1B, /* SCO Offset Rejected */
	SCO_INTERVAL_REJECTED 				 			= 0x1C, /* SCO Interval Rejected */
	SCO_AIR_MODE_REJECTED 				 			= 0x1D, /* SCO Air Mode Rejected */
	INVALID_LMP_OR_LL_PARAMETERS					= 0x1E, /* Invalid LMP Parameters / Invalid LL Parameters */
	UNSPECIFIED_ERROR 					 			= 0x1F, /* Unspecified Error */
	UNSUPPORTED_LMP_OR_LL_PAR_VALUE 	 			= 0x20, /* Unsupported LMP Parameter Value / Unsupported LL Parameter Value */
	ROLE_CHANGE_NOT_ALLOWED 			 			= 0x21, /* Role Change Not Allowed */
	LMP_OR_LL_RESPONSE_TIMEOUT			 			= 0x22, /* LMP Response Timeout / LL Response Timeout */
	LMP_OR_LL_COLLISION 				 			= 0x23, /* LMP Error Transaction Collision / LL Procedure Collision */
	LMP_PDU_NOT_ALLOWED					 			= 0x24, /* LMP PDU Not Allowed */
	ENCRYPTION_MODE_NOT_ACCEPTABLE 		 			= 0x25, /* Encryption Mode Not Acceptable */
	LINK_KEY_CANNOT_BE_CHANGED 			 			= 0x26, /* Link Key cannot be Changed */
	REQUESTED_QOS_NOT_SUPPORTED 		 			= 0x27, /* Requested QoS Not Supported */
	INSTANT_PASSED 						 			= 0x28, /* Instant Passed */
	PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED  			= 0x29, /* Pairing With Unit Key Not Supported */
	DIFFERENT_TRANSACTION_COLLISION 	 			= 0x2A, /* Different Transaction Collision */
	RESERVED_FOR_FUTURE_USE_1 			 			= 0x2B, /* Reserved for future use */
	QOS_UNACCEPTABLE_PARAMETER 			 			= 0x2C, /* QoS Unacceptable Parameter */
	QOS_REJECTED 						 			= 0x2D, /* QoS Rejected */
	CHANNEL_CLASSIFICATION_NOT_SUPPORTED 			= 0x2E, /* Channel Classification Not Supported */
	INSUFFICIENT_SECURITY 				 			= 0x2F, /* Insufficient Security */
	PARAMETER_OUT_OF_MANDATORY_RANGE 	 			= 0x30, /* Parameter Out Of Mandatory Range */
	RESERVED_FOR_FUTURE_USE_2 			 			= 0x31, /* Reserved for future use */
	ROLE_SWITCH_PENDING 				 			= 0x32, /* Role Switch Pending */
	RESERVED_FOR_FUTURE_USE_3			 			= 0x33, /* Reserved for future use */
	RESERVED_SLOT_VIOLATION 			 			= 0x34, /* Reserved Slot Violation */
	ROLE_SWITCH_FAILED 					 			= 0x35, /* Role Switch Failed */
	EXTENDED_INQUIRY_RESPONSE_TOO_LARGE  			= 0x36, /* Extended Inquiry Response Too Large */
	SECURE_SIMPLE_PAIRING_NOT_SUPPORTED  			= 0x37, /* Secure Simple Pairing Not Supported By Host */
	HOST_BUSY_PAIRING 					 			= 0x38, /* Host Busy - Pairing */
	CONN_REJECTED_NO_SUITABLE_CHANNEL_FOUND 		= 0x39, /* Connection Rejected due to No Suitable Channel Found */
	CONTROLLER_BUSY 								= 0x3A, /* Controller Busy */
	UNACCEPTABLE_CONNECTION_PARAMETERS 				= 0x3B, /* Unacceptable Connection Parameters */
	ADVERTISING_TIMEOUT 							= 0x3C, /* Advertising Timeout */
	CONN_TERMINATED_DUE_TO_MIC_FAILURE 				= 0x3D, /* Connection Terminated due to MIC Failure */
	CONN_FAIL_TO_ESTABLISH_OR_SYNC_TIMEOUT			= 0x3E, /* Connection Failed to be Established / Synchronization Timeout */
	MAC_CONNECTION_FAILED 							= 0x3F, /* MAC Connection Failed */
	COARSE_CLK_REJ_WILL_TRY_ADJ_WITH_CLK_DRAGGING 	= 0x40, /* Coarse Clock Adjustment Rejected but Will Try to Adjust Using Clock Dragging */
	TYPE0_SUBMAP_NOT_DEFINED 						= 0x41, /* Type0 Submap Not Defined */
	UNKNOWN_ADVERTISING_IDENTIFIER 					= 0x42, /* Unknown Advertising Identifier */
	LIMIT_REACHED 									= 0x43, /* Limit Reached */
	OPERATION_CANCELLED_BY_HOST 					= 0x44, /* Operation Cancelled by Host */
	PACKET_TOO_LONG 								= 0x45, /* Packet Too Long */
}CONTROLLER_ERROR_CODES;


typedef enum /* According to ST User Manual UM1865 - Rev 8, page 109 */
{
	/* These Hardware_Codes will be implementation-specific, and can be
	assigned to indicate various hardware problems. */
	SPI_FRAMING_ERROR   = 0,
	RADIO_STATE_ERROR   = 1,
	TIMER_OVERRUN_ERROR = 2
}BLE_HW_ERROR_CODE;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t HCI_LE_Set_Advertising_Data( uint8_t Advertising_Data_Length, uint8_t Advertising_Data[] );
void    HCI_LE_Set_Advertising_Data_Response( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );

uint8_t HCI_Read_Local_Version_Information( void );
void    HCI_Read_Local_Version_Information_Response( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );

void 	HCI_Hardware_Error( BLE_HW_ERROR_CODE Hardware_Code );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* HCI_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
