

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t BLE_Init( void );
static uint8_t Vendor_Specific_Init( void );
static void Vendor_Specific_Init_CallBack( void* ConfigData );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static BLE_STATES BLEState = VENDOR_SPECIFIC_INIT;
static BLE_STATUS VS_Init_Done_Flag;
static BLE_STATUS Standby_Flag = BLE_FALSE;
static BD_ADDR_TYPE DEFAULT_PUBLIC_ADDRESS = { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 } };


/****************************************************************/
/* Run_BLE()        	        								*/
/* Location: 					 								*/
/* Purpose: Run the BLE protocol								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Run_BLE( void )
{
	switch( BLEState )
	{
	case VENDOR_SPECIFIC_INIT:
		if( Vendor_Specific_Init(  ) )
		{
			BLEState = BLE_INITIAL_SETUP;
		}
		break;

	case BLE_INITIAL_SETUP:
		if( BLE_Init(  ) )
		{
			Standby_Flag = BLE_FALSE;
			BLEState = STANDBY_STATE;
		}
		break;

	case STANDBY_STATE:
		if( Standby_Flag != BLE_TRUE )
		{
			if( ACI_Hal_Device_Standby(  ) )
			{
				Standby_Flag = BLE_TRUE;
			}
		}
		break;

	case ADVERTISING_STATE:
		break;

	case SCANNING_STATE:
		break;

	case INITIATING_STATE:
		break;

	case CONNECTION_STATE:
		break;

	case SYNCHRONIZATION_STATE:
		break;

	case ISOCHRONOUS_BROADCASTING_STATE:
		break;
	}

	Vendor_Specific_Process();
}


/****************************************************************/
/* Get_BLE_State()        	       								*/
/* Location: 					 								*/
/* Purpose: Get the operating BLE state							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BLE_STATES Get_BLE_State( void )
{
	return( BLEState );
}


/****************************************************************/
/* BLE_Init()        	       									*/
/* Location: 					 								*/
/* Purpose: Init BLE 											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t BLE_Init( void )
{

}


/****************************************************************/
/* Vendor_Specific_Init()        	        					*/
/* Location: 					 								*/
/* Purpose: Await for vendor specific events and do vendor 		*/
/* specific configuration.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Vendor_Specific_Init( void )
{
	typedef enum
	{
		WRITE_CONFIG_DATA  = 0x00,
		VERIFY_CONFIG_DATA = 0x01,
		WAIT_CONFIG_END	   = 0x02,
		END_CONFIG_MODE	   = 0x03
	}INIT_STEPS;

	static uint8_t Result = FALSE;
	static INIT_STEPS InitSteps = WRITE_CONFIG_DATA;
	static CONFIG_DATA* ConfigDataPtr = NULL;

	switch ( InitSteps )
	{
	case WRITE_CONFIG_DATA:

		VS_Init_Done_Flag = BLE_FALSE;
		if( ConfigDataPtr != NULL )
		{
			free( ConfigDataPtr );
			ConfigDataPtr = NULL;
		}
		ConfigDataPtr = malloc( sizeof(CONFIG_DATA) );

		if( ConfigDataPtr != NULL )
		{
			ConfigDataPtr->Public_address = DEFAULT_PUBLIC_ADDRESS;
			ConfigDataPtr->LLWithoutHost = LL_ONLY;
			ConfigDataPtr->Role = SLAVE_AND_MASTER_12KB;

			if( Write_Config_Data( ConfigDataPtr, &Vendor_Specific_Init_CallBack ) == BLE_TRUE )
			{
				InitSteps = VERIFY_CONFIG_DATA;
			}else
			{
				free( ConfigDataPtr );
				ConfigDataPtr = NULL;
			}
		}
		break;

	case VERIFY_CONFIG_DATA:
		if( VS_Init_Done_Flag != BLE_FALSE )
		{
			if( VS_Init_Done_Flag == BLE_TRUE )
			{
				VS_Init_Done_Flag = BLE_FALSE;
				memset( ConfigDataPtr, 0, sizeof(CONFIG_DATA) ); /* Clear bytes */
				if( Read_Config_Data( ConfigDataPtr, &Vendor_Specific_Init_CallBack ) == BLE_TRUE )
				{
					InitSteps = WAIT_CONFIG_END;
				}else
				{
					if( ConfigDataPtr != NULL )
					{
						free( ConfigDataPtr );
						ConfigDataPtr = NULL;
					}
					InitSteps = WRITE_CONFIG_DATA;
				}
			}else
			{
				if( ConfigDataPtr != NULL )
				{
					free( ConfigDataPtr );
					ConfigDataPtr = NULL;
				}
				InitSteps = WRITE_CONFIG_DATA;
			}
		}
		break;

	case WAIT_CONFIG_END:
		if( VS_Init_Done_Flag != BLE_FALSE )
		{
			Result = FALSE;
			if( ConfigDataPtr != NULL )
			{
				if( memcmp( &ConfigDataPtr->Public_address, &DEFAULT_PUBLIC_ADDRESS, sizeof(BD_ADDR_TYPE) ) == 0 )
				{
					/* TODO: If the public address was updated, we assume all other fields were updated as well */
					Result = TRUE;
				}
				free( ConfigDataPtr );
				ConfigDataPtr = NULL;
			}
			InitSteps = END_CONFIG_MODE;
		}
		break;

	case END_CONFIG_MODE:
		if ( Get_Config_Step() == CONFIG_FREE )
		{
			InitSteps = WRITE_CONFIG_DATA;
			return ( Result );
		}
		break;
	}

	return ( FALSE );
}


/****************************************************************/
/* Vendor_Specific_Init_CallBack()        	       				*/
/* Location: 					 								*/
/* Purpose: Called to indicate the status of configuration.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Vendor_Specific_Init_CallBack( void* ConfigData )
{
	if( ConfigData != NULL )
	{
		VS_Init_Done_Flag = BLE_TRUE;
	}else
	{
		VS_Init_Done_Flag = BLE_ERROR;
	}
}


/****************************************************************/
/* ACI_Hal_Device_Standby_Event()        	       				*/
/* Location: 					 								*/
/* Purpose: Called to indicate the status of standby command.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void ACI_Hal_Device_Standby_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode )
{
	if( ( Event == COMMAND_COMPLETE ) && ( ErrorCode == COMMAND_SUCCESS ) )
	{
		Standby_Flag = BLE_TRUE;
	}else
	{
		Standby_Flag = BLE_ERROR;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
