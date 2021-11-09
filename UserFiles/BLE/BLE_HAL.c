

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


/****************************************************************/
/* Clr_Bluenrg_Reset_Pin()                                    	*/
/* Purpose: Clear (put 0V) to Bluenrg reset pin    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Clr_Bluenrg_Reset_Pin(void)
{
	HAL_GPIO_WritePin( BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_RESET );
}


/****************************************************************/
/* Set_Bluenrg_Reset_Pin()                                    	*/
/* Purpose: Set (put VDD) to Bluenrg reset pin    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Set_Bluenrg_Reset_Pin(void)
{
	HAL_GPIO_WritePin( BLE_RST_GPIO_Port, BLE_RST_Pin, GPIO_PIN_SET );
}


/****************************************************************/
/* Get_Bluenrg_IRQ_Pin()         	                            */
/* Purpose: Get IRQ pin state				    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Get_Bluenrg_IRQ_Pin(void)
{
	return( HAL_GPIO_ReadPin( BLE_IRQ_GPIO_Port, BLE_IRQ_Pin ) );
}


/****************************************************************/
/* Release_Bluenrg()        	    		                    */
/* Purpose: Release (unselect) Bluenrg SPI port.	    		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Release_Bluenrg(void)
{
	__HAL_SPI_DISABLE(&hspi1);
}


/****************************************************************/
/* Bluenrg_Abort_Transfer()        	    		                */
/* Purpose: Abort any ongoing SPI transfer.			    		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Bluenrg_Abort_Transfer(void)
{
	HAL_SPI_Abort(&hspi1);
}


/****************************************************************/
/* Bluenrg_Send_Frame()            					        	*/
/* Purpose: Send SPI frame to Bluenrg.	    		    		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Bluenrg_Send_Frame(SPI_TRANSFER_MODE Mode, uint8_t* TxPtr, uint8_t* RxPtr, uint16_t DataSize)
{
	HAL_StatusTypeDef status = HAL_ERROR;

	switch( Mode )
	{
	case SPI_WRITE:
		/* We must only use transmitting pointer to avoid reading back some garbage back to host memory */
		status = HAL_SPI_Transmit_DMA( &hspi1, TxPtr, DataSize );
		break;

	case SPI_HEADER_READ:
	case SPI_HEADER_WRITE:
	case SPI_READ:
		/* In this case the system will either handle a fixed amount of TX and RX bytes ( SPI_HEADER_READ )
		 * or an amount of read bytes with dummy bytes being sent. We don't need to worry with dummy bytes sent. */
		status = HAL_SPI_TransmitReceive_DMA( &hspi1, TxPtr, RxPtr, DataSize );
		break;
	}

	if( status != HAL_OK )
	{
		return (FALSE);
	}

	return (TRUE);
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
	Bluenrg_Frame_Status( TRANSFER_DONE );
}


/****************************************************************/
/* HAL_SPI_TxCpltCallback()        	                        	*/
/* Purpose: DMA TX complete callback			 	    		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	Bluenrg_Frame_Status( TRANSFER_DONE );
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
	Release_Bluenrg();
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
	Bluenrg_Frame_Status( TRANSFER_DEV_ERROR );
	Release_Bluenrg();
}


/****************************************************************/
/* Bluenrg_Error()               	                            */
/* Purpose: Called whenever the device has a problem	  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Bluenrg_Error(BLUENRG_ERROR_CODES Errorcode)
{
	Reset_Bluenrg( TRUE );
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
