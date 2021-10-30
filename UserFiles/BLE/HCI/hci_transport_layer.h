

#ifndef HCI_TRANSPORT_LAYER_H_
#define HCI_TRANSPORT_LAYER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "vendor_specific_hci.h"


/****************************************************************/
/* Type Defines (Packet structure)                              */
/****************************************************************/
/* Those are the packet types defined in the Bluetooth Core Specification */
/* The standard defines the UART Transport Layer in v5.2 Volume 4: Part A */
typedef enum
{
	HCI_COMMAND_PACKET 			= 0x01,
	HCI_ACL_DATA_PACKET 		= 0x02,
	HCI_SYNCHRONOUS_DATA_PACKET = 0x03,
	HCI_EVENT_PACKET			= 0x04,
	HCI_ISO_DATA_PACKET 		= 0x05
}HCI_PACKET_TYPE;


typedef struct
{
	uint8_t PacketType;
	HCI_COMMAND_PCKT CmdPacket;
}__attribute__((packed)) HCI_SERIAL_COMMAND_PCKT;


typedef struct
{
	uint8_t PacketType;
	HCI_ACL_DATA_PCKT ACLDataPacket;
}__attribute__((packed)) HCI_SERIAL_ACL_DATA_PCKT;


typedef struct
{
	uint8_t PacketType;
	HCI_SYNCHRONOUS_DATA_PCKT SyncDataPacket;
}__attribute__((packed)) HCI_SERIAL_SYNCHRONOUS_DATA_PCKT;


typedef struct
{
	uint8_t PacketType;
	HCI_EVENT_PCKT EventPacket;
}__attribute__((packed)) HCI_SERIAL_EVENT_PCKT;


typedef struct
{
	uint8_t PacketType;
	HCI_ISO_DATA_PCKT ISODataPacket;
}__attribute__((packed)) HCI_SERIAL_ISO_DATA_PCKT;


/****************************************************************/
/* Type Defines (For driver interface)                          */
/****************************************************************/
typedef enum
{
	CALL_BACK_AFTER_TRANSFER = 0,   /* Just after transfer is executed, callback is called */
	CALL_BACK_BUFFERED 		 = 1	/* The callback is buffered and is called asynchronously during software execution  */
}TRANSFER_CALL_BACK_MODE;


typedef enum
{
	TRANSFER_DONE 			= 0, /* Transfer was successful */
	TRANSFER_DEV_ERROR  	= 1, /* Some error occurred with the hardware */
	TRANSFER_TIMEOUT 		= 2  /* Could not respond within timeout */
}TRANSFER_STATUS;


typedef uint8_t (*TransferCallBack)(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);


typedef struct
{
	uint8_t* DataPtr;
	uint16_t DataSize;
	TRANSFER_CALL_BACK_MODE CallBackMode;
	TransferCallBack CallBack; /* Callback called after the operation. If set as NULL is not called. */
}TRANSFER_DESCRIPTOR;


typedef struct
{
	int8_t EnqueuedAtIndex; /* If negative, means that frame was not enqueued */
	int8_t NumberOfEnqueuedFrames;
	uint8_t RequestTransmission; /* If TRUE, the higher layers can request transmission of newly enqueued frame */
}FRAME_ENQUEUE_STATUS;


typedef enum
{
	FREE 	 = 0,
	BUSY	 = 1,
	ON_GOING = 2
}CMD_CB_STATUS;


typedef struct
{
	CMD_CB_STATUS Status: 8; /* CB_FREE/CB_BUSY/CB_ON_GOING */
	void* CmdCompleteCallBack; /* Function pointer */
	void* CmdStatusCallBack; /* Function pointer */
	void (*CmdCompleteHandler)(void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr); /* Function pointer */
}CMD_CALLBACK;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t HCI_Transmit(void* DataPtr, uint16_t DataSize,
		TRANSFER_CALL_BACK_MODE CallBackMode,
		TransferCallBack CallBack, CMD_CALLBACK* CmdCallBack);
void HCI_Receive(uint8_t* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);
void LE_Advertising_Report_Handler( HCI_EVENT_PCKT* EventPacketPtr );
void Clear_Command_CallBack( HCI_COMMAND_OPCODE OpCode );
void Command_Status_Handler( HCI_COMMAND_OPCODE OpCode, CMD_CALLBACK* CmdCallBack,
		HCI_EVENT_PCKT* EventPacketPtr );
void Command_Complete_Handler( HCI_COMMAND_OPCODE OpCode, CMD_CALLBACK* CmdCallBack,
		HCI_EVENT_PCKT* EventPacketPtr );
void Set_Number_Of_HCI_Data_Packets( uint16_t Num_HCI_Data_Packets );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* HCI_TRANSPORT_LAYER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
