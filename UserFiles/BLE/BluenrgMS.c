

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


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t BluenrgMS_Slave_Header_CallBack(SPI_SLAVE_HEADER* Header, TRANSFER_STATUS Status);
static uint8_t Request_Slave_Header(void);


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
const SPI_MASTER_HEADER SPIMasterHeaderWrite = { .CTRL = CTRL_WRITE, .Dummy = {0,0,0,} };
const SPI_MASTER_HEADER SPIMasterHeaderRead  = { .CTRL = CTRL_READ, .Dummy = {0,0,0,} };
static SPI_SLAVE_HEADER SPISlaveHeader;
static TRANSFER_DESCRIPTOR TXDesc[SIZE_OF_TX_FRAME_BUFFER];
static BUFFER_POSITION_DESC TXBuffer[SIZE_OF_TX_FRAME_BUFFER];
static BUFFER_POSITION_DESC* FreeBuffer;
static BUFFER_POSITION_DESC* LastTransferBuf = NULL;
static uint8_t WriteBufferSize = 0;
static uint8_t ReadBufferSize = 0;


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

	for( int8_t i = 0; i < SIZE_OF_TX_FRAME_BUFFER; i++ )
	{
		TXBuffer[i].TransferDescPtr = &TXDesc[i];
		TXBuffer[i].TxStatus = 0;
	}

	/* TODO: Ler e inserir flags para verificar se a interrupção
	 * deve ser lida ou não */
	Clr_BluenrgMS_Reset_Pin(); /* Put device in hardware reset state */

	/* The longest value of TResetActive is 94 ms */
	while( !TimeBase_DelayMs( &TimerResetActive, 100UL, FALSE) );

	Set_BluenrgMS_Reset_Pin(); /* Put device in running mode */
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
		// Apesar que ser for um comando de escrita o bluenrg não vai "descarregar os dados" e sim enviar um lixo no
		//spi de retorno
	}
}


/****************************************************************/
/* BluenrgMS_Get_Free_Frame_Buffer()                            */
/* Purpose: Get the pointer of the available buffer and its 	*/
/* status in the transmission queue. It always return the last	*/
/* available position. 					  						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BUFFER_POSITION_DESC* BluenrgMS_Get_Free_Frame_Buffer(void)
{
	//TODO: verificar se o buffer está vazio e etc
	return (&TXBuffer[0]);
}


/****************************************************************/
/* BluenrgMS_Get_Frame_Buffer_Head()                            */
/* Purpose: Get the pointer of the head of transfer queue.	 	*/
/* This is the transfer/buffer pointer that is going to be sent	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BUFFER_POSITION_DESC* BluenrgMS_Get_Frame_Buffer_Head(void)
{
	//TODO: verificar se o buffer está habilitado para ser enviado e etc
	//return NULL if no frame is available
	return (&TXBuffer[0]);
}


/****************************************************************/
/* BluenrgMS_Release_Frame_Buffer()                             */
/* Purpose: Release the frame just processed.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void BluenrgMS_Release_Frame_Buffer(BUFFER_POSITION_DESC* BuffPtr)
{
	//TODO: liberar o buffer na lista de transmissão
	BuffPtr->TxStatus = 0;
}


/****************************************************************/
/* BluenrgMS_Enqueue_Frame()            	                    */
/* Purpose: Enqueue the new frame for transmission in the  		*/
/* output buffer												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t BluenrgMS_Enqueue_Frame(BUFFER_POSITION_DESC* Buffer, uint8_t position)
{
	/* TODO: check the position is free, put the request in the desired position if possible
	 * make the buffer reordering. Load the position and only after that
	 * issue Request_BluenrgMS_Frame_Transmission if only one buffer is scheduled
	 * for transmission
	 */

	//if( Request_BluenrgMS_Frame_Transmission(Buffer->TransferDescPtr) )
	//{
	Request_BluenrgMS_Frame_Transmission();
		return (TRUE); /* TODO: avaliar se foi empilhado adequadamente */
	//}

	//return (FALSE); /* TODO: avaliar se foi empilhado adequadamente */
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
		Header->WBUF = WriteBufferSize;
		Header->RBUF = ReadBufferSize;
		/* TODO: enqueue new packet */
		/* TODO: check IRQ pin as below */
		/* do not release spi if the next command is a read */
		//return (DO_NOT_RELEASE_SPI); /* SPI was already release and a new transfer was requested */
	}else
	{
		WriteBufferSize = 0;
		ReadBufferSize = 0;

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
	FreeBuffer = BluenrgMS_Get_Free_Frame_Buffer();

	if( FreeBuffer != NULL )
	{
		/* Load transmitting queue position */
		FreeBuffer->TransferDescPtr->TxPtr = (uint8_t*)( &SPIMasterHeaderRead.CTRL );
		FreeBuffer->TransferDescPtr->RxPtr = &SPISlaveHeader.READY;
		FreeBuffer->TransferDescPtr->DataSize = sizeof(SPISlaveHeader); /* Put always the size of reading buffer to avoid writing in random memory */
		FreeBuffer->TransferDescPtr->CallBackMode = CALL_BACK_AFTER_TRANSFER;
		FreeBuffer->TransferDescPtr->CallBack = (TransferCallBack)( &BluenrgMS_Slave_Header_CallBack );
		if( BluenrgMS_Enqueue_Frame( FreeBuffer, 1 ) != FALSE ) /* We have things to read, so put the command in the first position of the queue */
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Set_BluenrgMS_Last_Sent_Frame_Buffer()                       */
/* Purpose: Set the pointer of last sent frame buffer			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Set_BluenrgMS_Last_Sent_Frame_Buffer(BUFFER_POSITION_DESC* Buf)
{
	LastTransferBuf = Buf;
}


/****************************************************************/
/* Get_BluenrgMS_Last_Sent_Frame_Buffer()                       */
/* Purpose: Get the pointer of last sent frame buffer			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BUFFER_POSITION_DESC* Get_BluenrgMS_Last_Sent_Frame_Buffer(void)
{
	return ( LastTransferBuf );
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
