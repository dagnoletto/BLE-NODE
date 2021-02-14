

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


typedef struct
{
	uint8_t AllowedWriteSize;
	uint8_t SizeToRead;
	uint8_t NumberOfFilledBuffers;
	BUFFER_POSITION_DESC* BufferHead; /* The head always points to the buffer to be transmitted first */
	BUFFER_POSITION_DESC* BufferTail; /* The tail always points to a free buffer */
	TRANSFER_DESCRIPTOR TXDesc[SIZE_OF_TX_FRAME_BUFFER];
	BUFFER_POSITION_DESC TXBuffer[SIZE_OF_TX_FRAME_BUFFER];
}BUFFER_MANAGEMENT;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t BluenrgMS_Slave_Header_CallBack(SPI_SLAVE_HEADER* Header, TRANSFER_STATUS Status);
static uint8_t Request_Slave_Header(void);
static void BluenrgMS_Init_Buffer_Manager(void);
static BUFFER_POSITION_DESC* BluenrgMS_Search_For_Free_Buffer(void);
static void Request_BluenrgMS_Frame_Transmission(void);
static TRANSFER_DESCRIPTOR* BluenrgMS_Get_Frame_Buffer_Head(void);
static void BluenrgMS_Release_Frame_Buffer(void);


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


/****************************************************************/
/* Reset_BluenrgMS()                                            */
/* Purpose: Hardware reset for Bluenrg-MS module 	    		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Reset_BluenrgMS(void)
{
	uint32_t TimerResetActive = 0;

	BluenrgMS_Init_Buffer_Manager();

	Clr_BluenrgMS_Reset_Pin(); /* Put device in hardware reset state */

	/* The longest value of TResetActive is 94 ms */
	while( !TimeBase_DelayMs( &TimerResetActive, 100UL, FALSE) );

	Set_BluenrgMS_Reset_Pin(); /* Put device in running mode */
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
	BufferManager.BufferHead->Prev = &BufferManager.TXBuffer[0]; /* Points to the last buffer */

	BufferManager.BufferTail = &BufferManager.TXBuffer[0]; /* The tail always points to last filled buffer */
	BufferManager.BufferTail->Next = &BufferManager.TXBuffer[0]; /* Points to the next free buffer */
	BufferManager.BufferTail->Prev = &BufferManager.TXBuffer[0]; /* Points to the last buffer */

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
	/* TODO: first test if there is an ongoing operation */
	if( /*!transfer_Running */ !0)
	{

		if( Request_Slave_Header() == FALSE )
		{
			///TODO: signals the system there is an SPI_IRQ request pending or verify after at Other interrupts the pin
		}

	}else
	{
		/* ///TODO signals the data read back during the last operation might be use full, check after tx complete */
		// Apesar que ser for um comando de escrita o bluenrg n�o vai "descarregar os dados" e sim enviar um lixo no
		//spi de retorno
	}
}


/****************************************************************/
/* BluenrgMS_Get_Frame_Buffer_Head()                            */
/* Purpose: Get the pointer of the head of transfer queue.	 	*/
/* This is the transfer/buffer pointer that is going to be sent	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static TRANSFER_DESCRIPTOR* BluenrgMS_Get_Frame_Buffer_Head(void)
{
	if( BufferManager.BufferHead->Status != BUFFER_FREE )
	{
		BufferManager.BufferHead->Status = BUFFER_TRANSMITTING;

		return ( BufferManager.BufferHead->TransferDescPtr );
	}
	return ( NULL );
}


/****************************************************************/
/* BluenrgMS_Release_Frame_Buffer()                             */
/* Purpose: Release the frame head.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void BluenrgMS_Release_Frame_Buffer(void)
{
	if( BufferManager.BufferHead->Status == BUFFER_TRANSMITTING )
	{
		BUFFER_POSITION_DESC* NewHead;

		BufferManager.BufferHead->Status = BUFFER_FREE;
		BufferManager.NumberOfFilledBuffers--;

		if( BufferManager.BufferTail->Next == NULL )
		{
			BufferManager.BufferTail->Next = BufferManager.BufferHead;
		}

		NewHead = BufferManager.BufferHead->Next;

		BufferManager.BufferHead = NewHead;
	}
}


/****************************************************************/
/* BluenrgMS_Enqueue_Frame()            	                    */
/* Purpose: Enqueue the new frame for transmission in the  		*/
/* output buffer												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t BluenrgMS_Enqueue_Frame(TRANSFER_DESCRIPTOR* TransferDescPtr, int8_t buffer_index)
{
	/* TODO: THIS PROCEDURE CANNOT BE INTERRUPTED BY SPI INTERRUPTS */
	/* BufferTail always represents the next available free buffer. If NULL, means
	 * all buffers are busy and transfer cannot be scheduled */

	BUFFER_POSITION_DESC* BufferPtr = BufferManager.BufferTail->Next;

	if( BufferPtr != NULL )
	{
		if( BufferPtr->Status == BUFFER_FREE ) /* Is, indeed, free */
		{

			*( BufferPtr->TransferDescPtr ) = *TransferDescPtr; /* Occupy this buffer */
			BufferPtr->Status = BUFFER_FULL;
			BufferManager.NumberOfFilledBuffers++;

			/* Search for next free buffer */
			BufferPtr->Next = BluenrgMS_Search_For_Free_Buffer();
			BufferPtr->Prev = BufferManager.BufferTail;
			BufferManager.BufferTail = BufferPtr;

			if( BufferManager.NumberOfFilledBuffers > 1 ) /* When more that one buffer is loaded, it is possible to reorder */
			{
				BUFFER_POSITION_DESC* ParentBuffer = BufferManager.BufferHead;
				BUFFER_POSITION_DESC* DesiredBuffer = BufferManager.BufferHead;

				if( buffer_index < 0 ){ buffer_index = 0; }
				else if( buffer_index > (SIZE_OF_TX_FRAME_BUFFER - 1) ){ buffer_index = (SIZE_OF_TX_FRAME_BUFFER - 1); }

				if( ( buffer_index == 0 ) && ( BufferManager.BufferHead->Status == BUFFER_TRANSMITTING ) )
				{
					/* Cannot interfere with the head being transmitted */
					buffer_index = 1;
				}

				/* There exists positions filled in the buffer, trace the path */
				int8_t filled_index;
				for ( filled_index = 0; filled_index < SIZE_OF_TX_FRAME_BUFFER; filled_index++ )
				{
					if( filled_index == buffer_index )
					{
						DesiredBuffer = ParentBuffer;
					}
					if( ParentBuffer->Next != BufferManager.BufferTail )
					{
						ParentBuffer = ParentBuffer->Next;
					}else
					{
						break;
					}
				}

//				if( buffer_index <= filled_index )
//				{
//
//					//TODO: CORRIGIR ISSO, N�O FUNCIONA EM TODAS AS SITUA��ES
//
//					/* It only makes sense to reorder buffers if desired position is lower or equal the actual buffer usage */
//					/* The parent buffer do not point to Tail anymore, but to the "Next" Tail: */
//					ParentBuffer->Next = BufferManager.BufferTail->Next;
//
//					/* The "Next" and "Prev" for Tail is not the Free buffer, but it is the position it is taking over */
//					BufferManager.BufferTail->Next = DesiredBuffer;
//
//					if( DesiredBuffer->Prev != DesiredBuffer )
//					{
//						BufferManager.BufferTail->Prev = DesiredBuffer->Prev;
//					}else
//					{
//						BufferManager.BufferTail->Prev = BufferManager.BufferTail;
//					}
//
//					/* The Desire buffer "Prev" now should point to the one that comes before it  */
//					DesiredBuffer->Prev = BufferManager.BufferTail;
//
//					if( BufferManager.BufferHead == DesiredBuffer )
//					{
//						BufferManager.BufferHead = BufferManager.BufferTail;
//					}
//
//					BufferManager.BufferTail = ParentBuffer;
//				}

			}else
			{
				Request_BluenrgMS_Frame_Transmission();
			}

			return (TRUE);

		}
	}

	return ( FALSE ); /* Could no enqueue the transfer */
}


/****************************************************************/
/* Request_BluenrgMS_Frame_Transmission()            	        */
/* Purpose: Send SPI message to BluenrgMS	    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Request_BluenrgMS_Frame_Transmission(void)
{
	TRANSFER_DESCRIPTOR* TransferDescPtr = BluenrgMS_Get_Frame_Buffer_Head();

	if( TransferDescPtr != NULL )
	{
		if( Send_BluenrgMS_SPI_Frame( TransferDescPtr->TxPtr, TransferDescPtr->RxPtr, TransferDescPtr->DataSize ) == FALSE )
		{
			Release_BluenrgMS_SPI();
		}
	}
}


/****************************************************************/
/* BluenrgMS_SPI_Frame_Transfer_Status()         	     		*/
/* Purpose: SPI transfer completed in hardware	    		   	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void BluenrgMS_SPI_Frame_Transfer_Status(TRANSFER_STATUS status)
{
	SPI_RELEASE ReleaseSPI = RELEASE_SPI;
	TRANSFER_DESCRIPTOR* TransferDescPtr = BluenrgMS_Get_Frame_Buffer_Head();

	if( TransferDescPtr != NULL && status == TRANSFER_DONE )
	{
		if( TransferDescPtr->CallBack != NULL )
		{
			if( TransferDescPtr->CallBackMode == CALL_BACK_AFTER_TRANSFER )
			{
				ReleaseSPI = TransferDescPtr->CallBack( TransferDescPtr->RxPtr, TRANSFER_DONE );
			}else
			{
				//TODO: put in the callback queue
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
	//TODO: quem sabe pode ser melhorado para encontrar buffer livre mais r�pido
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
static uint8_t BluenrgMS_Slave_Header_CallBack(SPI_SLAVE_HEADER* Header, TRANSFER_STATUS Status)
{
	//TODO: fazer a leitura consecutiva neste ponto
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
			//TODO: if there are still commands to process, make a header read to know when device has allowed more data transfer
			//if (has commands on buffer)
			/* Enqueue a new slave header read to check when device allows more data */
			Request_Slave_Header();
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
uint8_t Request_Slave_Header(void)
{
	TRANSFER_DESCRIPTOR TransferDesc;

	/* Load transmitting queue position */
	TransferDesc.TxPtr = (uint8_t*)( &SPIMasterHeaderRead.CTRL );
	TransferDesc.RxPtr = &SPISlaveHeader.READY;
	TransferDesc.DataSize = sizeof(SPISlaveHeader); /* Put always the size of reading buffer to avoid writing in random memory */
	TransferDesc.CallBackMode = CALL_BACK_AFTER_TRANSFER;
	TransferDesc.CallBack = (TransferCallBack)( &BluenrgMS_Slave_Header_CallBack );

	/* We need to know if we have to read or how much we can write, so put the command in the first position of the queue */
	return ( BluenrgMS_Enqueue_Frame( &TransferDesc, 1 ) );

}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
