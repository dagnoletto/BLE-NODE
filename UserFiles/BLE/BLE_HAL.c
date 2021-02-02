

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
static uint8_t Data_TX[] = { 6, 7, 8};
static uint8_t Data_RX[] = { 6, 7, 8};


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
/* Send_SPI_Frame_To_BluenrgMS()                                */
/* Purpose: Send SPI message to BluenrgMS	    		    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Send_SPI_Frame_To_BluenrgMS(void)
{
	HAL_SPI_Transmit_DMA(&hspi1, &Data_TX[0], sizeof(Data_TX));
	HAL_SPI_Receive_DMA(&hspi1, &Data_RX[0], sizeof(Data_RX));
}


/****************************************************************/
/* HAL_SPI_TxHalfCpltCallback()                                 */
/* Purpose: Callback from half complete TX DMA	    	    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
	volatile uint8_t i = 0;
}


/****************************************************************/
/* HAL_SPI_TxCpltCallback()        		                        */
/* Purpose: Callback from complete TX DMA	   		 	    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	volatile uint8_t i = 0;
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
	volatile uint8_t i = 0;
}


/****************************************************************/
/* HAL_SPI_RxHalfCpltCallback()        		                    */
/* Purpose: DMA RX half	complete   					 	    	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_SPI_RxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
	volatile uint8_t i = 0;
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
