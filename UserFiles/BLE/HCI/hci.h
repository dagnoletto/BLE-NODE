

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
	HCI_DISCONNECT						= (PARSE_OPCODE( 0x0006, LINK_CTRL_CMD 	   	 	 )),
	HCI_READ_REMOTE_VERSION_INFORMATION = (PARSE_OPCODE( 0x001D, LINK_CTRL_CMD 	 		 )),
	HCI_SET_EVENT_MASK					= (PARSE_OPCODE( 0x0001, CTRL_AND_BASEBAND_CMD 	 )),
	HCI_RESET							= (PARSE_OPCODE( 0x0003, CTRL_AND_BASEBAND_CMD 	 )),
	HCI_READ_TRANSMIT_POWER_LEVEL		= (PARSE_OPCODE( 0x002D, CTRL_AND_BASEBAND_CMD 	 )),
	HCI_READ_LOCAL_VERSION_INFORMATION 	= (PARSE_OPCODE( 0x0001, INFO_PARAMETERS_CMD	 )),
	HCI_LE_SET_ADVERTISING_DATA 		= (PARSE_OPCODE( 0x0008, LE_CONTROLLER_CMD 		 ))
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


typedef enum
{
	CORE_SPEC_1_0b 		 = 0,  /* Bluetooth Core Specification 1.0b (Withdrawn) */
	CORE_SPEC_1_1 		 = 1,  /* Bluetooth Core Specification 1.1 (Withdrawn) */
	CORE_SPEC_1_2 		 = 2,  /* Bluetooth Core Specification 1.2 (Withdrawn) */
	CORE_SPEC_2_0_EDR 	 = 3,  /* Bluetooth Core Specification 2.0 + EDR (Withdrawn) */
	CORE_SPEC_2_1_EDR 	 = 4,  /* Bluetooth Core Specification 2.1 + EDR (Withdrawn) */
	CORE_SPEC_3_0_HS	 = 5,  /* Bluetooth Core Specification 3.0 + HS (Withdrawn) */
	CORE_SPEC_4_0 		 = 6,  /* Bluetooth Core Specification 4.0 */
	CORE_SPEC_4_1		 = 7,  /* Bluetooth Core Specification 4.1 */
	CORE_SPEC_4_2		 = 8,  /* Bluetooth Core Specification 4.2 */
	CORE_SPEC_5_0		 = 9,  /* Bluetooth Core Specification 5.0 */
	CORE_SPEC_5_1		 = 10, /* Bluetooth Core Specification 5.1 */
	CORE_SPEC_5_2	     = 11  /* Bluetooth Core Specification 5.2 */
}HCI_VERSION;


typedef union
{
	struct Event_Mask_Bits
	{
		uint8_t Inquiry_Complete_event 								:1; /* Bit 0 */
		uint8_t Inquiry_Result_event								:1; /* Bit 1 */
		uint8_t Connection_Complete_event 							:1; /* Bit 2 */
		uint8_t Connection_Request_event 							:1; /* Bit 3 */
		uint8_t Disconnection_Complete_event 						:1; /* Bit 4 */
		uint8_t Authentication_Complete_event   					:1; /* Bit 5 */
		uint8_t Remote_Name_Request_Complete_event 					:1; /* Bit 6 */
		uint8_t Encryption_Change_event 							:1; /* Bit 7 */
		uint8_t Change_Connection_Link_Key_Complete_event 			:1; /* Bit 8 */
		uint8_t Master_Link_Key_Complete_event 						:1; /* Bit 9 */
		uint8_t Read_Remote_Supported_Features_Complete_event 		:1; /* Bit 10 */
		uint8_t Read_Remote_Version_Information_Complete_event 		:1; /* Bit 11 */
		uint8_t QoS_Setup_Complete_event 							:1; /* Bit 12 */
		uint8_t Reserved1											:2;
		uint8_t Hardware_Error_event 								:1; /* Bit 15 */
		uint8_t Flush_Occurred_event 								:1; /* Bit 16 */
		uint8_t Role_Change_event 									:1; /* Bit 17 */
		uint8_t Reserved2											:1;
		uint8_t Mode_Change_event 									:1; /* Bit 19 */
		uint8_t Return_Link_Keys_event 								:1; /* Bit 20 */
		uint8_t PIN_Code_Request_event 								:1; /* Bit 21 */
		uint8_t Link_Key_Request_event 								:1; /* Bit 22 */
		uint8_t Link_Key_Notification_event 						:1; /* Bit 23 */
		uint8_t Loopback_Command_event 								:1; /* Bit 24 */
		uint8_t Data_Buffer_Overflow_event 							:1; /* Bit 25 */
		uint8_t Max_Slots_Change_event 								:1; /* Bit 26 */
		uint8_t Read_Clock_Offset_Complete_event 					:1; /* Bit 27 */
		uint8_t Connection_Packet_Type_Changed_event 				:1; /* Bit 28 */
		uint8_t QoS_Violation_event 								:1; /* Bit 29 */
		uint8_t Page_Scan_Mode_Change_event 						:1; /* Bit 30 */ /* [deprecated] */
		uint8_t Page_Scan_Repetition_Mode_Change_event 				:1; /* Bit 31 */
		uint8_t Flow_Specification_Complete_event 					:1; /* Bit 32 */
		uint8_t Inquiry_Result_with_RSSI_event 						:1; /* Bit 33 */
		uint8_t Read_Remote_Extended_Features_Complete_event 		:1; /* Bit 34 */
		uint16_t Reserved3 											:8;
		uint8_t Synchronous_Connection_Complete_event 				:1; /* Bit 43 */
		uint8_t Synchronous_Connection_Changed_event 				:1; /* Bit 44 */
		uint8_t Sniff_Subrating_event 								:1; /* Bit 45 */
		uint8_t Extended_Inquiry_Result_event 						:1; /* Bit 46 */
		uint8_t Encryption_Key_Refresh_Complete_event 				:1; /* Bit 47 */
		uint8_t IO_Capability_Request_event 						:1; /* Bit 48 */
		uint8_t IO_Capability_Response_event 						:1; /* Bit 49 */
		uint8_t User_Confirmation_Request_event 					:1; /* Bit 50 */
		uint8_t User_Passkey_Request_event							:1; /* Bit 51 */
		uint8_t Remote_OOB_Data_Request_event 						:1; /* Bit 52 */
		uint8_t Simple_Pairing_Complete_event 						:1; /* Bit 53 */
		uint8_t Reserved4											:1;
		uint8_t Link_Supervision_Timeout_Changed_event 				:1; /* Bit 55 */
		uint8_t Enhanced_Flush_Complete_event 						:1; /* Bit 56 */
		uint8_t Reserved5											:1;
		uint8_t User_Passkey_Notification_event 					:1; /* Bit 58 */
		uint8_t Keypress_Notification_event 						:1; /* Bit 59 */
		uint8_t Remote_Host_Supported_Features_Notification_event 	:1; /* Bit 60 */
		uint8_t LE_Meta_event 										:1; /* Bit 61 */
		uint8_t Reserved6											:2;
	}__attribute__((packed)) Bits;
	uint8_t Bytes[sizeof(struct Event_Mask_Bits)];
	uint64_t U64Var;
}__attribute__((packed)) EVENT_MASK;


/* It might be the case that for some compilers uint64_t
 * does not exist and byte per byte assignment is necessary in the macro */
#define DEFAULT_EVENT_MASK_VAL 0x00001FFFFFFFFFFF /* Bits 0 to 44 are set */
#define SET_EVENT_MASK_DEFAULT(Mask) ( Mask.U64Var = DEFAULT_EVENT_MASK_VAL )


typedef enum /* According to ST User Manual UM1865 - Rev 8, page 109 */
{
	/* These Hardware_Codes will be implementation-specific, and can be
	assigned to indicate various hardware problems. */
	SPI_FRAMING_ERROR   = 0,
	RADIO_STATE_ERROR   = 1,
	TIMER_OVERRUN_ERROR = 2
}BLE_HW_ERROR_CODE;


#define MAX_CONNECTION_HANDLE 		0x0EFF
#define MAX_ADVERTISING_DATA_LENGTH 	31


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t HCI_Disconnect( uint16_t Connection_Handle, CONTROLLER_ERROR_CODES Reason );
void 	HCI_Disconnect_Status( CONTROLLER_ERROR_CODES Status );

uint8_t HCI_Read_Remote_Version_Information( uint16_t Connection_Handle );
void 	HCI_Read_Remote_Version_Information_Status( CONTROLLER_ERROR_CODES Status );

uint8_t HCI_Set_Event_Mask( EVENT_MASK Event_Mask );
void 	HCI_Set_Event_Mask_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status );

uint8_t HCI_Read_Transmit_Power_Level( uint16_t Connection_Handle, uint8_t Type );
void 	HCI_Read_Transmit_Power_Level_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_Read_Transmit_Power_Level_Complete( CONTROLLER_ERROR_CODES Status, uint16_t Connection_Handle, int8_t TX_Power_Level );

uint8_t HCI_Reset( void );
void 	HCI_Reset_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_Reset_Complete( CONTROLLER_ERROR_CODES Status );

uint8_t HCI_Read_Local_Version_Information( void );
void    HCI_Read_Local_Version_Information_Status( CONTROLLER_ERROR_CODES Status );
void    HCI_Read_Local_Version_Information_Complete( CONTROLLER_ERROR_CODES Status,
		HCI_VERSION HCI_Version, uint16_t HCI_Revision,
		uint8_t LMP_PAL_Version, uint16_t Manufacturer_Name,
		uint16_t LMP_PAL_Subversion);

uint8_t HCI_LE_Set_Advertising_Data( uint8_t Advertising_Data_Length, uint8_t Advertising_Data[] );
void    HCI_LE_Set_Advertising_Data_Status( CONTROLLER_ERROR_CODES Status );
void    HCI_LE_Set_Advertising_Data_Complete( CONTROLLER_ERROR_CODES Status );

void 	HCI_Hardware_Error( BLE_HW_ERROR_CODE Hardware_Code );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* HCI_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
