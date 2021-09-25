

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "App.h"


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
const BD_ADDR_TYPE MasterPublicAddress = { { 0, 10, 20, 30, 40, 50 } };
const BD_ADDR_TYPE SlavePublicAddress  = { { 0,  1,  2,  3,  4,  5 } };


/****************************************************************/
/* App_Init()            		                                */
/* Purpose: Used to configure the application             		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void App_Init(void)
{

}


/****************************************************************/
/* App_Run()            		                                */
/* Purpose: Run (execute) the main application             		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void App_Run(void)
{
#if ( BLE_NODE == BLE_MASTER )

	MasterNode();

#else

	SlaveNode();

#endif
}


/****************************************************************/
/* Configure_Public_Device_Address()							*/
/* Location: 					 								*/
/* Purpose: Retrieve the public address used by the device. 	*/
/* It should be implemented on application side.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BD_ADDR_TYPE Configure_Public_Device_Address( void )
{
	BD_ADDR_TYPE PublicAddr;

#if ( BLE_NODE == BLE_MASTER )

	PublicAddr = MasterPublicAddress;

#else

	PublicAddr = SlavePublicAddress;

#endif

	return (PublicAddr);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
