

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
	WAIT_CONFIG_MODE 	 	 =  4,
	CONFIG_READ 	 		 =  5,
	WAIT_CONFIG_READ 	 	 =  6,
	CONFIG_VERIFY 	 		 =  7,
	CONFIG_FINISHED	 		 =  8,
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
static BD_ADDR_TYPE DEFAULT_PUBLIC_ADDRESS = { { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 } };
static CONFIG_STEPS Config = WAIT_FW_STARTED_PROPERLY;
static CONFIG_DATA* ConfigDataPtr = NULL;
//TODO: fazer este código de forma que a leitura e escrita de parâmetros ocorra
//usando todos os dados de CONFIG_DATA, chamando funções do tipo read() a write() CONFIG_DATA, talvez com call backs para o retorno de chamadas


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
	{
		uint8_t LLWithoutHost = TRUE; /* The module should operate only in Link Layer only mode */
		if( ACI_Hal_Write_Config_Data( 0x2C, 1, &LLWithoutHost ) )
		{
			Config = WAIT_CONFIG_MODE;
		}
	}
	break;

	case WAIT_CONFIG_MODE:
		break;

	case CONFIG_READ:
		if( ACI_Hal_Read_Config_Data( 0 ) ) /* TODO: SÓ RETORNA A QUANTIDADE NECESSÁRIA POR OFFSET */
		{
			if( ConfigDataPtr != NULL )
			{
				free( ConfigDataPtr );
				ConfigDataPtr = NULL;
			}
			Config = WAIT_CONFIG_READ;
		}
		break;

	case WAIT_CONFIG_READ:
		break;

	case CONFIG_VERIFY:
		if( ConfigDataPtr != NULL )
		{
			Config = SET_PUBLIC_ADDRESS;
			if( ConfigDataPtr->LLWithoutHost ) /* Only the link layer is functional */
			{
				if( !memcmp( &( ConfigDataPtr->Public_address ), &DEFAULT_PUBLIC_ADDRESS, sizeof (BD_ADDR_TYPE) ) )
				{
					Config = CONFIG_FINISHED;
				}
			}
			free( ConfigDataPtr );
			ConfigDataPtr = NULL;
		}
		break;

	case CONFIG_FINISHED:
		return ( BLE_TRUE );
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

	case WAIT_CONFIG_MODE:
		if( ( Event == COMMAND_COMPLETE ) && ( ErrorCode == COMMAND_SUCCESS ) )
		{
			Config = CONFIG_READ;
		}else
		{
			Config = CONFIG_MODE;
		}
		break;

	default:
		break;
	}
}


/****************************************************************/
/* ACI_Hal_Read_Config_Data_Event()               		      	*/
/* Purpose: Vendor Specific Event 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void ACI_Hal_Read_Config_Data_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode, uint8_t* DataPtr, uint8_t DataSize )
{
	switch ( Config )
	{
	case WAIT_CONFIG_READ:
		if( ( Event == COMMAND_COMPLETE ) && ( ErrorCode == COMMAND_SUCCESS ) )
		{
			if( ConfigDataPtr != NULL )
			{
				free( ConfigDataPtr );
				ConfigDataPtr = NULL;
			}

			ConfigDataPtr = malloc( sizeof ( CONFIG_DATA ) );

			if( ( ConfigDataPtr != NULL ) && ( DataSize <= sizeof ( CONFIG_DATA ) ) )
			{
				memcpy( ConfigDataPtr, DataPtr, sizeof ( CONFIG_DATA ) );
				Config = CONFIG_VERIFY;
			}else
			{
				Config = CONFIG_READ;
			}
		}else
		{
			Config = CONFIG_READ;
		}
		break;

	default:
		break;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
