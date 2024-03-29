

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Bluenrg.h"
#include "InterruptCallbacks.h"


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
/* HAL_IncTick()               			                      	*/
/* Purpose: Systick increment			    					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_IncTick(void)
{
  uwTick += uwTickFreq;
  Bluerng_Command_Timeout(  );
}


/****************************************************************/
/* HAL_GPIO_EXTI_Callback()                                     */
/* Purpose: Handler for External Interrupts			    		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if( GPIO_Pin == BLE_IRQ_Pin )
	{
		Bluenrg_IRQ();
	}

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
