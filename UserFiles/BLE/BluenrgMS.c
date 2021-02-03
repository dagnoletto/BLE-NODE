

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "BluenrgMS.h"


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
/* Reset_BluenrgMS()                                            */
/* Purpose: Hardware reset for Bluenrg-MS module 	    		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Reset_BluenrgMS(void)
{
	uint32_t TimerResetActive = 0;
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

}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
