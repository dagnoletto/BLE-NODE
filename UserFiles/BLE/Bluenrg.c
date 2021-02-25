

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Bluenrg.h"


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define CTRL_WRITE 	  0x0A
#define CTRL_READ  	  0x0B
#define DEVICE_READY  0x02
#define SIZE_OF_LOCAL_MEMORY_BUFFER 255
#define DEVICE_NOT_READY_THRESHOLD	  3 /* Number of not ready responses in sequence to put device in hold mode for DEVICE_HOLD_TIME */
#define NO_ALLOWED_WRITE_THRESHOLD	  3 /* Number of not allowed write responses (in sequence) to put device in hold mode for DEVICE_HOLD_TIME */
#define ERRONEOUS_RESPONSE_THRESHOLD  3 /* Number of erroneous responses (in sequence) to put device in hold mode for DEVICE_HOLD_TIME */
#define DEVICE_HOLD_TIME			  20UL /* Time in ms to wait until frames are sent to the device again after
											device not ready for DEVICE_NOT_READY_THRESHOLD times */
#define SIZE_OF_FRAME_BUFFER 		  8
#define SIZE_OF_CALLBACK_BUFFER 	  4


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
	BUFFER_TRANSMITTING = 2,
	BUFFER_PAUSED 		= 3
}BUFFER_STATUS;


typedef struct
{
	uint8_t Status; /* It indicates the BUFFER_STATUS */
	uint8_t TransferMode; /* It indicates the SPI_TRANSFER_MODE */
	uint8_t TransferStatus; /* It indicates the TRANSFER_STATUS */
	uint8_t* TxPtr;
	uint8_t* RxPtr;
	uint16_t RemainingBytes;
	uint16_t Counter;
	void* Next;
	TRANSFER_DESCRIPTOR TransferDesc;
}BUFFER_DESC;


typedef struct
{
	uint16_t AllowedWriteSize;
	uint16_t SizeToRead;
	uint32_t HoldTime;
	uint32_t HoldCounter;
	int8_t NumberOfFilledBuffers;
	BUFFER_DESC* BufferHead; /* The head always points to the buffer to be transmitted first */
	BUFFER_DESC* BufferTail; /* The tail always points to a free buffer */
	BUFFER_DESC Buffer[SIZE_OF_FRAME_BUFFER];
}BUFFER_MANAGEMENT;


typedef struct
{
	uint8_t Status; /* It indicates the BUFFER_STATUS */
	uint8_t TransferStatus; /* It indicates the TRANSFER_STATUS */
	void* Next;
	TRANSFER_DESCRIPTOR TransferDesc;
}CALLBACK_DESC;


typedef struct
{
	int8_t NumberOfFilledBuffers;
	CALLBACK_DESC* CallBackHead; /* The head always points to the callback to be handled first */
	CALLBACK_DESC* CallBackTail; /* The tail always points to a free callback */
	CALLBACK_DESC CallBack[SIZE_OF_CALLBACK_BUFFER];
}CALLBACK_MANAGEMENT;


typedef enum
{
	DO_NOT_RELEASE_SPI = 0,   /* The return of a SPI transaction callback does not allow SPI communication release */
	RELEASE_SPI 	   = 1	  /* The return of a SPI transaction callback will release SPI communication */
}SPI_RELEASE;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Slave_Header_CallBack(TRANSFER_DESCRIPTOR* Transfer, SPI_TRANSFER_MODE HeaderMode, TRANSFER_STATUS Status);
static uint8_t Slave_Read_Complete_CallBack(TRANSFER_DESCRIPTOR* Transfer, uint16_t DataSize, TRANSFER_STATUS Status);
static uint8_t Receiver_Multiplexer(uint8_t* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);
static void Request_Slave_Header(SPI_TRANSFER_MODE HeaderMode);
static void Init_Buffer_Manager(void);
static FRAME_ENQUEUE_STATUS Enqueue_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index, SPI_TRANSFER_MODE TransferMode);
inline static BUFFER_DESC* Search_For_Free_Frame(void) __attribute__((always_inline));
inline static void Release_Frame(void) __attribute__((always_inline));
static uint8_t Enqueue_CallBack(BUFFER_DESC* Buffer);
inline static CALLBACK_DESC* Search_For_Free_CallBack(void) __attribute__((always_inline));
inline static void Release_CallBack(void) __attribute__((always_inline));
static void Request_Frame(void);
static FRAME_ENQUEUE_STATUS Add_Rx_Frame(uint16_t DataSize, int8_t buffer_index);
static uint8_t* Internal_malloc(TRANSFER_DESCRIPTOR* TransferDescPtr, uint16_t DataSize);
static void Internal_free(TRANSFER_DESCRIPTOR* TransferDescPtr);


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
static CALLBACK_MANAGEMENT CallBackManager;
static uint8_t LocalBytes[SIZE_OF_LOCAL_MEMORY_BUFFER];
static uint8_t DummyByte;
static uint32_t TimerResetActive = 0;
static uint8_t ResetBluenrgRequest = TRUE;
static uint8_t FirstBluenrgReset = TRUE;
static uint8_t LocalMemoryUsed = FALSE;


/****************************************************************/
/* Reset_Bluenrg()                                        	    */
/* Purpose: Hardware reset for Bluenrg module 	    			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Reset_Bluenrg(void)
{
	Clr_Bluenrg_Reset_Pin(); /* Put device in hardware reset state */

	ResetBluenrgRequest = TRUE;

	TimerResetActive = 0;

	Init_Buffer_Manager();
}


/****************************************************************/
/* Run_Bluenrg()               	                            	*/
/* Purpose: Continuous function call to run the Bluenrg  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Run_Bluenrg(void)
{
	if( ResetBluenrgRequest )
	{
		Clr_Bluenrg_Reset_Pin(); /* Put device in hardware reset state */

		Init_Buffer_Manager();

		/* The longest value of TResetActive is 94 ms */
		if( TimeBase_DelayMs( &TimerResetActive, 100UL, FALSE) )
		{
			Set_Bluenrg_Reset_Pin(); /* Put device in running mode */
			ResetBluenrgRequest = FALSE;
		}
	}else
	{
		if( CallBackManager.CallBackHead->Status != BUFFER_FREE )
		{
			/* TODO: if another message uses the same function callback, it can be called by the interrupt handler, overwriting the function being treated here. Just think of it.  */
			TRANSFER_DESCRIPTOR* TransferDescPtr = &CallBackManager.CallBackHead->TransferDesc;

			TransferDescPtr->CallBack( TransferDescPtr->DataPtr, TransferDescPtr->DataSize, CallBackManager.CallBackHead->TransferStatus );

			Internal_free( TransferDescPtr );

			Release_CallBack();
		}

		/* If Hold time is active, no more messages can be sent for the duration of hold */
		if( BufferManager.HoldTime )
		{
			if( TimeBase_DelayMs( &BufferManager.HoldCounter, BufferManager.HoldTime, TRUE) )
			{
				BufferManager.HoldTime = 0;
				Request_Frame();
			}
		}
	}
}


/****************************************************************/
/* Bluenrg_Error()               	                            */
/* Purpose: Called whenever the device has a problem	  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void Bluenrg_Error(BLUENRG_ERROR_CODES Errorcode)
{
	Reset_Bluenrg();
}


/****************************************************************/
/* Init_Buffer_Manager()                   			         	*/
/* Purpose: Initialize manager structure				  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Init_Buffer_Manager(void)
{
	for( int8_t i = 0; i < SIZE_OF_FRAME_BUFFER; i++ )
	{
		if(FirstBluenrgReset)
		{
			BufferManager.Buffer[i].TransferDesc.DataPtr = NULL;
		}else
		{
			Internal_free( &BufferManager.Buffer[i].TransferDesc );
		}
		BufferManager.Buffer[i].Status = BUFFER_FREE; /* Buffer is free */
	}

	BufferManager.HoldTime = 0;
	BufferManager.HoldCounter = 0;
	BufferManager.BufferHead = &BufferManager.Buffer[0]; /* The head always points to the buffer to be transmitted first */
	BufferManager.BufferHead->Next = &BufferManager.Buffer[0]; /* Points to the next buffer for transmission */

	BufferManager.BufferTail = &BufferManager.Buffer[0]; /* The tail always points to last filled buffer */
	BufferManager.BufferTail->Next = &BufferManager.Buffer[0]; /* Points to the next free buffer */

	BufferManager.AllowedWriteSize = 0;
	BufferManager.SizeToRead = 0;
	BufferManager.NumberOfFilledBuffers = 0;


	/* Callback buffer reset */
	for( int8_t i = 0; i < SIZE_OF_CALLBACK_BUFFER; i++ )
	{
		CallBackManager.CallBack[i].Status = BUFFER_FREE; /* Buffer is free */
	}

	CallBackManager.CallBackHead = &CallBackManager.CallBack[0]; /* The head always points to the callback to be handled first */
	CallBackManager.CallBackHead->Next = &CallBackManager.CallBack[0]; /* Points to the next callback for handling */

	CallBackManager.CallBackTail = &CallBackManager.CallBack[0]; /* The tail always points to last filled callback */
	CallBackManager.CallBackTail->Next = &CallBackManager.CallBack[0]; /* Points to the next free callback */

	CallBackManager.NumberOfFilledBuffers = 0;

	FirstBluenrgReset = FALSE;
}


/****************************************************************/
/* Bluenrg_IRQ()                                              	*/
/* Purpose: Handler for the Bluenrg-MS IRQ request pin     		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Bluenrg_IRQ(void)
{
	/* There is something to read, so enqueue a Slave_Header Request to see how many bytes to read */
	Request_Slave_Header( SPI_HEADER_READ );
	Request_Frame();
}


/****************************************************************/
/* Release_Frame()                        				    	*/
/* Purpose: Release the transfer buffer head.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Release_Frame(void)
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
/* Bluenrg_Add_Frame()         	 		              	    	*/
/* Purpose: Add HCI packet to the transmitting queue			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
FRAME_ENQUEUE_STATUS Bluenrg_Add_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index)
{
	FRAME_ENQUEUE_STATUS Status;
	uint8_t* DataPtr = TransferDescPtr->DataPtr; /* Save the caller's data pointer */
	TransferDescPtr->DataPtr = NULL; /* Buffer not allocated */

	if( Internal_malloc( TransferDescPtr, TransferDescPtr->DataSize ) != NULL )
	{
		/* The data MUST be ready at the buffer BEFORE the frame is enqueued */
		memcpy( (uint8_t*)(TransferDescPtr->DataPtr), DataPtr, TransferDescPtr->DataSize );

		/* External calls are always write calls. Read calls are triggered by IRQ pin. */
		Status = Enqueue_Frame(TransferDescPtr, buffer_index, SPI_WRITE);

		if( Status.EnqueuedAtIndex < 0 )
		{
			/* It was not possible to enqueue the frame, so dispose allocated memory  */
			Internal_free( TransferDescPtr );
		}else
		{
			/* This is the first successfully enqueued write message, so, request transmission */
			if( ( Status.EnqueuedAtIndex == 0 ) && ( BufferManager.NumberOfFilledBuffers == 1 ) )
			{
				/* Request transmission */
				Request_Frame();
			}
		}
	}else
	{
		Status.EnqueuedAtIndex = -1; /* Memory could'nt be allocated */
		Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;

		Bluenrg_Error( NO_MEMORY_AVAILABLE );
	}

	return ( Status );
}


/****************************************************************/
/* Release_CallBack()                     				        */
/* Purpose: Release the callback head.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Release_CallBack(void)
{
	CallBackManager.CallBackHead->Status = BUFFER_FREE;
	CallBackManager.NumberOfFilledBuffers--;

	if( CallBackManager.CallBackTail->Next == NULL )
	{
		CallBackManager.CallBackTail->Next = CallBackManager.CallBackHead;
	}

	CallBackManager.CallBackHead = CallBackManager.CallBackHead->Next;
}


/****************************************************************/
/* Enqueue_CallBack()            	   			                */
/* Purpose: Enqueue a new callback for asynchronous handling	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Enqueue_CallBack(BUFFER_DESC* Buffer)
{
	/* TODO: THIS PROCEDURE CANNOT BE INTERRUPTED BY SPI INTERRUPTS? */
	/* CallBackTail always represents the last filled callback.
	 * CallBackHead always represents the first filled callback.
	 * CallBackTail->Next points to a free callback, otherwise is NULL if free callback could not be found */

	/* Check if the next callback after tail is available */
	CALLBACK_DESC* CallBackPtr = CallBackManager.CallBackTail->Next;

	if( CallBackPtr != NULL )
	{
		if( CallBackPtr->Status == BUFFER_FREE ) /* Check if buffer is free */
		{

			CallBackPtr->TransferDesc = Buffer->TransferDesc; /* Occupy this buffer */
			CallBackPtr->Status = BUFFER_FULL;
			CallBackPtr->Next = Search_For_Free_CallBack();

			CallBackPtr->TransferStatus = Buffer->TransferStatus;

			CallBackManager.NumberOfFilledBuffers++;

			CallBackManager.CallBackTail = CallBackPtr;

			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Search_For_Free_CallBack()           		                */
/* Purpose: Find free callback or return NULL					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static CALLBACK_DESC* Search_For_Free_CallBack(void)
{
	/* Search for an available free callback. This function is not optimized
	 * in the sense it could start from a speculative index based on current
	 * callback usage to more quickly find a buffer. */
	for ( int8_t i = 0; i < SIZE_OF_CALLBACK_BUFFER; i++ )
	{
		if( CallBackManager.CallBack[i].Status == BUFFER_FREE )
		{
			return ( &CallBackManager.CallBack[i] );
		}
	}
	return ( NULL );
}


/****************************************************************/
/* Enqueue_Frame()            	     			               	*/
/* Purpose: Enqueue the new frame for transmission in the  		*/
/* output buffer												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static FRAME_ENQUEUE_STATUS Enqueue_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index, SPI_TRANSFER_MODE TransferMode)
{
	/* TODO: THIS PROCEDURE CANNOT BE INTERRUPTED BY SPI INTERRUPTS */
	/* BufferTail always represents the last filled buffer.
	 * BufferHead always represents the first filled buffer.
	 * BufferTail->Next points to a free buffer, otherwise is NULL if free buffers could not be found */

	FRAME_ENQUEUE_STATUS Status;

	/* Check if the next buffer after tail is available */
	BUFFER_DESC* BufferPtr = BufferManager.BufferTail->Next;

	if( BufferPtr != NULL )
	{
		if( BufferPtr->Status == BUFFER_FREE ) /* Check if buffer is free */
		{

			BufferPtr->TransferDesc = *TransferDescPtr; /* Occupy this buffer */
			BufferPtr->Status = BUFFER_FULL;
			BufferPtr->TransferMode = TransferMode;
			BufferPtr->RemainingBytes = TransferDescPtr->DataSize;
			BufferPtr->Counter = 0;
			BufferPtr->Next = Search_For_Free_Frame();

			if( TransferMode == SPI_HEADER_READ )
			{
				BufferPtr->TxPtr = (uint8_t*)&SPIMasterHeaderRead.CTRL;
				BufferPtr->RxPtr = TransferDescPtr->DataPtr;
			}else if( TransferMode == SPI_HEADER_WRITE )
			{
				BufferPtr->TxPtr = (uint8_t*)&SPIMasterHeaderWrite.CTRL;
				BufferPtr->RxPtr = TransferDescPtr->DataPtr;
			}else if( TransferMode == SPI_WRITE )
			{
				BufferPtr->TxPtr = TransferDescPtr->DataPtr;
				BufferPtr->RxPtr = &DummyByte; /* Just to have a valid pointer */
			}else
			{
				/* The slave header always precede the read operation, so we don't need a full valid TX dummy buffer */
				BufferPtr->TxPtr = &DummyByte; /* Just to have a valid pointer */
				BufferPtr->RxPtr = TransferDescPtr->DataPtr;
			}

			BufferManager.NumberOfFilledBuffers++;

			/* Search for next free buffer */
			BufferManager.BufferTail = BufferPtr;

			if( BufferManager.NumberOfFilledBuffers > 1 ) /* When more than one buffer is loaded, it is possible to reorder */
			{
				/* Makes no sense to reorder if desired position is higher than actual buffer usage */
				int8_t LastFilledPosition = BufferManager.NumberOfFilledBuffers - 1;

				if( buffer_index < LastFilledPosition )
				{
					BUFFER_DESC* ParentBuffer = BufferManager.BufferHead;
					BUFFER_DESC* DesiredBuffer = BufferManager.BufferHead;
					BUFFER_DESC* BeforeDesired = BufferManager.BufferHead;

					if( buffer_index < 0 ){ buffer_index = 0; }
					else if( buffer_index > (SIZE_OF_FRAME_BUFFER - 1) ){ buffer_index = (SIZE_OF_FRAME_BUFFER - 1); }

					if( buffer_index == 0 ) /* The new transfer wants to be the head of the queue */
					{
						if( BufferManager.BufferHead->Status == BUFFER_TRANSMITTING )
						{
							/* Cannot be the head since a transfer is ongoing */
							buffer_index = 1;
						}else if( ( BufferManager.BufferHead->Status == BUFFER_PAUSED ) &&
								( BufferPtr->TransferMode != SPI_HEADER_READ ) && ( BufferPtr->TransferMode != SPI_HEADER_WRITE ) )
						{
							/* The head is paused but the candidate transfer is not a header read/write to take over priority */
							buffer_index = 1;
						}
					}

					if( buffer_index < ( BufferManager.NumberOfFilledBuffers - 1 ) )
					{
						/* There exists positions filled in the buffer, trace the path */
						for ( int8_t filled_index = 0; filled_index < SIZE_OF_FRAME_BUFFER; filled_index++ )
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
				Status.EnqueuedAtIndex = 0;
			}

			Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;

			return (Status);

		}
	}

	Status.EnqueuedAtIndex = -1; /* Could not enqueue the frame */
	Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;

	Bluenrg_Error( QUEUE_IS_FULL );

	return ( Status ); /* Could not enqueue the transfer */
}


/****************************************************************/
/* Request_Frame()            	        						*/
/* Purpose: Request buffer transmission.	    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Request_Frame(void)
{
	uint16_t DataSize;
	uint8_t* TXDataPtr;

	CheckBufferHead:

	if( !BufferManager.HoldTime ) /* We are not holding */
	{
		if( ( BufferManager.BufferHead->Status != BUFFER_FREE ) && ( BufferManager.BufferHead->Status != BUFFER_TRANSMITTING ) )
		{
			switch( BufferManager.BufferHead->TransferMode )
			{

			case SPI_WRITE:
				/* When the write is called for the first time, the slave header read is requested to update AllowedWriteSize */
				if( ( BufferManager.AllowedWriteSize == 0 ) || ( BufferManager.BufferHead->Status == BUFFER_FULL ) )
				{
					BufferManager.BufferHead->Status = BUFFER_PAUSED;
					Request_Slave_Header( SPI_HEADER_WRITE );
					goto CheckBufferHead; /* I know, I know, ugly enough. But think of code savings and performance, OK? */
				}else
				{
					/* We can write the data, but check if all bytes can be sent at once or not */
					if( BufferManager.BufferHead->RemainingBytes > BufferManager.AllowedWriteSize )
					{
						DataSize = BufferManager.AllowedWriteSize;
					}else
					{
						DataSize = BufferManager.BufferHead->RemainingBytes;
					}

					BufferManager.BufferHead->TxPtr += BufferManager.BufferHead->Counter;
					TXDataPtr = BufferManager.BufferHead->TxPtr;

					BufferManager.BufferHead->RemainingBytes -= DataSize;
					BufferManager.BufferHead->Counter += DataSize;

					/* Force the reading of a new header before every new transfer just to make sure the module is ready
					 * and has valid buffer space to accept data */
					BufferManager.AllowedWriteSize = 0;
				}
				break;

			case SPI_READ:
				DataSize = BufferManager.BufferHead->RemainingBytes;
				TXDataPtr = BufferManager.BufferHead->TxPtr;

				if( ( BufferManager.SizeToRead == 0 ) || ( DataSize < BufferManager.SizeToRead ) )
				{
					/* If Size to read is zero or available buffer size is lower than the necessary, trigger an Error because we do not consider
					 * partial transfer at transport layer. Also, the read is always triggered by Bluenrg IRQ pin. So, it is expected SizeToRead to be non zero
					 * whenever this code is reached. */
					/* Here we consider that all read bytes belongs to the same information being reported. That is, when a read is available, it is assumed
				that all bytes regarding to a packet type (HCI event packet, for example) will be transferred in a single read transfer. The Bluenrg does
				specify a read transfer protocol to deal with partial transfer at transport layer. The maximum parameter size according to BLE is
				one byte long. So, the maximum message size will be 255 bytes excluding the header size. However, the ACL_Data_Packet has a data length
				parameter of 16 bits long. Therefore, for ACL_Data_Packets in the future, a transport protocol might be needed. */

					Bluenrg_Error( READ_ARGS_ERROR );

					return;
				}
				break;

			case SPI_HEADER_READ:
			case SPI_HEADER_WRITE:
				/* No restrictions about SPI header read since it is used to get slave status */
				DataSize = BufferManager.BufferHead->RemainingBytes;
				TXDataPtr = BufferManager.BufferHead->TxPtr;

				break;

			default:

				Bluenrg_Error( UNKNOWN_ERROR );

				return;

				break;
			}

			BufferManager.BufferHead->Status = BUFFER_TRANSMITTING;

			if( Bluenrg_Send_Frame( BufferManager.BufferHead->TransferMode, TXDataPtr,
					BufferManager.BufferHead->RxPtr, DataSize ) == FALSE )
			{
				Release_Bluenrg();
			}
		}
	}
}


/****************************************************************/
/* Bluenrg_Frame_Status()         	     						*/
/* Purpose: SPI transfer completed in hardware	    		   	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Bluenrg_Frame_Status(TRANSFER_STATUS status)
{
	if( BufferManager.BufferHead->Status == BUFFER_TRANSMITTING )
	{
		SPI_RELEASE ReleaseSPI = RELEASE_SPI;

		BufferManager.BufferHead->TransferStatus = status;

		/* The write is the only operation that can take more than one transfer */
		if( ( BufferManager.BufferHead->TransferMode == SPI_WRITE ) &&
				( BufferManager.BufferHead->TransferStatus == TRANSFER_DONE ) &&
				( BufferManager.BufferHead->RemainingBytes != 0 ) )
		{

			/* The operation is write, the last transfer was OK but we still have remaining bytes */
			BufferManager.BufferHead->Status = BUFFER_PAUSED;
			Release_Bluenrg();

		}else
		{
			TRANSFER_DESCRIPTOR* TransferDescPtr = &BufferManager.BufferHead->TransferDesc;

			if( TransferDescPtr->CallBack != NULL ) /* We have callback */
			{

				/* TODO: pode haver um problema de acesso mútuo se um mesmo callback estiver sendo processado de forma assíncrona e for chamado
				 * dentro da interrupção */

				if( TransferDescPtr->CallBackMode == CALL_BACK_AFTER_TRANSFER )
				{
					if( ( BufferManager.BufferHead->TransferMode == SPI_HEADER_READ ) || ( BufferManager.BufferHead->TransferMode == SPI_HEADER_WRITE ) )
					{

						ReleaseSPI = TransferDescPtr->CallBack( TransferDescPtr, BufferManager.BufferHead->TransferMode, BufferManager.BufferHead->TransferStatus );

						/* The SPI_HEADER_READ must not be deallocated since its memory is not dynamic. Just "lies" that memory is freed. */
						TransferDescPtr->DataPtr = NULL;

					}else if( BufferManager.BufferHead->TransferMode == SPI_WRITE )
					{

						TransferDescPtr->CallBack( TransferDescPtr->DataPtr, TransferDescPtr->DataSize, BufferManager.BufferHead->TransferStatus );
						Internal_free( TransferDescPtr );

					}else /* SPI_READ */
					{

						/* Read a first time to check what to do with the data */
						TransferDescPtr->CallBack( TransferDescPtr, TransferDescPtr->DataSize, BufferManager.BufferHead->TransferStatus );

						/* The SPI read enqueues the multiplexer function after the first read  */
						if( TransferDescPtr->CallBackMode == CALL_BACK_AFTER_TRANSFER )
						{
							/* Call the second time (now is the multiplexer function) */
							TransferDescPtr->CallBack( TransferDescPtr->DataPtr, TransferDescPtr->DataSize, BufferManager.BufferHead->TransferStatus );
							Internal_free( TransferDescPtr );
						}else
						{
							/* The multiplex function will be called asynchronously */
							/* Put in the callback queue: the transfer and data must be copied in order to release the transfer buffer */
							if( Enqueue_CallBack( BufferManager.BufferHead ) != TRUE )
							{
								Internal_free( TransferDescPtr );
								Bluenrg_Error( QUEUE_IS_FULL );
							}
							/* This is not a real deallocation, because the callback queue might still be using the pointer. It is just to free the buffer for new allocation */
							TransferDescPtr->DataPtr = NULL;
						}
					}
				}else
				{
					/* Put in the callback queue: the transfer and data must be copied in order to release the transfer buffer */
					if( Enqueue_CallBack( BufferManager.BufferHead ) != TRUE )
					{
						Internal_free( TransferDescPtr );
						Bluenrg_Error( QUEUE_IS_FULL );
					}
					/* This is not a real deallocation, because the callback queue might still be using the pointer. It is just to free the buffer for new allocation */
					TransferDescPtr->DataPtr = NULL;
				}
			}else
			{
				Internal_free( TransferDescPtr );
			}

			/* After a read or write we should always release the SPI. For the following reasons: */
			/*
			 * After read: all read is done in a single transfer, so we don't need to keep active after ending it.
			 * After write: after a write, the system should issue a header read to see if more bytes can be sent. In order
			 * to do it, the SPI must be released.
			 *
			 * After a header read, the system can release the SPI if nothing is going to happen after that or must
			 * keep it selected if the next operation is a read. In general, after a header read done due to IRQ pin,
			 * the host should keep SPI selected to read the data (if module is ready, of course).
			 */
			if( ( BufferManager.BufferHead->TransferMode == SPI_WRITE ) || ( BufferManager.BufferHead->TransferMode == SPI_READ ) )
			{
				Release_Bluenrg();

			}else if( ReleaseSPI == RELEASE_SPI )
			{
				Release_Bluenrg();
			}

			Release_Frame();
		}

		Request_Frame();
	}
}


/****************************************************************/
/* Search_For_Free_Frame()                  		         	*/
/* Purpose: Find free buffer or return NULL						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static BUFFER_DESC* Search_For_Free_Frame(void)
{
	/* Search for an available free buffer. This function is not optimized
	 * in the sense it could start from a speculative index based on current
	 * buffer usage to more quickly find a buffer. */
	for ( int8_t i = 0; i < SIZE_OF_FRAME_BUFFER; i++ )
	{
		if( BufferManager.Buffer[i].Status == BUFFER_FREE )
		{
			return ( &BufferManager.Buffer[i] );
		}
	}
	return ( NULL );
}


/****************************************************************/
/* Slave_Header_CallBack()                 			           	*/
/* Purpose: After sending a master header, the slave would send */
/* its header			  										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Slave_Header_CallBack(TRANSFER_DESCRIPTOR* Transfer, SPI_TRANSFER_MODE HeaderMode, TRANSFER_STATUS Status)
{
	uint8_t static DeviceNotReadyCounter = 0;
	uint8_t static NoAllowedWriteCounter = 0;
	uint8_t static ErroneousResponseCounter = 0;

	SPI_SLAVE_HEADER* Header = (SPI_SLAVE_HEADER*)( Transfer->DataPtr );

	if( Header->READY == DEVICE_READY )
	{
		DeviceNotReadyCounter = 0;

		BufferManager.AllowedWriteSize = Header->WBUF;
		BufferManager.SizeToRead = Header->RBUF;

		/* Read operation has higher priority: it can pause an uncompleted write transaction to read available bytes in the module */
		/* If the ongoing write should be finished first before the read, we should simply enqueue a read here and in the
		   Request_Frame function we should make sure to enqueue a header read request before the reading. */
		if( BufferManager.SizeToRead != 0 )
		{
			ErroneousResponseCounter = 0;
			NoAllowedWriteCounter = 0;
			BufferManager.AllowedWriteSize = 0; /* Just to make sure that an ongoing write transaction will request the header before restart */

			/* Enqueue a read command at the second buffer position (index == 1) and do not release the SPI */
			Add_Rx_Frame( BufferManager.SizeToRead, 1 );

			return (DO_NOT_RELEASE_SPI); /* For the read operation, keeps device asserted and send dummy bytes MOSI */

		}else if( HeaderMode == SPI_HEADER_WRITE )
		{
			ErroneousResponseCounter = 0;

			if( BufferManager.AllowedWriteSize != 0 )
			{
				NoAllowedWriteCounter = 0;
				return (DO_NOT_RELEASE_SPI); /* For the write operation, keeps device asserted to send payload bytes */
			}else
			{
				NoAllowedWriteCounter++;
				if( NoAllowedWriteCounter >= NO_ALLOWED_WRITE_THRESHOLD )
				{
					BufferManager.HoldTime = DEVICE_HOLD_TIME; /* hold time */
					BufferManager.HoldCounter = 0;	/* From now */
				}
			}
		}else
		{
			NoAllowedWriteCounter = 0;

			if( Get_Bluenrg_IRQ_Pin() ) /* Check if the IRQ pin is set */
			{
				/* Enqueue a new slave header read to check if device has bytes to read */
				Request_Slave_Header( SPI_HEADER_READ );
			}

			ErroneousResponseCounter++;
			if( ErroneousResponseCounter >= ERRONEOUS_RESPONSE_THRESHOLD )
			{
				BufferManager.HoldTime = DEVICE_HOLD_TIME; /* hold time */
				BufferManager.HoldCounter = 0;	/* From now */
			}
		}
	}else
	{
		/* If the SPI clock frequency to access the device is too high, it is necessary to read the header
		 * more times to get the ready status. Experiences with the BlueNRG-MS showed that above 500kbps,
		 * the DeviceNotReadyCounter increases up to two until DEVICE_READY is achieved. However, if very low
		 * frequency is used, you cannot achieve the best BLE throughput of 1 Mbps */

		NoAllowedWriteCounter = 0;
		ErroneousResponseCounter = 0;

		BufferManager.AllowedWriteSize = 0;
		BufferManager.SizeToRead = 0;

		if( Get_Bluenrg_IRQ_Pin() ) /* Check if the IRQ pin is set */
		{
			/* Enqueue a new slave header read to check if device is ready */
			Request_Slave_Header( SPI_HEADER_READ );
		}

		DeviceNotReadyCounter++;
		if( DeviceNotReadyCounter >= DEVICE_NOT_READY_THRESHOLD )
		{
			BufferManager.HoldTime = DEVICE_HOLD_TIME; /* hold time */
			BufferManager.HoldCounter = 0;	/* From now */
		}
	}

	return (RELEASE_SPI);
}


/****************************************************************/
/* Slave_Read_Complete_CallBack()   	                       	*/
/* Purpose: After receiving the slave read bytes.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Slave_Read_Complete_CallBack(TRANSFER_DESCRIPTOR* Transfer, uint16_t DataSize, TRANSFER_STATUS Status)
{
	Transfer->CallBackMode = CALL_BACK_BUFFERED;
	Transfer->CallBack = (TransferCallBack)( &Receiver_Multiplexer ); /* Call the multiplexer do decode the message */

	Bluenrg_CallBack_Config( &(Transfer->CallBackMode), (HCI_PACKET_TYPE)(*Transfer->DataPtr), (Transfer->DataPtr + 1) );

	return (RELEASE_SPI);
}


/****************************************************************/
/* Bluenrg_CallBack_Config()   	       				         	*/
/* Purpose: Configure the way callback is called				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void Bluenrg_CallBack_Config(TRANSFER_CALL_BACK_MODE* CallBackMode, HCI_PACKET_TYPE PacketType, uint8_t* DataPtr)
{

	/* This function may be implemented in higher layers to allow the upper layers to priors the
	 * handling of different received packets: they are handled either just after received or
	 * are put in a queue to be handled afterwards */


	/* By default, all packet types received are handled after transfer, that is, are put in a queue postponing its processing */
	*CallBackMode = CALL_BACK_BUFFERED;
}


/****************************************************************/
/* Request_Slave_Header()                   			        */
/* Purpose: Read the slave header								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Request_Slave_Header(SPI_TRANSFER_MODE HeaderMode)
{
	TRANSFER_DESCRIPTOR TransferDesc;

	/* Load transmitting queue position */
	TransferDesc.DataPtr = &SPISlaveHeader.READY;
	TransferDesc.DataSize = sizeof(SPISlaveHeader); /* Put always the size of reading buffer to avoid writing in random memory */
	TransferDesc.CallBackMode = CALL_BACK_AFTER_TRANSFER;
	TransferDesc.CallBack = (TransferCallBack)( &Slave_Header_CallBack );

	/* We need to know if we have to read or how much we can write, so put the command in the first position of the queue */
	Enqueue_Frame( &TransferDesc, 0, HeaderMode );
}


/****************************************************************/
/* Add_Rx_Frame()     	   	  				              	    */
/* Purpose: Add RX handler to the transmitting queue			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static FRAME_ENQUEUE_STATUS Add_Rx_Frame(uint16_t DataSize, int8_t buffer_index)
{
	FRAME_ENQUEUE_STATUS Status;
	TRANSFER_DESCRIPTOR TransferDesc;

	/* Load queue position */
	TransferDesc.DataPtr = NULL; /* Just for the allocate function to know that it is free */
	if( Internal_malloc( &TransferDesc, DataSize ) != NULL )
	{
		TransferDesc.DataSize = DataSize;
		TransferDesc.CallBackMode = CALL_BACK_AFTER_TRANSFER;
		TransferDesc.CallBack = (TransferCallBack)( &Slave_Read_Complete_CallBack );

		/* Read calls are triggered by IRQ pin. */
		Status = Enqueue_Frame( &TransferDesc, buffer_index, SPI_READ );

		if( Status.EnqueuedAtIndex < 0 )
		{
			/* It was not possible to enqueue the frame, so dispose allocated memory  */
			Internal_free( &TransferDesc );
		}
	}else
	{
		Status.EnqueuedAtIndex = -1; /* Memory could'nt be allocated */
		Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;

		Bluenrg_Error( NO_MEMORY_AVAILABLE );
	}

	return ( Status );
}


/****************************************************************/
/* Receiver_Multiplexer()     	  	 	                	    */
/* Purpose: Check the right function to be called based on 		*/
/* received	frame												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Receiver_Multiplexer(uint8_t* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	//static uint8_t FunctionBusy = FALSE;
	//uint8_t CallerGoAhead;

	//TODO: implementar
	//EnterCritical();
	//ExitCritical();

	if( Status != TRANSFER_DONE )
	{
		Bluenrg_Error( RECEPTION_ERROR );
	}else
	{
		HCI_Receive( DataPtr, DataSize );
	}

	return (RELEASE_SPI);
}


/****************************************************************/
/* Internal_malloc()     	  		 	                	    */
/* Purpose: Allocates memory for read and write operations 		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t* Internal_malloc(TRANSFER_DESCRIPTOR* TransferDescPtr, uint16_t DataSize)
{
	/* Function used whenever operations need memory. If local buffer is not being used,
	 * return it, otherwise, request from external sources */

	/* First make sure the memory is freed */
	Internal_free( TransferDescPtr );

	if( ( !LocalMemoryUsed ) && ( DataSize <= SIZE_OF_LOCAL_MEMORY_BUFFER ) )
	{
		LocalMemoryUsed = TRUE;
		TransferDescPtr->DataPtr = &(LocalBytes[0]);
	}else
	{
		TransferDescPtr->DataPtr = Bluenrg_malloc_Request(DataSize);
	}

	return ( TransferDescPtr->DataPtr );
}


/****************************************************************/
/* Bluenrg_malloc_Request()     	  		 	           	    */
/* Purpose: Allocates memory for read and write operations 		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) uint8_t* Bluenrg_malloc_Request(uint16_t DataSize)
{
	/* This function can be implemented externally also to have
	 * control over allocation */
	return ( malloc( DataSize ) );
}


/****************************************************************/
/* Internal_free()     	  		 	               		 	    */
/* Purpose: Deallocates memory for read and write operations 	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Internal_free(TRANSFER_DESCRIPTOR* TransferDescPtr)
{
	uint8_t* DataPtr = TransferDescPtr->DataPtr;

	if( DataPtr != NULL ) /* There is memory allocated */
	{
		if( DataPtr == ( &(LocalBytes[0]) ) )
		{
			/* It is local allocation */
			LocalMemoryUsed = FALSE;
		}else
		{
			/* It is external allocation */
			Bluenrg_free_Request(DataPtr);
		}

		TransferDescPtr->DataPtr = NULL;
	}
}


/****************************************************************/
/* Bluenrg_free_Request()     	  		 		       	    	*/
/* Purpose: Deallocates memory for read and write operations 	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void Bluenrg_free_Request(uint8_t* DataPtr)
{
	/* This function can be implemented externally also to have
	 * control over deallocation */
	free( DataPtr );
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
