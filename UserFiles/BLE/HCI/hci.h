

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
	HCI_DISCONNECT							= (PARSE_OPCODE( 0x0006, LINK_CTRL_CMD 	   	 	 )),
	HCI_READ_REMOTE_VERSION_INFORMATION 	= (PARSE_OPCODE( 0x001D, LINK_CTRL_CMD 	 		 )),
	HCI_SET_EVENT_MASK						= (PARSE_OPCODE( 0x0001, CTRL_AND_BASEBAND_CMD 	 )),
	HCI_RESET								= (PARSE_OPCODE( 0x0003, CTRL_AND_BASEBAND_CMD 	 )),
	HCI_READ_TRANSMIT_POWER_LEVEL			= (PARSE_OPCODE( 0x002D, CTRL_AND_BASEBAND_CMD 	 )),
	HCI_READ_LOCAL_VERSION_INFORMATION 		= (PARSE_OPCODE( 0x0001, INFO_PARAMETERS_CMD	 )),
	HCI_READ_LOCAL_SUPPORTED_COMMANDS		= (PARSE_OPCODE( 0x0002, INFO_PARAMETERS_CMD 	 )),
	HCI_READ_LOCAL_SUPPORTED_FEATURES		= (PARSE_OPCODE( 0x0003, INFO_PARAMETERS_CMD 	 )),
	HCI_READ_BD_ADDR						= (PARSE_OPCODE( 0x0009, INFO_PARAMETERS_CMD 	 )),
	HCI_READ_RSSI							= (PARSE_OPCODE( 0x0005, STATUS_PARAMETERS_CMD 	 )),
	HCI_LE_SET_EVENT_MASK					= (PARSE_OPCODE( 0x0001, LE_CONTROLLER_CMD 		 )),
	HCI_LE_READ_BUFFER_SIZE					= (PARSE_OPCODE( 0x0002, LE_CONTROLLER_CMD 		 )),
	HCI_LE_READ_LOCAL_SUPPORTED_FEATURES 	= (PARSE_OPCODE( 0x0003, LE_CONTROLLER_CMD 		 )),
	HCI_LE_SET_RANDOM_ADDRESS				= (PARSE_OPCODE( 0x0005, LE_CONTROLLER_CMD 		 )),
	HCI_LE_SET_ADVERTISING_DATA 			= (PARSE_OPCODE( 0x0008, LE_CONTROLLER_CMD 		 ))
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


typedef union
{
	struct Supported_Commands_Bits
	{
		uint8_t HCI_Inquiry 						:1; /* Octet 0 Bit 0 */
		uint8_t HCI_Inquiry_Cancel					:1; /* Octet 0 Bit 1 */
		uint8_t HCI_Periodic_Inquiry_Mode 			:1; /* Octet 0 Bit 2 */
		uint8_t HCI_Exit_Periodic_Inquiry_Mode 		:1; /* Octet 0 Bit 3 */
		uint8_t HCI_Create_Connection 				:1;	/* Octet 0 Bit 4 */
		uint8_t HCI_Disconnect 						:1;	/* Octet 0 Bit 5 */
		uint8_t HCI_Add_SCO_Connection 				:1; /* Octet 0 Bit 6 *//* [deprecated] */
		uint8_t HCI_Create_Connection_Cancel 		:1; /* Octet 0 Bit 7 */

		uint8_t HCI_Accept_Connection_Request 		:1; /* Octet 1 Bit 0 */
		uint8_t HCI_Reject_Connection_Request 		:1; /* Octet 1 Bit 1 */
		uint8_t HCI_Link_Key_Request_Reply 	  		:1; /* Octet 1 Bit 2 */
		uint8_t HCI_Link_Key_Request_Negative_Reply :1; /* Octet 1 Bit 3 */
		uint8_t HCI_PIN_Code_Request_Reply 			:1; /* Octet 1 Bit 4 */
		uint8_t HCI_PIN_Code_Request_Negative_Reply :1; /* Octet 1 Bit 5 */
		uint8_t HCI_Change_Connection_Packet_Type 	:1; /* Octet 1 Bit 6 */
		uint8_t HCI_Authentication_Requested 		:1; /* Octet 1 Bit 7 */

		uint8_t HCI_Set_Connection_Encryption		:1; /* Octet 2 Bit 0 */
		uint8_t HCI_Change_Connection_Link_Key      :1; /* Octet 2 Bit 1 */
		uint8_t HCI_Master_Link_Key 				:1; /* Octet 2 Bit 2 */
		uint8_t HCI_Remote_Name_Request				:1; /* Octet 2 Bit 3 */
		uint8_t HCI_Remote_Name_Request_Cancel		:1; /* Octet 2 Bit 4 */
		uint8_t HCI_Read_Remote_Supported_Features  :1; /* Octet 2 Bit 5 */
		uint8_t HCI_Read_Remote_Extended_Features   :1; /* Octet 2 Bit 6 */
		uint8_t HCI_Read_Remote_Version_Information :1; /* Octet 2 Bit 7 */

		uint8_t HCI_Read_Clock_Offset	:1; /* Octet 3 Bit 0 */
		uint8_t HCI_Read_LMP_Handle		:1; /* Octet 3 Bit 1 */
		uint8_t Reserved1				:6;	/* Octet 3 Bit 2 to Bit 7 */

		uint8_t Reserved2 				:1; /* Octet 4 Bit 0 */
		uint8_t HCI_Hold_Mode 			:1; /* Octet 4 Bit 1 */
		uint8_t HCI_Sniff_Mode			:1; /* Octet 4 Bit 2 */
		uint8_t HCI_Exit_Sniff_Mode 	:1; /* Octet 4 Bit 3 */
		uint8_t Previously_used_1		:2; /* Octet 4 Bit 4 to Bit 5 */
		uint8_t HCI_QoS_Setup 			:1; /* Octet 4 Bit 6 */
		uint8_t HCI_Role_Discovery 		:1; /* Octet 4 Bit 7 */

		uint8_t HCI_Switch_Role 						:1; /* Octet 5 Bit 0 */
		uint8_t HCI_Read_Link_Policy_Settings 			:1; /* Octet 5 Bit 1 */
		uint8_t HCI_Write_Link_Policy_Settings 			:1; /* Octet 5 Bit 2 */
		uint8_t HCI_Read_Default_Link_Policy_Settings 	:1; /* Octet 5 Bit 3 */
		uint8_t HCI_Write_Default_Link_Policy_Settings 	:1; /* Octet 5 Bit 4 */
		uint8_t HCI_Flow_Specification 					:1; /* Octet 5 Bit 5 */
		uint8_t HCI_Set_Event_Mask 						:1; /* Octet 5 Bit 6 */
		uint8_t HCI_Reset 								:1; /* Octet 5 Bit 7 */

		uint8_t HCI_Set_Event_Filter 			:1; /* Octet 6 Bit 0 */
		uint8_t HCI_Flush						:1; /* Octet 6 Bit 1 */
		uint8_t HCI_Read_PIN_Type				:1; /* Octet 6 Bit 2 */
		uint8_t HCI_Write_PIN_Type				:1; /* Octet 6 Bit 3 */
		uint8_t Previously_used_2				:1; /* Octet 6 Bit 4 */
		uint8_t HCI_Read_Stored_Link_Key		:1; /* Octet 6 Bit 5 */
		uint8_t HCI_Write_Stored_Link_Key		:1; /* Octet 6 Bit 6 */
		uint8_t HCI_Delete_Stored_Link_Key		:1; /* Octet 6 Bit 7 */

		uint8_t HCI_Write_Local_Name				:1; /* Octet 7 Bit 0 */
		uint8_t HCI_Read_Local_Name					:1; /* Octet 7 Bit 1 */
		uint8_t HCI_Read_Connection_Accept_Timeout	:1; /* Octet 7 Bit 2 */
		uint8_t HCI_Write_Connection_Accept_Timeout :1; /* Octet 7 Bit 3 */
		uint8_t HCI_Read_Page_Timeout				:1; /* Octet 7 Bit 4 */
		uint8_t HCI_Write_Page_Timeout				:1; /* Octet 7 Bit 5 */
		uint8_t HCI_Read_Scan_Enable				:1; /* Octet 7 Bit 6 */
		uint8_t HCI_Write_Scan_Enable				:1; /* Octet 7 Bit 7 */

		uint8_t HCI_Read_Page_Scan_Activity			:1;	/* Octet 8 Bit 0 */
		uint8_t HCI_Write_Page_Scan_Activity 		:1;	/* Octet 8 Bit 1 */
		uint8_t HCI_Read_Inquiry_Scan_Activity 		:1; /* Octet 8 Bit 2 */
		uint8_t HCI_Write_Inquiry_Scan_Activity 	:1; /* Octet 8 Bit 3 */
		uint8_t HCI_Read_Authentication_Enable 		:1; /* Octet 8 Bit 4 */
		uint8_t HCI_Write_Authentication_Enable 	:1;	/* Octet 8 Bit 5 */
		uint8_t HCI_Read_Encryption_Mode			:1;	/* Octet 8 Bit 6 *//* [deprecated] */
		uint8_t HCI_Write_Encryption_Mode			:1;	/* Octet 8 Bit 7 *//* [deprecated] */

		uint8_t HCI_Read_Class_Of_Device				:1; /* Octet 9 Bit 0 */
		uint8_t HCI_Write_Class_Of_Device				:1; /* Octet 9 Bit 1 */
		uint8_t HCI_Read_Voice_Setting					:1; /* Octet 9 Bit 2 */
		uint8_t HCI_Write_Voice_Setting					:1; /* Octet 9 Bit 3 */
		uint8_t HCI_Read_Automatic_Flush_Timeout		:1; /* Octet 9 Bit 4 */
		uint8_t HCI_Write_Automatic_Flush_Timeout		:1; /* Octet 9 Bit 5 */
		uint8_t HCI_Read_Num_Broadcast_Retransmissions	:1; /* Octet 9 Bit 6 */
		uint8_t HCI_Write_Num_Broadcast_Retransmissions	:1; /* Octet 9 Bit 7 */

		uint8_t HCI_Read_Hold_Mode_Activity					:1;	/* Octet 10 Bit 0 */
		uint8_t HCI_Write_Hold_Mode_Activity				:1;	/* Octet 10 Bit 1 */
		uint8_t HCI_Read_Transmit_Power_Level				:1;	/* Octet 10 Bit 2 */
		uint8_t HCI_Read_Synchronous_Flow_Control_Enable	:1; /* Octet 10 Bit 3 */
		uint8_t HCI_Write_Synchronous_Flow_Control_Enable	:1; /* Octet 10 Bit 4 */
		uint8_t HCI_Set_Controller_To_Host_Flow_Control		:1; /* Octet 10 Bit 5 */
		uint8_t HCI_Host_Buffer_Size						:1;	/* Octet 10 Bit 6 */
		uint8_t HCI_Host_Number_Of_Completed_Packets		:1; /* Octet 10 Bit 7 */

		uint8_t HCI_Read_Link_Supervision_Timeout		:1; /* Octet 11 Bit 0 */
		uint8_t HCI_Write_Link_Supervision_Timeout		:1; /* Octet 11 Bit 1 */
		uint8_t HCI_Read_Number_Of_Supported_IAC		:1; /* Octet 11 Bit 2 */
		uint8_t HCI_Read_Current_IAC_LAP				:1; /* Octet 11 Bit 3 */
		uint8_t HCI_Write_Current_IAC_LAP				:1; /* Octet 11 Bit 4 */
		uint8_t HCI_Read_Page_Scan_Mode_Period			:1; /* Octet 11 Bit 5 *//* [deprecated] */
		uint8_t HCI_Write_Page_Scan_Mode_Period			:1; /* Octet 11 Bit 6 *//* [deprecated] */
		uint8_t HCI_Read_Page_Scan_Mode					:1; /* Octet 11 Bit 7 *//* [deprecated] */

		uint8_t HCI_Write_Page_Scan_Mode					:1;	/* Octet 12 Bit 0 *//* [deprecated] */
		uint8_t HCI_Set_AFH_Host_Channel_Classification		:1; /* Octet 12 Bit 1 */
		uint8_t Reserved3 									:2; /* Octet 12 Bit 2 to Bit 3 */
		uint8_t HCI_Read_Inquiry_Scan_Type					:1; /* Octet 12 Bit 4 */
		uint8_t HCI_Write_Inquiry_Scan_Type					:1; /* Octet 12 Bit 5 */
		uint8_t HCI_Read_Inquiry_Mode						:1; /* Octet 12 Bit 6 */
		uint8_t HCI_Write_Inquiry_Mode						:1; /* Octet 12 Bit 7 */

		uint8_t HCI_Read_Page_Scan_Type						:1; /* Octet 13 Bit 0 */
		uint8_t HCI_Write_Page_Scan_Type					:1; /* Octet 13 Bit 1 */
		uint8_t HCI_Read_AFH_Channel_Assessment_Mode		:1; /* Octet 13 Bit 2 */
		uint8_t HCI_Write_AFH_Channel_Assessment_Mode		:1; /* Octet 13 Bit 3 */
		uint8_t Reserved4									:4;	/* Octet 13 Bit 4 to Bit 7 */

		uint8_t Reserved5 								:3; /* Octet 14 Bit 0 to Bit 2 */
		uint8_t HCI_Read_Local_Version_Information		:1; /* Octet 14 Bit 3 */
		uint8_t Reserved6								:1; /* Octet 14 Bit 4 */
		uint8_t HCI_Read_Local_Supported_Features 		:1; /* Octet 14 Bit 5 */
		uint8_t HCI_Read_Local_Extended_Features		:1; /* Octet 14 Bit 6 */
		uint8_t HCI_Read_Buffer_Size					:1; /* Octet 14 Bit 7 */

		uint8_t HCI_Read_Country_Code					:1; /* Octet 15 Bit 0 *//* [deprecated] */
		uint8_t HCI_Read_BD_ADDR						:1; /* Octet 15 Bit 1 */
		uint8_t HCI_Read_Failed_Contact_Counter			:1; /* Octet 15 Bit 2 */
		uint8_t HCI_Reset_Failed_Contact_Counter		:1; /* Octet 15 Bit 3 */
		uint8_t HCI_Read_Link_Quality					:1; /* Octet 15 Bit 4 */
		uint8_t HCI_Read_RSSI							:1; /* Octet 15 Bit 5 */
		uint8_t HCI_Read_AFH_Channel_Map				:1; /* Octet 15 Bit 6 */
		uint8_t HCI_Read_Clock							:1; /* Octet 15 Bit 7 */

		uint8_t HCI_Read_Loopback_Mode						:1; /* Octet 16 Bit 0 */
		uint8_t HCI_Write_Loopback_Mode						:1; /* Octet 16 Bit 1 */
		uint8_t HCI_Enable_Device_Under_Test_Mode			:1; /* Octet 16 Bit 2 */
		uint8_t HCI_Setup_Synchronous_Connection_Request	:1; /* Octet 16 Bit 3 */
		uint8_t HCI_Accept_Synchronous_Connection_Request	:1; /* Octet 16 Bit 4 */
		uint8_t HCI_Reject_Synchronous_Connection_Request	:1; /* Octet 16 Bit 5 */
		uint8_t Reserved7 									:2; /* Octet 16 Bit 6 to Bit7 */

		uint8_t HCI_Read_Extended_Inquiry_Response 		:1; /* Octet 17 Bit 0 */
		uint8_t HCI_Write_Extended_Inquiry_Response		:1; /* Octet 17 Bit 1 */
		uint8_t HCI_Refresh_Encryption_Key				:1; /* Octet 17 Bit 2 */
		uint8_t Reserved8								:1; /* Octet 17 Bit 3 */
		uint8_t HCI_Sniff_Subrating						:1; /* Octet 17 Bit 4 */
		uint8_t HCI_Read_Simple_Pairing_Mode			:1; /* Octet 17 Bit 5 */
		uint8_t HCI_Write_Simple_Pairing_Mode			:1; /* Octet 17 Bit 6 */
		uint8_t HCI_Read_Local_OOB_Data					:1; /* Octet 17 Bit 7 */

		uint8_t HCI_Read_Inquiry_Response_Transmit_Power_Level 	:1; /* Octet 18 Bit 0 */
		uint8_t HCI_Write_Inquiry_Transmit_Power_Level			:1; /* Octet 18 Bit 1 */
		uint8_t HCI_Read_Default_Erroneous_Data_Reporting		:1; /* Octet 18 Bit 2 */
		uint8_t HCI_Write_Default_Erroneous_Data_Reporting		:1; /* Octet 18 Bit 3 */
		uint8_t Reserved9										:3; /* Octet 18 Bit 4 to Bit 6 */
		uint8_t HCI_IO_Capability_Request_Reply					:1; /* Octet 18 Bit 7 */

		uint8_t HCI_User_Confirmation_Request_Reply				:1; /* Octet 19 Bit 0 */
		uint8_t HCI_User_Confirmation_Request_Negative_Reply	:1; /* Octet 19 Bit 1 */
		uint8_t HCI_User_Passkey_Request_Reply					:1; /* Octet 19 Bit 2 */
		uint8_t HCI_User_Passkey_Request_Negative_Reply			:1; /* Octet 19 Bit 3 */
		uint8_t HCI_Remote_OOB_Data_Request_Reply				:1; /* Octet 19 Bit 4 */
		uint8_t HCI_Write_Simple_Pairing_Debug_Mode				:1; /* Octet 19 Bit 5 */
		uint8_t HCI_Enhanced_Flush								:1; /* Octet 19 Bit 6 */
		uint8_t HCI_Remote_OOB_Data_Request_Negative_Reply		:1; /* Octet 19 Bit 7 */

		uint8_t Reserved10 										:2; /* Octet 20 Bit 0 to Bit 1 */
		uint8_t HCI_Send_Keypress_Notification					:1; /* Octet 20 Bit 2 */
		uint8_t HCI_IO_Capability_Request_Negative_Reply		:1; /* Octet 20 Bit 3 */
		uint8_t HCI_Read_Encryption_Key_Size					:1; /* Octet 20 Bit 4 */
		uint8_t Reserved11 										:3; /* Octet 20 Bit 5 to Bit 7 */

		uint8_t HCI_Create_Physical_Link		:1; /* Octet 21 Bit 0 */
		uint8_t HCI_Accept_Physical_Link		:1; /* Octet 21 Bit 1 */
		uint8_t HCI_Disconnect_Physical_Link	:1; /* Octet 21 Bit 2 */
		uint8_t HCI_Create_Logical_Link			:1; /* Octet 21 Bit 3 */
		uint8_t HCI_Accept_Logical_Link			:1; /* Octet 21 Bit 4 */
		uint8_t HCI_Disconnect_Logical_Link		:1; /* Octet 21 Bit 5 */
		uint8_t HCI_Logical_Link_Cancel			:1; /* Octet 21 Bit 6 */
		uint8_t HCI_Flow_Spec_Modify			:1; /* Octet 21 Bit 7 */

		uint8_t HCI_Read_Logical_Link_Accept_Timeout	:1; /* Octet 22 Bit 0 */
		uint8_t HCI_Write_Logical_Link_Accept_Timeout	:1; /* Octet 22 Bit 1 */
		uint8_t HCI_Set_Event_Mask_Page_2				:1; /* Octet 22 Bit 2 */
		uint8_t HCI_Read_Location_Data					:1; /* Octet 22 Bit 3 */
		uint8_t HCI_Write_Location_Data					:1; /* Octet 22 Bit 4 */
		uint8_t HCI_Read_Local_AMP_Info					:1; /* Octet 22 Bit 5 */
		uint8_t HCI_Read_Local_AMP_ASSOC				:1; /* Octet 22 Bit 6 */
		uint8_t HCI_Write_Remote_AMP_ASSOC				:1; /* Octet 22 Bit 7 */

		uint8_t HCI_Read_Flow_Control_Mode				:1; /* Octet 23 Bit 0 */
		uint8_t HCI_Write_Flow_Control_Mode				:1; /* Octet 23 Bit 1 */
		uint8_t HCI_Read_Data_Block_Size				:1; /* Octet 23 Bit 2 */
		uint8_t Reserved12 								:2; /* Octet 23 Bit 3 to Bit 4 */
		uint8_t HCI_Enable_AMP_Receiver_Reports			:1; /* Octet 23 Bit 5 */
		uint8_t HCI_AMP_Test_End						:1; /* Octet 23 Bit 6 */
		uint8_t HCI_AMP_Test							:1; /* Octet 23 Bit 7 */

		uint8_t HCI_Read_Enhanced_Transmit_Power_Level		:1; /* Octet 24 Bit 0 */
		uint8_t Reserved13 									:1; /* Octet 24 Bit 1 */
		uint8_t HCI_Read_Best_Effort_Flush_Timeout			:1; /* Octet 24 Bit 2 */
		uint8_t HCI_Write_Best_Effort_Flush_Timeout			:1; /* Octet 24 Bit 3 */
		uint8_t HCI_Short_Range_Mode						:1; /* Octet 24 Bit 4 */
		uint8_t HCI_Read_LE_Host_Support					:1; /* Octet 24 Bit 5 */
		uint8_t HCI_Write_LE_Host_Support					:1; /* Octet 24 Bit 6 */
		uint8_t Reserved14 									:1; /* Octet 24 Bit 7 */

		uint8_t HCI_LE_Set_Event_Mask								:1; /* Octet 25 Bit 0 */
		uint8_t HCI_LE_Read_Buffer_Size_v1							:1; /* Octet 25 Bit 1 *//* [v1] */
		uint8_t HCI_LE_Read_Local_Supported_Features				:1; /* Octet 25 Bit 2 */
		uint8_t Reserved15 											:1; /* Octet 25 Bit 3 */
		uint8_t HCI_LE_Set_Random_Address							:1; /* Octet 25 Bit 4 */
		uint8_t HCI_LE_Set_Advertising_Parameters					:1; /* Octet 25 Bit 5 */
		uint8_t HCI_LE_Read_Advertising_Physical_Channel_Tx_Power	:1; /* Octet 25 Bit 6 */
		uint8_t HCI_LE_Set_Advertising_Data							:1; /* Octet 25 Bit 7 */

		uint8_t HCI_LE_Set_Scan_Response_Data		:1; /* Octet 26 Bit 0 */
		uint8_t HCI_LE_Set_Advertising_Enable		:1; /* Octet 26 Bit 1 */
		uint8_t HCI_LE_Set_Scan_Parameters			:1; /* Octet 26 Bit 2 */
		uint8_t HCI_LE_Set_Scan_Enable				:1; /* Octet 26 Bit 3 */
		uint8_t HCI_LE_Create_Connection			:1; /* Octet 26 Bit 4 */
		uint8_t HCI_LE_Create_Connection_Cancel		:1; /* Octet 26 Bit 5 */
		uint8_t HCI_LE_Read_White_List_Size			:1; /* Octet 26 Bit 6 */
		uint8_t HCI_LE_Clear_White_List				:1; /* Octet 26 Bit 7 */

		uint8_t HCI_LE_Add_Device_To_White_List				:1; /* Octet 27 Bit 0 */
		uint8_t HCI_LE_Remove_Device_From_White_List		:1; /* Octet 27 Bit 1 */
		uint8_t HCI_LE_Connection_Update					:1; /* Octet 27 Bit 2 */
		uint8_t HCI_LE_Set_Host_Channel_Classification		:1; /* Octet 27 Bit 3 */
		uint8_t HCI_LE_Read_Channel_Map						:1; /* Octet 27 Bit 4 */
		uint8_t HCI_LE_Read_Remote_Features					:1; /* Octet 27 Bit 5 */
		uint8_t HCI_LE_Encrypt								:1; /* Octet 27 Bit 6 */
		uint8_t HCI_LE_Rand									:1; /* Octet 27 Bit 7 */

		uint8_t HCI_LE_Enable_Encryption					:1; /* Octet 28 Bit 0 */
		uint8_t HCI_LE_Long_Term_Key_Request_Reply			:1; /* Octet 28 Bit 1 */
		uint8_t HCI_LE_Long_Term_Key_Request_Negative_Reply	:1; /* Octet 28 Bit 2 */
		uint8_t HCI_LE_Read_Supported_States				:1; /* Octet 28 Bit 3 */
		uint8_t HCI_LE_Receiver_Test_v1						:1; /* Octet 28 Bit 4 *//* [v1] */
		uint8_t HCI_LE_Transmitter_Test_v1					:1; /* Octet 28 Bit 5 *//* [v1] */
		uint8_t HCI_LE_Test_End								:1; /* Octet 28 Bit 6 */
		uint8_t Reserved16									:1; /* Octet 28 Bit 7 */

		uint8_t Reserved17 									:3; /* Octet 29 Bit 0 to Bit 2 */
		uint8_t HCI_Enhanced_Setup_Synchronous_Connection	:1; /* Octet 29 Bit 3 */
		uint8_t HCI_Enhanced_Accept_Synchronous_Connection	:1; /* Octet 29 Bit 4 */
		uint8_t HCI_Read_Local_Supported_Codecs_v1			:1;	/* Octet 29 Bit 5 *//* [v1] */
		uint8_t HCI_Set_MWS_Channel_Parameters				:1; /* Octet 29 Bit 6 */
		uint8_t HCI_Set_External_Frame_Configuration		:1;	/* Octet 29 Bit 7 */

		uint8_t HCI_Set_MWS_Signaling						:1; /* Octet 30 Bit 0 */
		uint8_t HCI_Set_MWS_Transport_Layer					:1; /* Octet 30 Bit 1 */
		uint8_t HCI_Set_MWS_Scan_Frequency_Table			:1; /* Octet 30 Bit 2 */
		uint8_t HCI_Get_MWS_Transport_Layer_Configuration	:1; /* Octet 30 Bit 3 */
		uint8_t HCI_Set_MWS_PATTERN_Configuration			:1; /* Octet 30 Bit 4 */
		uint8_t HCI_Set_Triggered_Clock_Capture				:1; /* Octet 30 Bit 5 */
		uint8_t HCI_Truncated_Page							:1; /* Octet 30 Bit 6 */
		uint8_t HCI_Truncated_Page_Cancel					:1;	/* Octet 30 Bit 7 */

		uint8_t HCI_Set_Connectionless_Slave_Broadcast			:1; /* Octet 31 Bit 0 */
		uint8_t HCI_Set_Connectionless_Slave_Broadcast_Receive	:1; /* Octet 31 Bit 1 */
		uint8_t HCI_Start_Synchronization_Train					:1; /* Octet 31 Bit 2 */
		uint8_t HCI_Receive_Synchronization_Train				:1; /* Octet 31 Bit 3 */
		uint8_t HCI_Set_Reserved_LT_ADDR						:1; /* Octet 31 Bit 4 */
		uint8_t HCI_Delete_Reserved_LT_ADDR						:1; /* Octet 31 Bit 5 */
		uint8_t HCI_Set_Connectionless_Slave_Broadcast_Data		:1; /* Octet 31 Bit 6 */
		uint8_t HCI_Read_Synchronization_Train_Parameters		:1; /* Octet 31 Bit 7 */

		uint8_t HCI_Write_Synchronization_Train_Parameters		:1; /* Octet 32 Bit 0 */
		uint8_t HCI_Remote_OOB_Extended_Data_Request_Reply		:1; /* Octet 32 Bit 1 */
		uint8_t HCI_Read_Secure_Connections_Host_Support		:1; /* Octet 32 Bit 2 */
		uint8_t HCI_Write_Secure_Connections_Host_Support		:1; /* Octet 32 Bit 3 */
		uint8_t HCI_Read_Authenticated_Payload_Timeout			:1; /* Octet 32 Bit 4 */
		uint8_t HCI_Write_Authenticated_Payload_Timeout			:1; /* Octet 32 Bit 5 */
		uint8_t HCI_Read_Local_OOB_Extended_Data				:1; /* Octet 32 Bit 6 */
		uint8_t HCI_Write_Secure_Connections_Test_Mode			:1; /* Octet 32 Bit 7 */

		uint8_t HCI_Read_Extended_Page_Timeout								:1; /* Octet 33 Bit 0 */
		uint8_t HCI_Write_Extended_Page_Timeout								:1; /* Octet 33 Bit 1 */
		uint8_t HCI_Read_Extended_Inquiry_Length							:1; /* Octet 33 Bit 2 */
		uint8_t HCI_Write_Extended_Inquiry_Length							:1; /* Octet 33 Bit 3 */
		uint8_t HCI_LE_Remote_Connection_Parameter_Request_Reply			:1; /* Octet 33 Bit 4 */
		uint8_t HCI_LE_Remote_Connection_Parameter_Request_Negative_Reply	:1; /* Octet 33 Bit 5 */
		uint8_t HCI_LE_Set_Data_Length										:1; /* Octet 33 Bit 6 */
		uint8_t HCI_LE_Read_Suggested_Default_Data_Length					:1; /* Octet 33 Bit 7 */

		uint8_t HCI_LE_Write_Suggested_Default_Data_Length	:1; /* Octet 34 Bit 0 */
		uint8_t HCI_LE_Read_Local_P_256_Public_Key			:1; /* Octet 34 Bit 1 */
		uint8_t HCI_LE_Generate_DHKey_v1					:1; /* Octet 34 Bit 2 *//* [v1] */
		uint8_t HCI_LE_Add_Device_To_Resolving_List			:1; /* Octet 34 Bit 3 */
		uint8_t HCI_LE_Remove_Device_From_Resolving_List	:1; /* Octet 34 Bit 4 */
		uint8_t HCI_LE_Clear_Resolving_List					:1; /* Octet 34 Bit 5 */
		uint8_t HCI_LE_Read_Resolving_List_Size				:1; /* Octet 34 Bit 6 */
		uint8_t HCI_LE_Read_Peer_Resolvable_Address			:1; /* Octet 34 Bit 7 */

		uint8_t HCI_LE_Read_Local_Resolvable_Address			:1; /* Octet 35 Bit 0 */
		uint8_t HCI_LE_Set_Address_Resolution_Enable			:1; /* Octet 35 Bit 1 */
		uint8_t HCI_LE_Set_Resolvable_Private_Address_Timeout	:1; /* Octet 35 Bit 2 */
		uint8_t HCI_LE_Read_Maximum_Data_Length					:1; /* Octet 35 Bit 3 */
		uint8_t HCI_LE_Read_PHY									:1; /* Octet 35 Bit 4 */
		uint8_t HCI_LE_Set_Default_PHY							:1; /* Octet 35 Bit 5 */
		uint8_t HCI_LE_Set_PHY									:1; /* Octet 35 Bit 6 */
		uint8_t HCI_LE_Receiver_Test_v2							:1; /* Octet 35 Bit 7 *//* [v2] */

		uint8_t HCI_LE_Transmitter_Test_v2							:1; /* Octet 36 Bit 0 *//* [v2] */
		uint8_t HCI_LE_Set_Advertising_Set_Random_Address			:1; /* Octet 36 Bit 1 */
		uint8_t HCI_LE_Set_Extended_Advertising_Parameters			:1; /* Octet 36 Bit 2 */
		uint8_t HCI_LE_Set_Extended_Advertising_Data				:1; /* Octet 36 Bit 3 */
		uint8_t HCI_LE_Set_Extended_Scan_Response_Data				:1; /* Octet 36 Bit 4 */
		uint8_t HCI_LE_Set_Extended_Advertising_Enable				:1; /* Octet 36 Bit 5 */
		uint8_t HCI_LE_Read_Maximum_Advertising_Data_Length			:1; /* Octet 36 Bit 6 */
		uint8_t HCI_LE_Read_Number_of_Supported_Advertising_Sets	:1; /* Octet 36 Bit 7 */

		uint8_t HCI_LE_Remove_Advertising_Set				:1; /* Octet 37 Bit 0 */
		uint8_t HCI_LE_Clear_Advertising_Sets				:1; /* Octet 37 Bit 1 */
		uint8_t HCI_LE_Set_Periodic_Advertising_Parameters	:1; /* Octet 37 Bit 2 */
		uint8_t HCI_LE_Set_Periodic_Advertising_Data		:1; /* Octet 37 Bit 3 */
		uint8_t HCI_LE_Set_Periodic_Advertising_Enable		:1; /* Octet 37 Bit 4 */
		uint8_t HCI_LE_Set_Extended_Scan_Parameters			:1; /* Octet 37 Bit 5 */
		uint8_t HCI_LE_Set_Extended_Scan_Enable				:1; /* Octet 37 Bit 6 */
		uint8_t HCI_LE_Extended_Create_Connection			:1; /* Octet 37 Bit 7 */

		uint8_t HCI_LE_Periodic_Advertising_Create_Sync				:1; /* Octet 38 Bit 0 */
		uint8_t HCI_LE_Periodic_Advertising_Create_Sync_Cancel		:1; /* Octet 38 Bit 1 */
		uint8_t HCI_LE_Periodic_Advertising_Terminate_Sync			:1; /* Octet 38 Bit 2 */
		uint8_t HCI_LE_Add_Device_To_Periodic_Advertiser_List		:1; /* Octet 38 Bit 3 */
		uint8_t HCI_LE_Remove_Device_From_Periodic_Advertiser_List	:1; /* Octet 38 Bit 4 */
		uint8_t HCI_LE_Clear_Periodic_Advertiser_List				:1; /* Octet 38 Bit 5 */
		uint8_t HCI_LE_Read_Periodic_Advertiser_List_Size			:1; /* Octet 38 Bit 6 */
		uint8_t HCI_LE_Read_Transmit_Power							:1; /* Octet 38 Bit 7 */

		uint8_t HCI_LE_Read_RF_Path_Compensation					:1; /* Octet 39 Bit 0 */
		uint8_t HCI_LE_Write_RF_Path_Compensation					:1; /* Octet 39 Bit 1 */
		uint8_t HCI_LE_Set_Privacy_Mode								:1; /* Octet 39 Bit 2 */
		uint8_t HCI_LE_Receiver_Test_v3								:1; /* Octet 39 Bit 3 *//* [v3] */
		uint8_t HCI_LE_Transmitter_Test_v3							:1; /* Octet 39 Bit 4 *//* [v3] */
		uint8_t HCI_LE_Set_Connectionless_CTE_Transmit_Parameters	:1; /* Octet 39 Bit 5 */
		uint8_t HCI_LE_Set_Connectionless_CTE_Transmit_Enable		:1;	/* Octet 39 Bit 6 */
		uint8_t HCI_LE_Set_Connectionless_IQ_Sampling_Enable		:1; /* Octet 39 Bit 7 */

		uint8_t HCI_LE_Set_Connection_CTE_Receive_Parameters		:1; /* Octet 40 Bit 0 */
		uint8_t HCI_LE_Set_Connection_CTE_Transmit_Parameters		:1; /* Octet 40 Bit 1 */
		uint8_t HCI_LE_Connection_CTE_Request_Enable				:1; /* Octet 40 Bit 2 */
		uint8_t HCI_LE_Connection_CTE_Response_Enable				:1; /* Octet 40 Bit 3 */
		uint8_t HCI_LE_Read_Antenna_Information						:1; /* Octet 40 Bit 4 */
		uint8_t HCI_LE_Set_Periodic_Advertising_Receive_Enable		:1; /* Octet 40 Bit 5 */
		uint8_t HCI_LE_Periodic_Advertising_Sync_Transfer			:1; /* Octet 40 Bit 6 */
		uint8_t HCI_LE_Periodic_Advertising_Set_Info_Transfer		:1; /* Octet 40 Bit 7 */

		uint8_t HCI_LE_Set_Periodic_Advertising_Sync_Transfer_Parameters		 	:1; /* Octet 41 Bit 0 */
		uint8_t HCI_LE_Set_Default_Periodic_Advertising_Sync_Transfer_Parameters	:1; /* Octet 41 Bit 1 */
		uint8_t HCI_LE_Generate_DHKey_v2											:1; /* Octet 41 Bit 2 *//* [v2] */
		uint8_t HCI_Read_Local_Simple_Pairing_Options								:1; /* Octet 41 Bit 3 */
		uint8_t HCI_LE_Modify_Sleep_Clock_Accuracy									:1; /* Octet 41 Bit 4 */
		uint8_t HCI_LE_Read_Buffer_Size_v2											:1; /* Octet 41 Bit 5 *//* [v2] */
		uint8_t HCI_LE_Read_ISO_TX_Sync												:1; /* Octet 41 Bit 6 */
		uint8_t HCI_LE_Set_CIG_Parameters											:1; /* Octet 41 Bit 7 */

		uint8_t HCI_LE_Set_CIG_Parameters_Test	:1; /* Octet 42 Bit 0 */
		uint8_t HCI_LE_Create_CIS				:1; /* Octet 42 Bit 1 */
		uint8_t HCI_LE_Remove_CIG				:1; /* Octet 42 Bit 2 */
		uint8_t HCI_LE_Accept_CIS_Request		:1; /* Octet 42 Bit 3 */
		uint8_t HCI_LE_Reject_CIS_Request		:1; /* Octet 42 Bit 4 */
		uint8_t HCI_LE_Create_BIG				:1; /* Octet 42 Bit 5 */
		uint8_t HCI_LE_Create_BIG_Test			:1; /* Octet 42 Bit 6 */
		uint8_t HCI_LE_Terminate_BIG			:1; /* Octet 42 Bit 7 */

		uint8_t HCI_LE_BIG_Create_Sync			:1; /* Octet 43 Bit 0 */
		uint8_t HCI_LE_BIG_Terminate_Sync		:1; /* Octet 43 Bit 1 */
		uint8_t HCI_LE_Request_Peer_SCA			:1; /* Octet 43 Bit 2 */
		uint8_t HCI_LE_Setup_ISO_Data_Path		:1;	/* Octet 43 Bit 3 */
		uint8_t HCI_LE_Remove_ISO_Data_Path		:1; /* Octet 43 Bit 4 */
		uint8_t HCI_LE_ISO_Transmit_Test		:1;	/* Octet 43 Bit 5 */
		uint8_t HCI_LE_ISO_Receive_Test			:1; /* Octet 43 Bit 6 */
		uint8_t HCI_LE_ISO_Read_Test_Counters	:1; /* Octet 43 Bit 7 */

		uint8_t HCI_LE_ISO_Test_End							:1; /* Octet 44 Bit 0 */
		uint8_t HCI_LE_Set_Host_Feature						:1; /* Octet 44 Bit 1 */
		uint8_t HCI_LE_Read_ISO_Link_Quality				:1; /* Octet 44 Bit 2 */
		uint8_t HCI_LE_Enhanced_Read_Transmit_Power_Level	:1; /* Octet 44 Bit 3 */
		uint8_t HCI_LE_Read_Remote_Transmit_Power_Level		:1; /* Octet 44 Bit 4 */
		uint8_t HCI_LE_Set_Path_Loss_Reporting_Parameters	:1; /* Octet 44 Bit 5 */
		uint8_t HCI_LE_Set_Path_Loss_Reporting_Enable		:1; /* Octet 44 Bit 6 */
		uint8_t HCI_LE_Set_Transmit_Power_Reporting_Enable	:1; /* Octet 44 Bit 7 */

		uint8_t HCI_LE_Transmitter_Test						:1; /* Octet 45 Bit 0 */
		uint8_t HCI_Set_Ecosystem_Base_Interval				:1; /* Octet 45 Bit 1 */
		uint8_t HCI_Read_Local_Supported_Codecs_v2			:1; /* Octet 45 Bit 2 *//* [v2] */
		uint8_t HCI_Read_Local_Supported_Codec_Capabilities	:1; /* Octet 45 Bit 3 */
		uint8_t HCI_Read_Local_Supported_Controller_Delay	:1; /* Octet 45 Bit 4 */
		uint8_t HCI_Configure_Data_Path						:1; /* Octet 45 Bit 5 */
		uint8_t Reserved18 									:2; /* Octet 45 Bit 6 to Bit 7 */
	}__attribute__((packed)) Bits;
	uint8_t Bytes[sizeof(struct Supported_Commands_Bits)];
}__attribute__((packed)) SUPPORTED_COMMANDS;


typedef union
{
	struct Supported_Features_Bits
	{
		uint8_t _3_slot_packets		:1;/* 0 */ /* Byte 0 Bit 0 */
		uint8_t _5_slot_packets		:1;/* 1 */ /* Byte 0 Bit 1 */
		uint8_t Encryption			:1;/* 2 */ /* Byte 0 Bit 2 */
		uint8_t Slot_offset			:1;/* 3 */ /* Byte 0 Bit 3 */
		uint8_t Timing_accuracy		:1;/* 4 */ /* Byte 0 Bit 4 */
		uint8_t Role_switch			:1;/* 5 */ /* Byte 0 Bit 5 */
		uint8_t Hold_mode 			:1;/* 6 */ /* Byte 0 Bit 6 */
		uint8_t Sniff_mode			:1;/* 7 */ /* Byte 0 Bit 7 */

		uint8_t Previously_used						:1;/* 8 */ /* Byte 1 Bit 0 */
		uint8_t Power_control_requests				:1;/* 9 */ /* Byte 1 Bit 1 */
		uint8_t Channel_quality_driven_data_rate 	:1;/* 10 */ /* Byte 1 Bit 2 */ /* (CQDDR) */
		uint8_t SCO_link							:1;/* 11 */ /* Byte 1 Bit 3 */
		uint8_t HV2_packets							:1;/* 12 */ /* Byte 1 Bit 4 */
		uint8_t HV3_packets							:1;/* 13 */ /* Byte 1 Bit 5 */
		uint8_t u_law_log_synchronous_data			:1;/* 14 */ /* Byte 1 Bit 6 */
		uint8_t A_law_log_synchronous_data			:1;/* 15 */ /* Byte 1 Bit 7 */

		uint8_t CVSD_synchronous_data			:1;/* 16 */ /* Byte 2 Bit 0 */
		uint8_t Paging_parameter_negotiation	:1;/* 17 */ /* Byte 2 Bit 1 */
		uint8_t Power_control					:1;/* 18 */ /* Byte 2 Bit 2 */
		uint8_t Transparent_synchronous_data	:1;/* 19 */ /* Byte 2 Bit 3 */
		uint8_t Flow_control_lag 				:3;/* 20/21/22 */ /* Byte 2 Bit 4 to Bit 6 */
		uint8_t Broadcast_Encryption			:1;/* 23 */ /* Byte 2 Bit 7 */

		uint8_t Reserved1					:1;/* 24 */ /* Byte 3 Bit 0 */
		uint8_t EDR_ACL_2_Mb_mode			:1;/* 25 */ /* Byte 3 Bit 1 *//* Enhanced Data Rate ACL 2 Mb/s mode */
		uint8_t EDR_ACL_3_Mb_mode			:1;/* 26 */ /* Byte 3 Bit 2 *//* Enhanced Data Rate ACL 3 Mb/s mode */
		uint8_t Enhanced_inquiry_scan 		:1;/* 27 */ /* Byte 3 Bit 3 *//* �Enhanced Inquiry Scan� is no longer used in the specification.
																	 Devices may set this bit but are not required to. */
		uint8_t Interlaced_inquiry_scan		:1;/* 28 */ /* Byte 3 Bit 4 */
		uint8_t Interlaced_page_scan		:1;/* 29 */ /* Byte 3 Bit 5 */
		uint8_t RSSI_with_inquiry_results	:1;/* 30 */ /* Byte 3 Bit 6 */
		uint8_t Extended_SCO_link			:1;/* 31 */ /* Byte 3 Bit 7 *//* (EV3 packets) */

		uint8_t EV4_packets					:1;/* 32 */ /* Byte 4 Bit 0 */
		uint8_t EV5_packets					:1;/* 33 */ /* Byte 4 Bit 1 */
		uint8_t Reserved2					:1;/* 34 */ /* Byte 4 Bit 2 */
		uint8_t AFH_capable_slave			:1;/* 35 */ /* Byte 4 Bit 3 */
		uint8_t AFH_classification_slave	:1;/* 36 */ /* Byte 4 Bit 4 */
		uint8_t BR_EDR_Not_Supported		:1;/* 37 */ /* Byte 4 Bit 5 */
		uint8_t LE_Supported				:1;/* 38 */ /* Byte 4 Bit 6 *//* (Controller) */
		uint8_t _3_slot_EDR_ACL_packets		:1;/* 39 */ /* Byte 4 Bit 7 *//* 3-slot Enhanced Data Rate ACL packets */

		uint8_t _5_slot_EDR_ACL_packets		:1;/* 40 */ /* Byte 5 Bit 0 *//* 5-slot Enhanced Data Rate ACL packets */
		uint8_t Sniff_subrating				:1;/* 41 */ /* Byte 5 Bit 1 */
		uint8_t Pause_encryption			:1;/* 42 */ /* Byte 5 Bit 2 */
		uint8_t AFH_capable_master			:1;/* 43 */ /* Byte 5 Bit 3 */
		uint8_t AFH_classification_master	:1;/* 44 */ /* Byte 5 Bit 4 */
		uint8_t EDR_eSCO_2_Mb_mode			:1;/* 45 */ /* Byte 5 Bit 5 *//* Enhanced Data Rate eSCO 2 Mb/s mode */
		uint8_t EDR_eSCO_3_Mb_mode			:1;/* 46 */ /* Byte 5 Bit 6 *//* Enhanced Data Rate eSCO 3 Mb/s mode */
		uint8_t _3_slot_EDR_eSCO_packets	:1;/* 47 */ /* Byte 5 Bit 7 *//* 3-slot Enhanced Data Rate eSCO packets */

		uint8_t Extended_Inquiry_Response										:1;/* 48 */ /* Byte 6 Bit 0 */
		uint8_t Simultaneous_LE_and_BR_EDR_to_Same_Device_Capable				:1;/* 49 */ /* Byte 6 Bit 1 *//* (Controller) */
		uint8_t Reserved3														:1;/* 50 */ /* Byte 6 Bit 2 */
		uint8_t Secure_Simple_Pairing											:1;/* 51 */ /* Byte 6 Bit 3 *//* (Controller Support) */
		uint8_t Encapsulated_PDU												:1;/* 52 */ /* Byte 6 Bit 4 */
		uint8_t Erroneous_Data_Reporting										:1;/* 53 */ /* Byte 6 Bit 5 */
		uint8_t Non_flushable_Packet_Boundary_Flag								:1;/* 54 */ /* Byte 6 Bit 6 */
		uint8_t Reserved4														:1;/* 55 */ /* Byte 6 Bit 7 */

		uint8_t HCI_Link_Supervision_Timeout_Changed_event	:1;/* 56 */ /* Byte 7 Bit 0 */
		uint8_t Variable_Inquiry_TX_Power_Level				:1;/* 57 */ /* Byte 7 Bit 1 */
		uint8_t Enhanced_Power_Control						:1;/* 58 */ /* Byte 7 Bit 2 */
		uint8_t Reserved5 									:4;/* 59/60/61/62 */ /* Byte 7 Bit 3 to Bit 6 */
		uint8_t Extended_features 							:1;/* 63 */ /* Byte 7 Bit 7 */
	}__attribute__((packed)) Bits;
	uint8_t Bytes[sizeof(struct Supported_Features_Bits)];
}__attribute__((packed)) SUPPORTED_FEATURES;


/* TODO: Implementar demais p�ginas e o comando: HCI_Read_Local_Extended_Features */
typedef SUPPORTED_FEATURES SUPPORTED_FEATURES_PAGE_0;


typedef struct
{
	uint8_t Byte[6];
}BD_ADDR_TYPE;


typedef union
{
	struct LE_Event_Mask_Bits
	{
		uint8_t LE_Connection_Complete_event 							:1; /* Bit 0 */
		uint8_t LE_Advertising_Report_event								:1; /* Bit 1 */
		uint8_t LE_Connection_Update_Complete_event						:1; /* Bit 2 */
		uint8_t LE_Read_Remote_Features_Complete_event					:1; /* Bit 3 */
		uint8_t LE_Long_Term_Key_Request_event							:1; /* Bit 4 */
		uint8_t LE_Remote_Connection_Parameter_Request_event			:1; /* Bit 5 */
		uint8_t LE_Data_Length_Change_event								:1; /* Bit 6 */
		uint8_t LE_Read_Local_P_256_Public_Key_Complete_event			:1; /* Bit 7 */
		uint8_t LE_Generate_DHKey_Complete_event						:1; /* Bit 8 */
		uint8_t LE_Enhanced_Connection_Complete_event					:1; /* Bit 9 */
		uint8_t LE_Directed_Advertising_Report_event					:1; /* Bit 10 */
		uint8_t LE_PHY_Update_Complete_event							:1; /* Bit 11 */
		uint8_t LE_Extended_Advertising_Report_event					:1; /* Bit 12 */
		uint8_t LE_Periodic_Advertising_Sync_Established_event			:1; /* Bit 13 */
		uint8_t LE_Periodic_Advertising_Report_event					:1; /* Bit 14 */
		uint8_t LE_Periodic_Advertising_Sync_Lost_event					:1; /* Bit 15 */
		uint8_t LE_Scan_Timeout_event									:1; /* Bit 16 */
		uint8_t LE_Advertising_Set_Terminated_event						:1; /* Bit 17 */
		uint8_t LE_Scan_Request_Received_event							:1; /* Bit 18 */
		uint8_t LE_Channel_Selection_Algorithm_event					:1; /* Bit 19 */
		uint8_t LE_Connectionless_IQ_Report_event						:1; /* Bit 20 */
		uint8_t LE_Connection_IQ_Report_event							:1; /* Bit 21 */
		uint8_t LE_CTE_Request_Failed_event								:1; /* Bit 22 */
		uint8_t LE_Periodic_Advertising_Sync_Transfer_Received_event	:1; /* Bit 23 */
		uint8_t LE_CIS_Established_event								:1; /* Bit 24 */
		uint8_t LE_CIS_Request_event									:1; /* Bit 25 */
		uint8_t LE_Create_BIG_Complete_event							:1; /* Bit 26 */
		uint8_t LE_Terminate_BIG_Complete_event							:1; /* Bit 27 */
		uint8_t LE_BIG_Sync_Established_event							:1; /* Bit 28 */
		uint8_t LE_BIG_Sync_Lost_event									:1; /* Bit 29 */
		uint8_t LE_Request_Peer_SCA_Complete_event						:1; /* Bit 30 */
		uint8_t LE_Path_Loss_Threshold_event							:1; /* Bit 31 */
		uint8_t LE_Transmit_Power_Reporting_event						:1; /* Bit 32 */
		uint8_t LE_BIGInfo_Advertising_Report_event						:1; /* Bit 33 */
		uint8_t Reserved1												:6; /* Bit 34 to Bit 39 */
		uint8_t Reserved2;
		uint8_t Reserved3;
		uint8_t Reserved4;
	}__attribute__((packed)) Bits;
	uint8_t Bytes[sizeof(struct LE_Event_Mask_Bits)];
	uint64_t U64Var;
}__attribute__((packed)) LE_EVENT_MASK;


/* It might be the case that for some compilers uint64_t
 * does not exist and byte per byte assignment is necessary in the macro */
#define DEFAULT_LE_EVENT_MASK_VAL 0x000000000000001F /* Bits 0 to 4 are set */
#define SET_LE_EVENT_MASK_DEFAULT(Mask) ( Mask.U64Var = DEFAULT_LE_EVENT_MASK_VAL )


typedef union
{
	struct LE_Supported_Features_Bits
	{
		uint8_t LE_Encryption									:1;/* Bit 0 */
		uint8_t Connection_Parameters_Request_Procedure			:1;/* Bit 1 */
		uint8_t Extended_Reject_Indication						:1;/* Bit 2 */
		uint8_t Slave_initiated_Features_Exchange 				:1;/* Bit 3 */
		uint8_t LE_Ping 										:1;/* Bit 4 */
		uint8_t LE_Data_Packet_Length_Extension					:1;/* Bit 5 */
		uint8_t LL_Privacy										:1;/* Bit 6 */
		uint8_t Extended_Scanner_Filter_Policies				:1;/* Bit 7 */
		uint8_t LE_2M_PHY										:1;/* Bit 8 */
		uint8_t Stable_Modulation_Index_Transmitter				:1;/* Bit 9 */
		uint8_t Stable_Modulation_Index_Receiver				:1;/* Bit 10 */
		uint8_t LE_Coded_PHY									:1;/* Bit 11 */
		uint8_t LE_Extended_Advertising							:1;/* Bit 12 */
		uint8_t LE_Periodic_Advertising							:1;/* Bit 13 */
		uint8_t Channel_Selection_Algorithm_2					:1;/* Bit 14 */
		uint8_t LE_Power_Class_1								:1;/* Bit 15 */
		uint8_t Minimum_Number_of_Used_Channels_Procedure		:1;/* Bit 16 */
		uint8_t Connection_CTE_Request							:1;/* Bit 17 */
		uint8_t Connection_CTE_Response							:1;/* Bit 18 */
		uint8_t Connectionless_CTE_Transmitter					:1;/* Bit 19 */
		uint8_t Connectionless_CTE_Receiver						:1;/* Bit 20 */
		uint8_t Antenna_Switching_During_CTE_Transmission_AoD 	:1;/* Bit 21 */
		uint8_t Antenna_Switching_During_CTE_Reception_AoA    	:1;/* Bit 22 */
		uint8_t Receiving_Constant_Tone_Extensions 				:1;/* Bit 23 */
		uint8_t Periodic_Advertising_Sync_Transfer_Sender		:1;/* Bit 24 */
		uint8_t Periodic_Advertising_Sync_Transfer_Recipient 	:1;/* Bit 25 */
		uint8_t Sleep_Clock_Accuracy_Updates					:1;/* Bit 26 */
		uint8_t Remote_Public_Key_Validation					:1;/* Bit 27 */
		uint8_t Connected_Isochronous_Stream_Master				:1;/* Bit 28 */
		uint8_t Connected_Isochronous_Stream_Slave				:1;/* Bit 29 */
		uint8_t Isochronous_Broadcaster							:1;/* Bit 30 */
		uint8_t Synchronized_Receiver							:1;/* Bit 31 */
		uint8_t Isochronous_Channels							:1;/* Bit 32 *//* (Host Support) */
		uint8_t LE_Power_Control_Request						:1;/* Bit 33 */
		uint8_t LE_Power_Change_Indication						:1;/* Bit 34 */
		uint8_t LE_Path_Loss_Monitoring							:1;/* Bit 35 */
		uint8_t Reserved1										:4;/* Bit 36 to Bit Bit 39 */
		uint8_t Reserved2[3];
	}__attribute__((packed)) Bits;
	uint8_t Bytes[sizeof(struct LE_Supported_Features_Bits)];
}__attribute__((packed)) LE_SUPPORTED_FEATURES;


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

uint8_t HCI_Read_Local_Supported_Commands( void );
void	HCI_Read_Local_Supported_Commands_Status( CONTROLLER_ERROR_CODES Status );
void	HCI_Read_Local_Supported_Commands_Complete( CONTROLLER_ERROR_CODES Status, SUPPORTED_COMMANDS* Supported_Commands );

uint8_t HCI_Read_Local_Supported_Features( void );
void 	HCI_Read_Local_Supported_Features_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status, SUPPORTED_FEATURES* LMP_Features );

uint8_t HCI_Read_BD_ADDR( void );
void 	HCI_Read_BD_ADDR_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_Read_BD_ADDR_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE BD_ADDR );

uint8_t HCI_Read_RSSI( uint16_t Handle );
void 	HCI_Read_RSSI_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_Read_RSSI_Complete( CONTROLLER_ERROR_CODES Status, uint16_t Handle, int8_t RSSI );

uint8_t HCI_LE_Set_Event_Mask( LE_EVENT_MASK LE_Event_Mask );
void    HCI_LE_Set_Event_Mask_Status( CONTROLLER_ERROR_CODES Status );
void    HCI_LE_Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status );

uint8_t HCI_LE_Read_Buffer_Size( void );
void 	HCI_LE_Read_Buffer_Size_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_LE_Read_Buffer_Size_Complete( CONTROLLER_ERROR_CODES Status, uint16_t LE_ACL_Data_Packet_Length,
		uint8_t Total_Num_LE_ACL_Data_Packets );

uint8_t HCI_LE_Read_Local_Supported_Features( void );
void 	HCI_LE_Read_Local_Supported_Features_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_LE_Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status, LE_SUPPORTED_FEATURES* LE_Features );

uint8_t HCI_LE_Set_Random_Address( BD_ADDR_TYPE Random_Address );
void 	HCI_LE_Set_Random_Address_Status( CONTROLLER_ERROR_CODES Status );
void 	HCI_LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );

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
