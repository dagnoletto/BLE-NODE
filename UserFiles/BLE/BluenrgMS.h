

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
#define SIZE_OF_FRAME_BUFFER 	8
#define SIZE_OF_CALLBACK_BUFFER 4


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	DEVICE_CONDITION_NOT_EXPECTED   = 0,  /* The device is in a not expected state */
	QUEUE_IS_FULL 					= 1,  /* The queue is full. */
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
void BluenrgMS_Configure_Receive_CallBack(TRANSFER_CALL_BACK_MODE* CallBackMode, HCI_PACKET_TYPE PacketType, uint8_t* DataPtr);
FRAME_ENQUEUE_STATUS BluenrgMS_Add_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index);
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
