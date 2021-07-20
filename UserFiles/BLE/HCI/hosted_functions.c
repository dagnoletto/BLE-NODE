

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hosted_functions.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Address_Resol_Controller = FALSE;


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
/* Hosted_LE_Set_Address_Resolution_Enable()               	    */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Set_Address_Resolution_Enable(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = (HCI_SERIAL_COMMAND_PCKT*)DataPtr;
	return (TRUE);
}


/****************************************************************/
/* Hosted_LE_Clear_Resolving_List()               	            */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Clear_Resolving_List(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = (HCI_SERIAL_COMMAND_PCKT*)DataPtr;
	return (TRUE);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
