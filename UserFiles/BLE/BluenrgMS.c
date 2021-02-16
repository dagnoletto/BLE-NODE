

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "BluenrgMS.h"


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define CTRL_WRITE 	  0x0A
#define CTRL_READ  	  0x0B
#define DEVICE_READY  0x02
#define LOCAL_TX_BYTE_BUFFER_SIZE 255 /* Minimum value is 127 */
#define LOCAL_RX_BYTE_BUFFER_SIZE 255 /* Minimum value is 127 */


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef struct
{
	uint8_t CTRL;
	uint8_t Dummy[4];
}SPI_MASTER_HEADER;


typedef struct
{
	uint8_t READY;
	uint8_t WBUF;
	uint8_t Dummy1;
	uint8_t RBUF;
	uint8_t Dummy2;
}SPI_SLAVE_HEADER;


typedef enum
{
	BUFFER_FREE 		= 0,
	BUFFER_FULL 		= 1,
	BUFFER_TRANSMITTING = 2
}BUFFER_STATUS;


typedef struct
{
	uint8_t Status; /* It indicates the BUFFER_STATUS */
	TRANSFER_DESCRIPTOR* TransferDescPtr;
	void* Next;
}BUFFER_POSITION_DESC;


typedef struct
{
	uint8_t AllowedWriteSize;
	uint8_t SizeToRead;
	int8_t NumberOfFilledBuffers;
	BUFFER_POSITION_DESC* BufferHead; /* The head always points to the buffer to be transmitted first */
	BUFFER_POSITION_DESC* BufferTail; /* The tail always points to a free buffer */
	TRANSFER_DESCRIPTOR TXDesc[SIZE_OF_TX_FRAME_BUFFER];
	BUFFER_POSITION_DESC TXBuffer[SIZE_OF_TX_FRAME_BUFFER];
}BUFFER_MANAGEMENT;


typedef enum
{
	DO_NOT_RELEASE_SPI = 0,   /* The return of a SPI transaction callback does not allow SPI communication release */
	RELEASE_SPI 	   = 1	  /* The return of a SPI transaction callback will release SPI communication */
}SPI_RELEASE;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t BluenrgMS_Slave_Header_CallBack(TRANSFER_DESCRIPTOR* Transfer, TRANSFER_STATUS Status);
static void Request_Slave_Header(void);
static void BluenrgMS_Init_Buffer_Manager(void);
inline static BUFFER_POSITION_DESC* BluenrgMS_Search_For_Free_Buffer(void) __attribute__((always_inline));
inline static void Request_BluenrgMS_Frame_Transmission(void) __attribute__((always_inline));
inline static void BluenrgMS_Release_Frame_Buffer(void) __attribute__((always_inline));


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
const SPI_MASTER_HEADER SPIMasterHeaderWrite = { .CTRL = CTRL_WRITE, .Dummy = {0,0,0,} };
const SPI_MASTER_HEADER SPIMasterHeaderRead  = { .CTRL = CTRL_READ, .Dummy = {0,0,0,} };
static SPI_SLAVE_HEADER SPISlaveHeader;
static BUFFER_MANAGEMENT BufferManager;
static uint8_t TXBytes[LOCAL_TX_BYTE_BUFFER_SIZE];
static uint8_t RXBytes[LOCAL_RX_BYTE_BUFFER_SIZE];
static uint32_t TimerResetActive = 0;
static uint8_t ResetBluenrgMSRequest = TRUE;


/****************************************************************/
/* Reset_BluenrgMS()                                            */
/* Purpose: Hardware reset for Bluenrg-MS module 	    		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Reset_BluenrgMS(void)
{
	BluenrgMS_Init_Buffer_Manager();

	Clr_BluenrgMS_Reset_Pin(); /* Put device in hardware reset state */

	ResetBluenrgMSRequest = TRUE;
	TimerResetActive = 0;
}


/****************************************************************/
/* Run_BluenrgMS()               	                            */
/* Purpose: Continuous function call to run the BluenrgMS  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Run_BluenrgMS(void)
{
	if( ResetBluenrgMSRequest )
	{
		BluenrgMS_Init_Buffer_Manager();

		Clr_BluenrgMS_Reset_Pin(); /* Put device in hardware reset state */

		/* The longest value of TResetActive is 94 ms */
		if( TimeBase_DelayMs( &TimerResetActive, 100UL, FALSE) )
		{
			Set_BluenrgMS_Reset_Pin(); /* Put device in running mode */
			ResetBluenrgMSRequest = FALSE;
		}
	}else
	{

	}
}


/****************************************************************/
/* BluenrgMS_Error()               	                            */
/* Purpose: Called whenever the device has a problem	  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void BluenrgMS_Error(BLUENRG_ERROR_CODES Errorcode)
{
	Reset_BluenrgMS();
}


/****************************************************************/
/* BluenrgMS_Init_Buffer_Manager()                              */
/* Purpose: Initialize manager structure				  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void BluenrgMS_Init_Buffer_Manager(void)
{
	for( int8_t i = 0; i < SIZE_OF_TX_FRAME_BUFFER; i++ )
	{
		BufferManager.TXBuffer[i].TransferDescPtr = &BufferManager.TXDesc[i];
		BufferManager.TXBuffer[i].Status = BUFFER_FREE; /* Buffer is free */
	}

	BufferManager.BufferHead = &BufferManager.TXBuffer[0]; /* The head always points to the buffer to be transmitted first */
	BufferManager.BufferHead->Next = &BufferManager.TXBuffer[0]; /* Points to the next buffer for transmission */

	BufferManager.BufferTail = &BufferManager.TXBuffer[0]; /* The tail always points to last filled buffer */
	BufferManager.BufferTail->Next = &BufferManager.TXBuffer[0]; /* Points to the next free buffer */

	BufferManager.AllowedWriteSize = 0;
	BufferManager.SizeToRead = 0;
	BufferManager.NumberOfFilledBuffers = 0;
}


/****************************************************************/
/* BluenrgMS_IRQ()                                              */
/* Purpose: Handler for the Bluenrg-MS IRQ request pin     		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void BluenrgMS_IRQ(void)
{
	/* There is something to read, so enqueue a Slave_Header Request to see how many bytes to read */
	Request_Slave_Header();
}


/****************************************************************/
/* BluenrgMS_Release_Frame_Buffer()                             */
/* Purpose: Release the transfer buffer head.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void BluenrgMS_Release_Frame_Buffer(void)
{
	BufferManager.BufferHead->Status = BUFFER_FREE;
	BufferManager.NumberOfFilledBuffers--;

	if( BufferManager.BufferTail->Next == NULL )
	{
		BufferManager.BufferTail->Next = BufferManager.BufferHead;
	}

	BufferManager.BufferHead = BufferManager.BufferHead->Next;
}


/****************************************************************/
/* BluenrgMS_Enqueue_Frame()            	                    */
/* Purpose: Enqueue the new frame for transmission in the  		*/
/* output buffer												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
FRAME_ENQUEUE_STATUS BluenrgMS_Enqueue_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index)
{
	/* TODO: THIS PROCEDURE CANNOT BE INTERRUPTED BY SPI INTERRUPTS */
	/* BufferTail always represents the last filled buffer.
	 * BufferHead always represents the first filled buffer.
	 * BufferTail->Next points to a free buffer, otherwise is NULL if free buffers could not be found */

	FRAME_ENQUEUE_STATUS Status;

	/* Check if the next buffer after tail is available */
	BUFFER_POSITION_DESC* BufferPtr = BufferManager.BufferTail->Next;

	if( BufferPtr != NULL )
	{
		if( BufferPtr->Status == BUFFER_FREE ) /* Check if buffer is free */
		{

			*( BufferPtr->TransferDescPtr ) = *TransferDescPtr; /* Occupy this buffer */
			BufferPtr->Status = BUFFER_FULL;
			BufferManager.NumberOfFilledBuffers++;

			/* Search for next free buffer */
			BufferPtr->Next = BluenrgMS_Search_For_Free_Buffer();
			BufferManager.BufferTail = BufferPtr;

			if( BufferManager.NumberOfFilledBuffers > 1 ) /* When more than one buffer is loaded, it is possible to reorder */
			{
				/* Makes no sense to reorder if desired position is higher than actual buffer usage */
				int8_t LastFilledPosition = BufferManager.NumberOfFilledBuffers - 1;

				if( buffer_index < LastFilledPosition )
				{
					BUFFER_POSITION_DESC* ParentBuffer = BufferManager.BufferHead;
					BUFFER_POSITION_DESC* DesiredBuffer = BufferManager.BufferHead;
					BUFFER_POSITION_DESC* BeforeDesired = BufferManager.BufferHead;

					if( buffer_index < 0 ){ buffer_index = 0; }
					else if( buffer_index > (SIZE_OF_TX_FRAME_BUFFER - 1) ){ buffer_index = (SIZE_OF_TX_FRAME_BUFFER - 1); }

					if( ( buffer_index == 0 ) && ( BufferManager.BufferHead->Status == BUFFER_TRANSMITTING ) )
					{
						/* Cannot interfere with the head being transmitted */
						buffer_index = 1;
					}

					if( buffer_index < ( BufferManager.NumberOfFilledBuffers - 1 ) )
					{
						/* There exists positions filled in the buffer, trace the path */
						for ( int8_t filled_index = 0; filled_index < SIZE_OF_TX_FRAME_BUFFER; filled_index++ )
						{
							if( filled_index == buffer_index )
							{
								DesiredBuffer = ParentBuffer;
							}else if( filled_index == ( buffer_index - 1 ) )
							{
								BeforeDesired = ParentBuffer;
							}
							if( ParentBuffer->Next == BufferManager.BufferTail )
							{
								break;
							}else
							{
								ParentBuffer = ParentBuffer->Next;
							}
						}

						/* The parent buffer do not point to Tail anymore, but to the "Tail.Next": */
						ParentBuffer->Next = BufferManager.BufferTail->Next;
						BufferManager.BufferTail->Next = DesiredBuffer;

						if( BeforeDesired != DesiredBuffer )
						{
							BeforeDesired->Next = BufferManager.BufferTail;
						}

						if( buffer_index == 0 ) /* If the tails wants to become the head */
						{
							BufferManager.BufferHead = BufferManager.BufferTail;
						}

						BufferManager.BufferTail = ParentBuffer;

						Status.EnqueuedAtIndex = buffer_index;
					}else
					{
						Status.EnqueuedAtIndex = LastFilledPosition;
					}
				}else
				{
					Status.EnqueuedAtIndex = LastFilledPosition;
				}
			}else
			{
				Request_BluenrgMS_Frame_Transmission();

				Status.EnqueuedAtIndex = 0;
			}

			Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;

			return (Status);

		}
	}

	Status.EnqueuedAtIndex = -1; /* Could not enqueue the frame */
	Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;

	BluenrgMS_Error( MESSAGE_QUEUE_IS_FULL );

	return ( Status ); /* Could not enqueue the transfer */
}


/****************************************************************/
/* Request_BluenrgMS_Frame_Transmission()            	        */
/* Purpose: Request buffer transmission.	    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Request_BluenrgMS_Frame_Transmission(void)
{
	if( BufferManager.BufferHead->Status == BUFFER_FULL )
	{
		TRANSFER_DESCRIPTOR* TransferDescPtr = BufferManager.BufferHead->TransferDescPtr;

		BufferManager.BufferHead->Status = BUFFER_TRANSMITTING;

		if( Send_BluenrgMS_SPI_Frame( TransferDescPtr->TxPtr, TransferDescPtr->RxPtr, TransferDescPtr->DataSize ) == FALSE )
		{
			Release_BluenrgMS_SPI();
		}
	}
}


/****************************************************************/
/* Status_BluenrgMS_SPI_Frame()         	     				*/
/* Purpose: SPI transfer completed in hardware	    		   	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Status_BluenrgMS_SPI_Frame(TRANSFER_STATUS status)
{
	SPI_RELEASE ReleaseSPI = RELEASE_SPI;

	if( BufferManager.BufferHead->Status == BUFFER_TRANSMITTING )
	{
		TRANSFER_DESCRIPTOR* TransferDescPtr = BufferManager.BufferHead->TransferDescPtr;

		if( TransferDescPtr->CallBack != NULL )
		{
			if( TransferDescPtr->CallBackMode == CALL_BACK_AFTER_TRANSFER )
			{
				ReleaseSPI = TransferDescPtr->CallBack( TransferDescPtr, status );
			}else
			{
				//TODO: put in the callback queue: the transfer and data must be copied in order to release the transfer buffer below
			}
		}

		BluenrgMS_Release_Frame_Buffer();
	}

	if( ReleaseSPI == RELEASE_SPI )
	{
		Release_BluenrgMS_SPI();
	}

	Request_BluenrgMS_Frame_Transmission();
}


/****************************************************************/
/* BluenrgMS_Search_For_Free_Buffer()                           */
/* Purpose: Find free buffer or return NULL						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static BUFFER_POSITION_DESC* BluenrgMS_Search_For_Free_Buffer(void)
{
	/* Search for an available free buffer. This function is not optimized
	 * in the sense it could start from a speculative index based on current
	 * buffer usage to more quickly find a buffer. */
	for ( int8_t i = 0; i < SIZE_OF_TX_FRAME_BUFFER; i++ )
	{
		if( BufferManager.TXBuffer[i].Status == BUFFER_FREE )
		{
			return ( &BufferManager.TXBuffer[i] );
		}
	}
	return ( NULL );
}


/****************************************************************/
/* BluenrgMS_Slave_Header_CallBack()                            */
/* Purpose: After sending a master header, the slave would send */
/* its header			  										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t BluenrgMS_Slave_Header_CallBack(TRANSFER_DESCRIPTOR* Transfer, TRANSFER_STATUS Status)
{
	//TODO: fazer a leitura consecutiva neste ponto
	SPI_SLAVE_HEADER* Header = (SPI_SLAVE_HEADER*)( Transfer->RxPtr );

	if( Header->READY == DEVICE_READY )
	{
		BufferManager.AllowedWriteSize = Header->WBUF;
		BufferManager.SizeToRead = Header->RBUF;
		/* TODO: enqueue new packet */
		/* TODO: check IRQ pin as below */
		/* do not release spi if the next command is a read */
		//return (DO_NOT_RELEASE_SPI); /* SPI was already release and a new transfer was requested */

		if( BufferManager.SizeToRead != 0 ) /* Read operation has the higher priority */
		{
			/* TODO: enqueue new cmd do not release spi if the next command is a read */
			return (DO_NOT_RELEASE_SPI); /* For the read operation, keeps device asserted and send dummy bytes MOSI */
		}else if( BufferManager.AllowedWriteSize != 0 )
		{
			//TODO: verify if there are commands to write and if there have datalength equal or lower the ammount available
		}else
		{
			/* If there are still commands to process, make a header read to know when device has allowed more data transfer */
			if (BufferManager.NumberOfFilledBuffers)
			{
				/* Enqueue a new slave header read to check when device allows more data */
				Request_Slave_Header();
			}
		}
	}else
	{
		BufferManager.AllowedWriteSize = 0;
		BufferManager.SizeToRead = 0;

		if( Get_BluenrgMS_IRQ_Pin() ) /* Check if the IRQ pin is set */
		{
			/* Enqueue a new slave header read to check if device is ready */
			Request_Slave_Header();
		}
	}

	return (RELEASE_SPI);
}


/****************************************************************/
/* Request_Slave_Header()                   			        */
/* Purpose: Read the slave header								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Request_Slave_Header(void)
{
	TRANSFER_DESCRIPTOR TransferDesc;

	/* Load transmitting queue position */
	TransferDesc.TxPtr = (uint8_t*)( &SPIMasterHeaderRead.CTRL );
	TransferDesc.RxPtr = &SPISlaveHeader.READY;
	TransferDesc.DataSize = sizeof(SPISlaveHeader); /* Put always the size of reading buffer to avoid writing in random memory */
	TransferDesc.CallBackMode = CALL_BACK_AFTER_TRANSFER;
	TransferDesc.CallBack = (TransferCallBack)( &BluenrgMS_Slave_Header_CallBack );

	/* We need to know if we have to read or how much we can write, so put the command in the first position of the queue */
	BluenrgMS_Enqueue_Frame( &TransferDesc, 0 );
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
