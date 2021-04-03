

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "vendor_specific.h"


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
static REASON_CODE InitReason = REASON_RESERVED;


/****************************************************************/
/* Vendor_Specific_Init()        	        					*/
/* Location: 					 								*/
/* Purpose: Await for vendor specific events and do vendor 		*/
/* specific configuration.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Vendor_Specific_Init( void )
{
	/* TODO: Implement vendor specific configuration/initialization */
	if( InitReason == FIRMWARE_STARTED_PROPERLY )
	{
		return ( TRUE );
	}

	return ( FALSE );
}


/****************************************************************/
/* ACI_Blue_Initialized_Event()                    		      	*/
/* Purpose: Vendor Specific Event 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void ACI_Blue_Initialized_Event( REASON_CODE Code )
{
	/* Check if initialization was OK */
	InitReason = Code;
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
