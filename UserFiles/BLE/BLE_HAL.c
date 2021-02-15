

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
/* Get_BluenrgMS_IRQ_Pin()         	                            */
/* Purpose: Get IRQ pin state				    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Get_BluenrgMS_IRQ_Pin(void)
{
	return( HAL_GPIO_ReadPin( BLE_IRQ_GPIO_Port, BLE_IRQ_Pin ) );
}


/****************************************************************/
/* Release_BluenrgMS_SPI()        	                            */
/* Purpose: Release (unselect) BluenrgMS	    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Release_BluenrgMS_SPI(void)
{
	__HAL_SPI_DISABLE(&hspi1);
}


/****************************************************************/
/* Send_BluenrgMS_SPI_Frame()            				        */
/* Purpose: Send SPI message to BluenrgMS	    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Send_BluenrgMS_SPI_Frame(uint8_t* TxPtr, uint8_t* RxPtr, uint8_t DataSize)
{
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_DMA( &hspi1, TxPtr, RxPtr, DataSize );

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
	Status_BluenrgMS_SPI_Frame( TRANSFER_DONE );
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
	Release_BluenrgMS_SPI();
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
	Release_BluenrgMS_SPI();
	//TODO: oque fazer nos erros?
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
