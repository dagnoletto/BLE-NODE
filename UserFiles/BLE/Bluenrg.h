

#ifndef BLUENRG_DRIVER_H_
#define BLUENRG_DRIVER_H_


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
	SPI_HEADER_WRITE = 1, /* The operation will be a header write, that is, both TX and RX buffers have valid data */
	SPI_WRITE 		 = 2, /* The bytes will be transferred from the host to the controller (Bluenrg). The RX buffer values can be discarded */
	SPI_READ  		 = 3  /* The bytes will be transferred from the controller (Bluenrg) to the host. The write TX buffer is made of dummy bytes */
}SPI_TRANSFER_MODE;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void Reset_Bluenrg(void);
void Run_Bluenrg(void);
void Bluenrg_IRQ(void);
void Release_Bluenrg(void);
void Bluenrg_Abort_Transfer(void);
uint8_t Bluenrg_Send_Frame(SPI_TRANSFER_MODE Mode, uint8_t* TxPtr, uint8_t* RxPtr, uint16_t DataSize);
void Bluenrg_Frame_Status(TRANSFER_STATUS status);
void Bluenrg_Error(BLUENRG_ERROR_CODES Errorcode);
void Bluenrg_CallBack_Config(TRANSFER_CALL_BACK_MODE* CallBackMode, HCI_PACKET_TYPE PacketType, uint8_t* DataPtr);
FRAME_ENQUEUE_STATUS Bluenrg_Enqueue_Frame(int8_t buffer_index, SPI_TRANSFER_MODE TransferMode, uint8_t Priority);
void Bluenrg_Set_Buffer_Full( void* BufferPtr, uint16_t DataSize );
void Bluenrg_Request_Frame(void);
void Clr_Bluenrg_Reset_Pin(void);
void Set_Bluenrg_Reset_Pin(void);
uint8_t Get_Bluenrg_IRQ_Pin(void);
int8_t Bluenrg_Get_Max_Transfer_Queue_Size(void);
int8_t Bluenrg_Get_Max_CallBack_Queue_Size(void);


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLUENRG_DRIVER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
