

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


typedef enum
{
	BUFFER_FREE 		= 0,
	BUFFER_FULL 		= 1,
	BUFFER_TRANSMITTING = 2
}BUFFER_STATUS;


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
	uint8_t Status; /* It indicates the BUFFER_STATUS */
	TRANSFER_DESCRIPTOR* TransferDescPtr;
	void* Next;
	void* Prev;
}BUFFER_POSITION_DESC;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void Reset_BluenrgMS(void);
void BluenrgMS_IRQ(void);
void BluenrgMS_SPI_Frame_Transfer_Status(TRANSFER_STATUS status);
uint8_t BluenrgMS_Enqueue_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index);

extern void Clr_BluenrgMS_Reset_Pin(void);
extern void Set_BluenrgMS_Reset_Pin(void);
extern uint8_t Get_BluenrgMS_IRQ_Pin(void);
extern void Release_BluenrgMS_SPI(void);
extern uint8_t Send_BluenrgMS_SPI_Frame(uint8_t* TxPtr, uint8_t* RxPtr, uint8_t DataSize);


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLUENRG_MS_DRIVER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
