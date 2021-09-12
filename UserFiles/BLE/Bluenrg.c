

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
static uint8_t Slave_Header_CallBack(TRANSFER_DESCRIPTOR* TransferDescPtr, SPI_TRANSFER_MODE HeaderMode, TRANSFER_STATUS Status);
static uint8_t Transmitter_Multiplexer(TRANSFER_DESCRIPTOR* TransferDescPtr, TRANSFER_STATUS Status);
static uint8_t Receiver_Multiplexer(TRANSFER_DESCRIPTOR* TransferDescPtr, TRANSFER_STATUS Status);
static uint8_t Request_Slave_Header(SPI_TRANSFER_MODE HeaderMode, uint8_t Priority);
static void Init_Buffer_Manager(void);
static void Init_CallBack_Manager(CALLBACK_MANAGEMENT* ManagerPtr);
static FRAME_ENQUEUE_STATUS Enqueue_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index, SPI_TRANSFER_MODE TransferMode, uint8_t Priority);
inline static BUFFER_DESC* Search_For_Free_Frame(void) __attribute__((always_inline));
inline static void Release_Frame(void) __attribute__((always_inline));
static uint8_t Enqueue_CallBack(BUFFER_DESC* Buffer, CALLBACK_MANAGEMENT* ManagerPtr);
inline static CALLBACK_DESC* Search_For_Free_CallBack(CALLBACK_MANAGEMENT* ManagerPtr) __attribute__((always_inline));
inline static void Release_CallBack(CALLBACK_MANAGEMENT* ManagerPtr) __attribute__((always_inline));
static FRAME_ENQUEUE_STATUS Add_Rx_Frame(uint16_t DataSize, int8_t buffer_index);
static uint8_t* Internal_malloc(TRANSFER_DESCRIPTOR* TransferDescPtr, uint16_t DataSize);
static void Internal_free(TRANSFER_DESCRIPTOR* TransferDescPtr);
inline static void Safe_Enqueue_CallBack( TRANSFER_DESCRIPTOR* TransferDescPtr, CALLBACK_MANAGEMENT* ManagerPtr ) __attribute__((always_inline));;
static void Process_CallBack(CALLBACK_MANAGEMENT* ManagerPtr, SPI_TRANSFER_MODE TransferMode);


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
static CALLBACK_MANAGEMENT ReadCallBackManager;
static CALLBACK_MANAGEMENT WriteCallBackManager;
static uint8_t LocalBytes[SIZE_OF_LOCAL_MEMORY_BUFFER];
static uint8_t DummyByte;
static uint32_t TimerResetActive = 0;
static uint8_t ResetBluenrgRequest = TRUE;
static uint8_t FirstBluenrgReset = TRUE;
static uint8_t LocalMemoryUsed = FALSE;
static volatile uint8_t BlockFrameHead = 0;
static volatile uint8_t BlockRequestFrame = 0;
static volatile uint8_t FrameHeadRelease = 0;
static volatile uint8_t FrameHeadReleaseRequest = 0;
static volatile uint8_t FrameEnqueueSignal = 0;


/****************************************************************/
/* Reset_Bluenrg()                                        	    */
/* Purpose: Hardware reset for Bluenrg module 	    			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Reset_Bluenrg(void)
{
	/* Abort any ongoing transmission */
	/* This function MUST NOT trigger any interrupt or procedure
	 *  that would call Bluenrg_Frame_Status */
	Bluenrg_Abort_Transfer();

	Release_Bluenrg();

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
		Process_CallBack( &ReadCallBackManager, SPI_READ );
		Process_CallBack( &WriteCallBackManager, SPI_WRITE );

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
/* Process_CallBack()                        		            */
/* Purpose: Dequeue and process callbacks				  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Process_CallBack(CALLBACK_MANAGEMENT* ManagerPtr, SPI_TRANSFER_MODE TransferMode)
{
	int8_t NumberOfCallbacksPerCall = SIZE_OF_CALLBACK_BUFFER/2; /* To avoid being blocked by too much callbacks to process */

	TRANSFER_DESCRIPTOR* TransferDescPtr;
	TRANSFER_STATUS Status;

	while( ( ManagerPtr->CallBackHead->Status == BUFFER_FULL ) && ( NumberOfCallbacksPerCall > 0 ) )
	{
		TransferDescPtr = &ManagerPtr->CallBackHead->TransferDesc;
		Status = ManagerPtr->CallBackHead->TransferStatus;

		/* Blocks until the resource is acquired */
		if( TransferMode == SPI_READ )
		{
			while( Receiver_Multiplexer( TransferDescPtr, Status ) != TRUE );
		}else
		{
			while( Transmitter_Multiplexer( TransferDescPtr, Status ) != TRUE );
		}

		Internal_free( TransferDescPtr );

		Release_CallBack( ManagerPtr );

		NumberOfCallbacksPerCall--;
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

	Init_CallBack_Manager( &ReadCallBackManager );
	Init_CallBack_Manager( &WriteCallBackManager );

	FirstBluenrgReset = FALSE;
}


/****************************************************************/
/* Init_CallBack_Manager()                                    	*/
/* Purpose: Initialize callback manager    						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Init_CallBack_Manager(CALLBACK_MANAGEMENT* ManagerPtr)
{
	/* This strange procedure is used to avoid losing context in the cases where the Bluenrg is issued while the system is serving a callback */
	int8_t indexbuffhead = 0;
	CALLBACK_DESC* FreeBuffer;

	for( int8_t i = 0; i < SIZE_OF_CALLBACK_BUFFER; i++ )
	{
		if( FirstBluenrgReset )
		{
			ManagerPtr->CallBack[i].Status = BUFFER_FREE; /* Buffer is free */
			ManagerPtr->CallBack[i].TransferDesc.DataPtr = NULL;
		}else if( &ManagerPtr->CallBack[i] == ManagerPtr->CallBackHead )
		{
			if( ManagerPtr->CallBackHead->Status == BUFFER_FULL ) /* The buffer is being or will be handled */
			{
				indexbuffhead = i;
			}else
			{
				ManagerPtr->CallBack[i].Status = BUFFER_FREE; /* Buffer is free */
				Internal_free( &ManagerPtr->CallBack[i].TransferDesc );
			}
		}else
		{
			ManagerPtr->CallBack[i].Status = BUFFER_FREE; /* Buffer is free */
			Internal_free( &ManagerPtr->CallBack[i].TransferDesc );
		}
	}

	ManagerPtr->CallBackHead = &ManagerPtr->CallBack[indexbuffhead]; /* The head always points to the callback to be handled first */

	if( ManagerPtr->CallBackHead->Status == BUFFER_FREE )
	{
		FreeBuffer = &ManagerPtr->CallBack[indexbuffhead];
		ManagerPtr->NumberOfFilledBuffers = 0;
	}else
	{
		FreeBuffer = Search_For_Free_CallBack( ManagerPtr );
		ManagerPtr->NumberOfFilledBuffers = 1;
	}

	ManagerPtr->CallBackHead->Next = FreeBuffer; /* Points to the next callback for handling */

	ManagerPtr->CallBackTail = &ManagerPtr->CallBack[indexbuffhead]; /* The tail always points to last filled callback */

	ManagerPtr->CallBackTail->Next = FreeBuffer; /* Points to the next free callback */
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
	EnterCritical(); /* Critical section enter */

	FrameHeadRelease++;

	if( FrameHeadRelease != 1 )
	{
		ExitCritical(); /* Critical section exit */
		return;
	}

	if( FrameEnqueueSignal ) /* Cannot release under frame enqueue */
	{
		FrameHeadReleaseRequest++;
		FrameHeadRelease = 0;
		ExitCritical(); /* Critical section exit */
		return;
	}

	if( BlockFrameHead ) /* Cannot release the head that is blocked for transmission */
	{
		FrameHeadRelease = 0;
		ExitCritical(); /* Critical section exit */
		return;
	}

	ExitCritical(); /* Critical section exit */


	BufferManager.BufferHead->Status = BUFFER_FREE;

	if( BufferManager.NumberOfFilledBuffers != 0 )
	{
		BufferManager.NumberOfFilledBuffers--;
	}

	if( BufferManager.BufferTail->Next == NULL )
	{
		BufferManager.BufferTail->Next = BufferManager.BufferHead;
	}

	BufferManager.BufferHead = BufferManager.BufferHead->Next;


	EnterCritical(); /* Critical section enter */

	FrameHeadRelease = 0;
	FrameHeadReleaseRequest = 0;

	ExitCritical(); /* Critical section exit */
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
		Status = Enqueue_Frame(TransferDescPtr, buffer_index, SPI_WRITE, 1);

		if( Status.EnqueuedAtIndex < 0 )
		{
			/* It was not possible to enqueue the frame, so dispose allocated memory  */
			Internal_free( TransferDescPtr );
		}
	}else
	{
		Status.EnqueuedAtIndex = -1; /* Memory could'nt be allocated */
		Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;
		Status.RequestTransmission = FALSE;

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
static void Release_CallBack(CALLBACK_MANAGEMENT* ManagerPtr)
{
	ManagerPtr->CallBackHead->Status = BUFFER_FREE;

	if( ManagerPtr->NumberOfFilledBuffers != 0 )
	{
		ManagerPtr->NumberOfFilledBuffers--;
	}

	if( ManagerPtr->CallBackTail->Next == NULL )
	{
		ManagerPtr->CallBackTail->Next = ManagerPtr->CallBackHead;
	}

	ManagerPtr->CallBackHead = ManagerPtr->CallBackHead->Next;
}


/****************************************************************/
/* Enqueue_CallBack()            	   			                */
/* Purpose: Enqueue a new callback for asynchronous handling	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Enqueue_CallBack(BUFFER_DESC* Buffer, CALLBACK_MANAGEMENT* ManagerPtr)
{
	/* CallBackTail always represents the last filled callback.
	 * CallBackHead always represents the first filled callback.
	 * CallBackTail->Next points to a free callback, otherwise is NULL if free callback could not be found */
	uint8_t Status = FALSE;

	/* Check if the next callback after tail is available */
	CALLBACK_DESC* CallBackPtr = ManagerPtr->CallBackTail->Next;

	if( CallBackPtr != NULL )
	{
		if( CallBackPtr->Status == BUFFER_FREE ) /* Check if buffer is free */
		{

			CallBackPtr->TransferDesc = Buffer->TransferDesc; /* Occupy this buffer */
			CallBackPtr->Status = BUFFER_FULL;
			CallBackPtr->Next = Search_For_Free_CallBack( ManagerPtr );

			CallBackPtr->TransferStatus = Buffer->TransferStatus;

			ManagerPtr->NumberOfFilledBuffers++;

			ManagerPtr->CallBackTail = CallBackPtr;

			Status = TRUE;
		}
	}

	return (Status);
}


/****************************************************************/
/* Search_For_Free_CallBack()           		                */
/* Purpose: Find free callback or return NULL					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static CALLBACK_DESC* Search_For_Free_CallBack(CALLBACK_MANAGEMENT* ManagerPtr)
{
	/* Search for an available free callback. This function is not optimized
	 * in the sense it could start from a speculative index based on current
	 * callback usage to more quickly find a buffer. */
	for ( int8_t i = 0; i < SIZE_OF_CALLBACK_BUFFER; i++ )
	{
		if( ManagerPtr->CallBack[i].Status == BUFFER_FREE )
		{
			return ( &ManagerPtr->CallBack[i] );
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
static FRAME_ENQUEUE_STATUS Enqueue_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index, SPI_TRANSFER_MODE TransferMode, uint8_t Priority)
{
	/* BufferTail always represents the last filled buffer.
	 * BufferHead always represents the first filled buffer.
	 * BufferTail->Next points to a free buffer, otherwise is NULL if free buffers could not be found */
	static volatile uint8_t Acquire = 0;

	FRAME_ENQUEUE_STATUS Status;

	Status.EnqueuedAtIndex = -1; /* Could not enqueue the frame */
	Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;
	Status.RequestTransmission = FALSE;

	EnterCritical(); /* Critical section enter */

	Acquire++;

	if( Acquire != 1 ) /* Cannot enqueue while a frame is being released */
	{
		ExitCritical(); /* Critical section exit */
		return (Status); /* This function is already being handled and we are not going to mix it up */
	}

	if( FrameHeadRelease )
	{
		Acquire = 0;
		ExitCritical(); /* Critical section exit */
		return (Status); /* This function is already being handled and we are not going to mix it up */
	}

	ExitCritical();


	/* Check if the next buffer after tail is available */
	BUFFER_DESC* BufferPtr = BufferManager.BufferTail->Next;

	if( BufferPtr != NULL )
	{
		if( BufferPtr->Status == BUFFER_FREE ) /* Check if buffer is free */
		{

			EnterCritical(); /* A new frame will be enqueued, safely signal that to the rest of software. */

			FrameEnqueueSignal++;

			ExitCritical();

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
						volatile uint8_t FrameHeadBlocked;

						/* Critical: check if Request Frame is not being handled. */
						EnterCritical();

						if( Priority == 0 ) /* Depending the priority of whom call the function, it can bypass BlockFrameHead. */
						{
							FrameHeadBlocked = 0;
						}else
						{
							FrameHeadBlocked = BlockFrameHead;
						}

						ExitCritical();

						if( ( BufferManager.BufferHead->Status == BUFFER_TRANSMITTING ) || ( FrameHeadBlocked != 0 ) )
						{
							/* Cannot be the head since a transfer/releasing is ongoing */
							buffer_index = 1;
						}else if( ( BufferManager.BufferHead->Status == BUFFER_PAUSED ) &&
								( BufferPtr->TransferMode != SPI_HEADER_READ ) && ( BufferPtr->TransferMode != SPI_HEADER_WRITE ) )
						{
							/* The head is paused but the candidate transfer is not a header read/write to take over priority */
							buffer_index = 1;
						}

						/* If a message is going to be enqueued in the fist position, avoid request frame transmission since the
						 * buffer index 0 always refers to the buffer head and this one is sent first.  */
						if( buffer_index == 0 )
						{
							EnterCritical();

							BlockRequestFrame++; /* Safely signals buffer head must not be transmitted since is going to be updated now. */

							ExitCritical();
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

			EnterCritical();

			FrameEnqueueSignal = 0;

			if( FrameHeadReleaseRequest )
			{
				ExitCritical();
				Release_Frame();
				Status.RequestTransmission = TRUE;
			}
			EnterCritical();

			BlockRequestFrame = 0;
			Acquire = 0;

			ExitCritical();

			return (Status);

		}
	}

	Status.EnqueuedAtIndex = -1; /* Could not enqueue the frame */
	Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;

	Bluenrg_Error( QUEUE_IS_FULL );


	EnterCritical();

	BlockRequestFrame = 0;
	Acquire = 0;
	FrameEnqueueSignal = 0;

	ExitCritical();

	return ( Status ); /* Could not enqueue the transfer */
}


/****************************************************************/
/* Request_Frame()            	        						*/
/* Purpose: Request buffer transmission.	    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Request_Frame(void)
{
	static volatile uint8_t Acquire = 0;

	EnterCritical(); /* Critical section enter */

	Acquire++;

	if( Acquire != 1 )
	{
		ExitCritical(); /* Critical section exit */
		/* The driver can only send a message at a time, so, while the function is being
		 * handled, no other source can call it to push a new frame */
		return; /* This function is already being handled and we are not going to mix it up */
	}

	/* The Enqueue function blocked this function because it is putting a new frame onto the head. */
	if( BlockRequestFrame )
	{
		Acquire = 0;
		ExitCritical();
		return;
	}

	BlockFrameHead++; /* Blocks the frame head to avoid changes by the Enqueue function */

	ExitCritical(); /* Critical section exit */

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

					uint8_t Result = Request_Slave_Header( SPI_HEADER_WRITE, 0 );

					if( Result != TRUE )
					{
						EnterCritical(); /* Critical section enter */

						BlockFrameHead = 0;
						Acquire = 0;
						BufferManager.BufferHead->Status = BUFFER_FULL; /* Back to previous state */

						ExitCritical(); /* Critical section exit */
						return;
					}

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

					EnterCritical(); /* Critical section enter */

					BlockFrameHead = 0;
					Acquire = 0;

					ExitCritical(); /* Critical section exit */
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

				EnterCritical(); /* Critical section enter */

				BlockFrameHead = 0;
				Acquire = 0;

				ExitCritical(); /* Critical section exit */
				return;
				break;
			}

			BufferManager.BufferHead->Status = BUFFER_TRANSMITTING;

			if( Bluenrg_Send_Frame( BufferManager.BufferHead->TransferMode, TXDataPtr,
					BufferManager.BufferHead->RxPtr, DataSize ) == FALSE )
			{
				Release_Bluenrg();
			}
		}else if( ( BufferManager.BufferHead->Status == BUFFER_FREE ) && ( Get_Bluenrg_IRQ_Pin() ) ) /* Check if the IRQ pin is set */
		{
			/* Enqueue a new slave header read to check if device is ready */
			if( Request_Slave_Header( SPI_HEADER_READ, 1 ) )
			{
				goto CheckBufferHead; /* I know, I know, ugly enough. But think of code savings and performance, OK? */
			}
		}
	}

	EnterCritical(); /* Critical section enter */

	BlockFrameHead = 0;
	Acquire = 0;

	ExitCritical(); /* Critical section exit */
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
				if( TransferDescPtr->CallBackMode == CALL_BACK_AFTER_TRANSFER )
				{
					if( ( BufferManager.BufferHead->TransferMode == SPI_HEADER_READ ) || ( BufferManager.BufferHead->TransferMode == SPI_HEADER_WRITE ) )
					{
						ReleaseSPI = Slave_Header_CallBack( TransferDescPtr, BufferManager.BufferHead->TransferMode, BufferManager.BufferHead->TransferStatus );

						/* The SPI_HEADER_READ must not be deallocated since its memory is not dynamic. Just "lies" that memory is freed. */
						TransferDescPtr->DataPtr = NULL;

					}else if( BufferManager.BufferHead->TransferMode == SPI_WRITE )
					{
						if( Transmitter_Multiplexer( TransferDescPtr, BufferManager.BufferHead->TransferStatus ) == TRUE )
						{
							Internal_free( TransferDescPtr );
						}else
						{
							/* The handler is blocked by some other process. Put the callback in the queue to be processed later. */
							Safe_Enqueue_CallBack( TransferDescPtr, &WriteCallBackManager );
						}
					}else /* SPI_READ */
					{
						/* Call application to check what to do with the data */
						Bluenrg_CallBack_Config( &(TransferDescPtr->CallBackMode), (HCI_PACKET_TYPE)(*TransferDescPtr->DataPtr), (TransferDescPtr->DataPtr + 1) );

						/* The SPI read enqueues the multiplexer function */
						if( TransferDescPtr->CallBackMode == CALL_BACK_AFTER_TRANSFER )
						{
							if( Receiver_Multiplexer( TransferDescPtr, BufferManager.BufferHead->TransferStatus ) == TRUE )
							{
								Internal_free( TransferDescPtr );
							}else
							{
								Safe_Enqueue_CallBack( TransferDescPtr, &ReadCallBackManager );
							}
						}else
						{
							Safe_Enqueue_CallBack( TransferDescPtr, &ReadCallBackManager );
						}
					}
				}else
				{
					CALLBACK_MANAGEMENT* ManagerPtr;

					if( BufferManager.BufferHead->TransferMode == SPI_WRITE )
					{
						ManagerPtr = &WriteCallBackManager;
					}else
					{
						ManagerPtr = &ReadCallBackManager;
					}

					Safe_Enqueue_CallBack( TransferDescPtr, ManagerPtr );
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
/* Safe_Enqueue_CallBack()                  		         	*/
/* Purpose:	Enqueue the callback and free the memory if not.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Safe_Enqueue_CallBack( TRANSFER_DESCRIPTOR* TransferDescPtr, CALLBACK_MANAGEMENT* ManagerPtr )
{
	/* The multiplex function will be called asynchronously */
	/* Put in the callback queue: the transfer and data must be copied in order to release the transfer buffer */
	static volatile uint8_t Acquire = 0;

	EnterCritical(); /* Critical section enter */

	Acquire++;

	if( Acquire != 1 )
	{
		ExitCritical(); /* Critical section exit */
		return;
	}

	ExitCritical(); /* Critical section exit */

	if( Enqueue_CallBack( BufferManager.BufferHead, ManagerPtr ) != TRUE )
	{
		Internal_free( TransferDescPtr );
		Bluenrg_Error( QUEUE_IS_FULL );
	}
	/* This is not a real deallocation, because the callback queue might still be using the pointer. It is just to free the buffer for new allocation */
	TransferDescPtr->DataPtr = NULL;

	EnterCritical(); /* Critical section enter */

	Acquire = 0;

	ExitCritical(); /* Critical section exit */
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
static uint8_t Slave_Header_CallBack(TRANSFER_DESCRIPTOR* TransferDescPtr, SPI_TRANSFER_MODE HeaderMode, TRANSFER_STATUS Status)
{
	uint8_t static DeviceNotReadyCounter = 0;
	uint8_t static NoAllowedWriteCounter = 0;
	uint8_t static ErroneousResponseCounter = 0;

	SPI_SLAVE_HEADER* Header = (SPI_SLAVE_HEADER*)( TransferDescPtr->DataPtr );

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
static uint8_t Request_Slave_Header(SPI_TRANSFER_MODE HeaderMode, uint8_t Priority)
{
	TRANSFER_DESCRIPTOR TransferDesc;

	/* Load transmitting queue position */
	TransferDesc.DataPtr = &SPISlaveHeader.READY;
	TransferDesc.DataSize = sizeof(SPISlaveHeader); /* Put always the size of reading buffer to avoid writing in random memory */
	TransferDesc.CallBackMode = CALL_BACK_AFTER_TRANSFER;
	/* The callback function MUST be configured, although not used in this case */
	TransferDesc.CallBack = (TransferCallBack)(&Slave_Header_CallBack);

	/* We need to know if we have to read or how much we can write, so put the command in the first position of the queue */
	/* Acho que não deve empilhar em posição diferente de zero se for um request para slave */
	if( Enqueue_Frame( &TransferDesc, 0, HeaderMode, Priority ).EnqueuedAtIndex != 0 )
	{
		return (FALSE);
	}

	return (TRUE);
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

		/* The callback function MUST be configured, although not used in this case */
		TransferDesc.CallBack = (TransferCallBack)(&Bluenrg_CallBack_Config);

		/* Read calls are triggered by IRQ pin. */
		Status = Enqueue_Frame( &TransferDesc, buffer_index, SPI_READ, 1 );

		if( Status.EnqueuedAtIndex < 0 )
		{
			/* It was not possible to enqueue the frame, so dispose allocated memory  */
			Internal_free( &TransferDesc );
		}
	}else
	{
		Status.EnqueuedAtIndex = -1; /* Memory could'nt be allocated */
		Status.NumberOfEnqueuedFrames = BufferManager.NumberOfFilledBuffers;
		Status.RequestTransmission = FALSE;

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
static uint8_t Receiver_Multiplexer(TRANSFER_DESCRIPTOR* TransferDescPtr, TRANSFER_STATUS Status)
{
	static volatile uint8_t Acquire = 0;

	EnterCritical(); /* Critical section enter */

	Acquire++;

	if( Acquire != 1 )
	{
		ExitCritical(); /* Critical section exit */
		return (FALSE);
	}

	ExitCritical(); /* Critical section exit */

	HCI_Receive( TransferDescPtr->DataPtr, TransferDescPtr->DataSize, Status );

	EnterCritical(); /* Critical section enter */

	Acquire = 0;

	ExitCritical(); /* Critical section exit */

	return (TRUE);
}


/****************************************************************/
/* Transmitter_Multiplexer()     	  		               	    */
/* Purpose: Check the right function to be called based on 		*/
/* transmitted frame											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Transmitter_Multiplexer(TRANSFER_DESCRIPTOR* TransferDescPtr, TRANSFER_STATUS Status)
{
	static volatile uint8_t Acquire = 0;

	EnterCritical(); /* Critical section enter */

	Acquire++;

	if( Acquire != 1 )
	{
		ExitCritical(); /* Critical section exit */
		return (FALSE);
	}

	ExitCritical(); /* Critical section exit */

	TransferDescPtr->CallBack( TransferDescPtr->DataPtr, TransferDescPtr->DataSize, Status );

	EnterCritical(); /* Critical section enter */

	Acquire = 0;

	ExitCritical(); /* Critical section exit */

	return (TRUE);

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
/* Bluenrg_Get_Max_Transfer_Queue_Size()     	       	    	*/
/* Purpose: Return the transfer queue size					 	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
int8_t Bluenrg_Get_Max_Transfer_Queue_Size(void)
{
	return ( SIZE_OF_FRAME_BUFFER );
}


/****************************************************************/
/* Bluenrg_Get_Max_CallBack_Queue_Size()     	       	    	*/
/* Purpose: Return the callback queue size					 	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
int8_t Bluenrg_Get_Max_CallBack_Queue_Size(void)
{
	return ( SIZE_OF_CALLBACK_BUFFER );
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
