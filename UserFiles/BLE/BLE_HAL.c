

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "BLE_HAL.h"
#include "main.h"
#include "spi.h"
#include "hci_transport_layer.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Data_TX[] = { 0xB0, 0, 0, 0, 0, 0 };
static uint8_t Data_RX[] = { 0, 0, 0, 0, 0, 0, 0 };
static HCI_SERIAL_EVENT_PCKT Teste;


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
	//HAL_SPI_TransmitReceive( &hspi1, &Data_TX[0], &Data_RX[0], 1, 100 );

	Data_TX[0] = 0X0B;
	//HAL_SPI_Receive( &hspi1, &Data_TX[0], 5, 100 );
	HAL_SPI_TransmitReceive_DMA( &hspi1, &Data_TX[0], &/*Teste*/Data_RX[0], 5 );
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
	volatile uint8_t i = 0;
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
	static volatile uint8_t i = 0;
	Data_TX[0] = 0X0B;
    /* Disable SPI Peripheral */
    __HAL_SPI_DISABLE(hspi);
	//HAL_SPI_DMAStop(&hspi1);
	//SPI_CloseRxTx_ISR(&hspi1);
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
	volatile uint8_t i = 0;
	//TODO: oque fazer nos erros?
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
