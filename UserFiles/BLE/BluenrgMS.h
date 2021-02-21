

#ifndef BLUENRG_MS_DRIVER_H_
#define BLUENRG_MS_DRIVER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "TimeFunctions.h"
#include "hci_transport_layer.h"
#include <stdlib.h>


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define SIZE_OF_TX_FRAME_BUFFER 8


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	CALL_BACK_AFTER_TRANSFER = 0,   /* Just after transfer is executed, callback is called */
	CALL_BACK_BUFFERED 		 = 1	/* The callback is buffered and is called asynchronously during software execution  */
}TRANSFER_CALL_BACK_MODE;


typedef enum
{
	TRANSFER_DONE 		= 0, /* Transfer was successful */
	TRANSFER_DEV_ERROR  = 1, /* Some error occurred with the hardware */
	TRANSFER_TIMEOUT 	= 2  /* Could not within timeout */
}TRANSFER_STATUS;


typedef uint8_t (*TransferCallBack)(void* Header, uint8_t Status);


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
}FRAME_ENQUEUE_STATUS;


typedef enum
{
	DEVICE_CONDITION_NOT_EXPECTED   = 0,  /* The device is in a not expected state */
	MESSAGE_QUEUE_IS_FULL 			= 1,  /* The message queue buffer is full. */
	READ_ARGS_ERROR		 			= 2,  /* The read process could not start because of error in the reading arguments. */
	RECEPTION_ERROR					= 3,  /* Some error occurred due to device hardware or timeout during reception */
	NO_MEMORY_AVAILABLE				= 4,  /* Could not allocate memory */
	UNKNOWN_ERROR		 			= 5   /* Unknown error. */
}BLUENRG_ERROR_CODES;


typedef enum
{
	SPI_HEADER_READ  = 0, /* The operation will be a header read, that is, both TX and RX buffers have valid data */
	SPI_WRITE 		 = 1, /* The bytes will be transferred from the host to the controller (BluenrgMS). The RX buffer values can be discarded */
	SPI_READ  		 = 2  /* The bytes will be transferred from the controller (BluenrgMS) to the host. The write TX buffer is made of dummy bytes */
}SPI_TRANSFER_MODE;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void Reset_BluenrgMS(void);
void Run_BluenrgMS(void);
void BluenrgMS_IRQ(void);
extern uint8_t Send_BluenrgMS_SPI_Frame(SPI_TRANSFER_MODE Mode, uint8_t* TxPtr, uint8_t* RxPtr, uint16_t DataSize);
extern void BluenrgMS_Error(BLUENRG_ERROR_CODES Errorcode);
void Status_BluenrgMS_SPI_Frame(TRANSFER_STATUS status);
FRAME_ENQUEUE_STATUS BluenrgMS_Add_Message(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index);
void BluenrgMS_Configure_Receive_CallBack(TRANSFER_CALL_BACK_MODE* CallBackMode, HCI_PACKET_TYPE PacketType, uint8_t* DataPtr);
extern void Clr_BluenrgMS_Reset_Pin(void);
extern void Set_BluenrgMS_Reset_Pin(void);
extern uint8_t Get_BluenrgMS_IRQ_Pin(void);
extern void Release_BluenrgMS_SPI(void);
extern uint8_t* BluenrgMS_Allocate_Memory(uint16_t DataSize);
extern void BluenrgMS_Deallocate_Memory(uint8_t* DataPtr);


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLUENRG_MS_DRIVER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
