

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "BLE_HAL.h"
#include "main.h"
#include "spi.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static TRANSFER_DESCRIPTOR* LastTransferDescPtr = NULL;


/****************************************************************/
/* Clr_BluenrgMS_Reset_Pin()                                    */
/* Purpose: Clear (put 0V) to Bluenrg reset pin    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Clr_BluenrgMS_Reset_Pin(void)
{
	HAL_GPIO_WritePin( BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_RESET );
}


/****************************************************************/
/* Set_BluenrgMS_Reset_Pin()                                    */
/* Purpose: Set (put VDD) to Bluenrg reset pin    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Set_BluenrgMS_Reset_Pin(void)
{
	HAL_GPIO_WritePin( BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_SET );
}


/****************************************************************/
/* Request_BluenrgMS_Frame_Transmission()            	        */
/* Purpose: Send SPI message to BluenrgMS	    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Request_BluenrgMS_Frame_Transmission(TRANSFER_DESCRIPTOR* TransferDescPtr)
{
	if( HAL_SPI_TransmitReceive_DMA( &hspi1, TransferDescPtr->TxPtr, TransferDescPtr->RxPtr, TransferDescPtr->DataSize ) == HAL_OK )
	{
		LastTransferDescPtr = TransferDescPtr;
		return (TRUE);
	}else
	{
		LastTransferDescPtr = NULL;
		return (FALSE);
	}

	//TODO: disable half complete for transmission and disable interrupt on
	//completion if there is only one frame and the frame has no callback
}


/****************************************************************/
/* HAL_SPI_RxCpltCallback()        			                    */
/* Purpose: DMA RX complete callback				 	    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	__HAL_SPI_DISABLE(hspi);

}


/****************************************************************/
/* HAL_SPI_TxRxCpltCallback()        	                        */
/* Purpose: DMA TX and RX complete callback			 	    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	__HAL_SPI_DISABLE(hspi);

	if( LastTransferDescPtr != NULL )
	{
		if( LastTransferDescPtr->CallBack != NULL )
		{
			if( LastTransferDescPtr->CallBackMode == CALL_BACK_AFTER_TRANSFER )
			{
				LastTransferDescPtr->CallBack( LastTransferDescPtr->RxPtr, TRANSFER_DONE );
			}else
			{
				//TODO: put in the callback queue
			}
		}
	}
	//HAL_SPI_Receive( &hspi1, &Data_TX[0], 5, 100 );
	//HAL_SPI_TransmitReceive_DMA( &hspi1, &Data_TX[0], &/*Teste*/Data_RX[0], 5 );
	//	if( Data_RX[3] != 0 && Data_RX[0] == 0x02 )
	//	{
	//		i = 0;
	//		HAL_SPI_TransmitReceive_DMA( &hspi1, &Data_TX[0], &/*Teste*/Data_RX[0], Data_RX[3] );
	//	}else
	//	{
	//		HAL_SPI_TransmitReceive_DMA( &hspi1, &Data_TX[0], &/*Teste*/Data_RX[0], 6 );
	//	}
}


/****************************************************************/
/* HAL_SPI_ErrorCallback()        		                        */
/* Purpose: DMA error callback	   					 	    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
	__HAL_SPI_DISABLE(hspi);


	//TODO: oque fazer nos erros?
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
