

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "vendor_specific.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	VS_INIT_FAILURE			 = -1,
	WAIT_FW_STARTED_PROPERLY =  0,
	SET_PUBLIC_ADDRESS 		 =  1,
	WAIT_PUBLIC_ADDRESS 	 =  2,
	CONFIG_MODE				 =  3,
}CONFIG_STEPS;


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
const BD_ADDR_TYPE DEFAULT_PUBLIC_ADDRESS = { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 } };
static CONFIG_STEPS Config = WAIT_FW_STARTED_PROPERLY;


/****************************************************************/
/* Vendor_Specific_Init()        	        					*/
/* Location: 					 								*/
/* Purpose: Await for vendor specific events and do vendor 		*/
/* specific configuration.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BLE_STATUS Vendor_Specific_Init( void )
{
	/* TODO: Implement vendor specific configuration/initialization */
	switch ( Config )
	{
	case WAIT_FW_STARTED_PROPERLY:
		break;

	case SET_PUBLIC_ADDRESS:
		if( ACI_Hal_Write_Config_Data( 0, sizeof(BD_ADDR_TYPE), (uint8_t*)( &DEFAULT_PUBLIC_ADDRESS.Byte ) ) )
		{
			Config = WAIT_PUBLIC_ADDRESS;
		}
		break;

	case WAIT_PUBLIC_ADDRESS:
		break;

	case CONFIG_MODE:
		break;

	case VS_INIT_FAILURE:
	default:
		return ( BLE_ERROR );
		break;

	}

	return ( BLE_FALSE );
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
	if ( Code == FIRMWARE_STARTED_PROPERLY )
	{
		Config = SET_PUBLIC_ADDRESS;
	}else
	{
		/* Treat all other modes as failure. If a different mode after
		 * reset is requested, this code must change accordingly */
		Config = VS_INIT_FAILURE;
	}
}


/****************************************************************/
/* ACI_Hal_Write_Config_Data_Event()               		      	*/
/* Purpose: Vendor Specific Event 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void ACI_Hal_Write_Config_Data_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode )
{
	switch ( Config )
	{
	case WAIT_PUBLIC_ADDRESS:
		if( ( Event == COMMAND_COMPLETE ) && ( ErrorCode == COMMAND_SUCCESS ) )
		{
			Config = CONFIG_MODE;
		}else
		{
			Config = SET_PUBLIC_ADDRESS;
		}
		break;

	default:
		break;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
