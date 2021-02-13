

#ifndef BLUENRG_MS_DRIVER_H_
#define BLUENRG_MS_DRIVER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "TimeFunctions.h"
#include "hci_transport_layer.h"


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
	DO_NOT_RELEASE_SPI = 0,   /* The return of a SPI transaction callback does not allow SPI communication release */
	RELEASE_SPI 	   = 1	  /* The return of a SPI transaction callback will release SPI communication */
}SPI_RELEASE;


typedef enum
{
	TRANSFER_DONE 		= 0, /* Transfer was successful */
	TRANSFER_DEV_ERROR  = 1, /* Some error occurred with the hardware */
	TRANSFER_TIMEOUT 	= 2  /* Could not within timeout */
}TRANSFER_STATUS;


typedef uint8_t (*TransferCallBack)(void* Header, uint8_t Status);


typedef struct
{
	uint8_t* TxPtr;
	uint8_t* RxPtr;
	uint8_t DataSize;
	//uint32_t TransferTimeout; /*TODO Is it needed? */
	TRANSFER_CALL_BACK_MODE CallBackMode;
	TransferCallBack CallBack; /* Callback called after the operation. If set as NULL is not called. */
}TRANSFER_DESCRIPTOR;


typedef struct
{
	int8_t TxStatus; /* It indicates the status/place in the queue: if 0, it is being transmitted, if 1, will be sent next. If < 0, it is not available  */
	TRANSFER_DESCRIPTOR* TransferDescPtr;
}BUFFER_POSITION_DESC;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void Reset_BluenrgMS(void);
void BluenrgMS_IRQ(void);
BUFFER_POSITION_DESC* BluenrgMS_Get_Free_Frame_Buffer(void);
BUFFER_POSITION_DESC* BluenrgMS_Get_Frame_Buffer_Head(void);
void BluenrgMS_Release_Frame_Buffer(BUFFER_POSITION_DESC* BuffPtr);
void Set_BluenrgMS_Last_Sent_Frame_Buffer(BUFFER_POSITION_DESC* Buf);
BUFFER_POSITION_DESC* Get_BluenrgMS_Last_Sent_Frame_Buffer(void);
uint8_t BluenrgMS_Enqueue_Frame(BUFFER_POSITION_DESC* Buffer, uint8_t position);
extern void Clr_BluenrgMS_Reset_Pin(void);
extern void Set_BluenrgMS_Reset_Pin(void);
extern uint8_t Get_BluenrgMS_IRQ_Pin(void);
extern void Release_BluenrgMS_SPI(void);
extern uint8_t Request_BluenrgMS_Frame_Transmission(void);


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLUENRG_MS_DRIVER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
