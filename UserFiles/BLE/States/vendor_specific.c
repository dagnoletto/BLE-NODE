

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "vendor_specific.h"
#include "TimeFunctions.h"
#include "ble_states.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef struct
{
	uint8_t Offset;
	uint8_t* DataPtr;
	uint16_t DataSize;
}JOB_LIST;


typedef struct
{
	int8_t NumberOfJobs;
	JOB_LIST JobList[];
}CONFIG_JOBS;


typedef struct
{
	CONFIG_STEPS Step;
	CONFIG_STEPS RqtType;
	VS_Callback CallBack;
	CONFIG_JOBS* Jobs;
}VS_STATE_MACHINE;


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define PUBLIC_ADDRESS_OFFSET 0x00
#define DIV_OFFSET 			  0x06
#define ER_OFFSET 			  0x08
#define IR_OFFSET 			  0x18
#define LLWITHOUTHOST_OFFSET  0x2C
#define ROLE_OFFSET 		  0x2D


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static BLE_STATUS Request_VS_Config( uint8_t RqtType, CONFIG_JOBS* Jobs, VS_Callback CallBackFun );
static uint8_t Check_Config_Request( uint8_t Offset, uint8_t* DataPtr, uint16_t DataSize );
static CONFIG_JOBS* All_Job_List( CONFIG_DATA* ConfigData );
static CONFIG_JOBS* Single_Job_List( uint8_t Offset, uint8_t* DataPtr, uint16_t DataSize );
static void Default_VS_Config_CallBack(void* Data);
static void Hal_Write_Config_Data_Event( CONTROLLER_ERROR_CODES Status );
static void Hal_Read_Config_Data_Event( CONTROLLER_ERROR_CODES Status, uint8_t* DataPtr, uint8_t DataSize );


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static VS_STATE_MACHINE Config = { .Step = CONFIG_BLOCKED, .Jobs = NULL };
static VS_Callback ConfigCallBack;
static uint32_t ConfigTimeoutCounter;


/****************************************************************/
/* Request_VS_Config()        	        						*/
/* Location: 					 								*/
/* Purpose: Read/write vendor specific configuration data.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static BLE_STATUS Request_VS_Config( uint8_t RqtType, CONFIG_JOBS* Jobs, VS_Callback CallBackFun )
{
	if( Config.Step == CONFIG_FREE )
	{
		if( ( Jobs != NULL ) && ( Jobs->NumberOfJobs >= 1 ) && ( ( RqtType == CONFIG_READ ) || ( RqtType == CONFIG_WRITE ) ) )
		{
			/* Check if all jobs are consistent */
			for( int8_t i = 0; i < Jobs->NumberOfJobs; i++ )
			{
				if( !Check_Config_Request( Jobs->JobList[i].Offset, Jobs->JobList[i].DataPtr, Jobs->JobList[i].DataSize ) )
				{
					return ( BLE_FALSE );
				}
			}
			Config.Step = RqtType;
			Config.RqtType = RqtType;
			Config.CallBack = CallBackFun;
			Config.Jobs = Jobs;
			return ( BLE_TRUE ); /* Operation will be executed */
		}
	}else if( Config.Step == CONFIG_BLOCKED )
	{
		return ( BLE_ERROR ); /* No operation will be done because of some failure */
	}

	return ( BLE_FALSE ); /* No operation will be done */
}


/****************************************************************/
/* Check_Config_Request()        	        					*/
/* Location: 					 								*/
/* Purpose: Check if configuration request parameters are OK	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Config_Request( uint8_t Offset, uint8_t* DataPtr, uint16_t DataSize )
{
	uint16_t MemberSize;

	switch ( Offset )
	{
	case PUBLIC_ADDRESS_OFFSET:
		MemberSize = sizeof( ((CONFIG_DATA*)NULL)->Public_address );
		break;

	case DIV_OFFSET:
		MemberSize = sizeof( ((CONFIG_DATA*)NULL)->DIV );
		break;

	case ER_OFFSET:
		MemberSize = sizeof( ((CONFIG_DATA*)NULL)->ER );
		break;

	case IR_OFFSET:
		MemberSize = sizeof( ((CONFIG_DATA*)NULL)->IR );
		break;

	case LLWITHOUTHOST_OFFSET:
		MemberSize = sizeof( ((CONFIG_DATA*)NULL)->LLWithoutHost );
		break;

	case ROLE_OFFSET:
		MemberSize = sizeof( ((CONFIG_DATA*)NULL)->Role );
		break;

	default:
		return (FALSE);
		break;
	}

	if ( ( MemberSize != DataSize ) || ( DataPtr == NULL ) )
	{
		return (FALSE);
	}else
	{
		return (TRUE);
	}
}


/****************************************************************/
/* Get_Config_Step()        	        						*/
/* Location: 					 								*/
/* Purpose: Request the configuration step.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
CONFIG_STEPS Get_Config_Step( void )
{
	return (Config.Step);
}


/****************************************************************/
/* Set_Config_Step()        	        						*/
/* Location: 					 								*/
/* Purpose: Set the configuration step.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Set_Config_Step( CONFIG_STEPS Step )
{
	Config.Step = Step;
}


/****************************************************************/
/* Vendor_Specific_Process()        	        				*/
/* Location: 					 								*/
/* Purpose: Process the VS configuration request				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Vendor_Specific_Process( void )
{
	int8_t JobIndex;

	switch ( Config.Step )
	{
	case CONFIG_FREE: /* Awaits requests */
		break;

	case CONFIG_READ:
		JobIndex = Config.Jobs->NumberOfJobs - 1;
		ConfigTimeoutCounter = 0;
		Config.Step = ACI_Hal_Read_Config_Data( Config.Jobs->JobList[JobIndex].Offset,
				&Hal_Read_Config_Data_Event, NULL ) ? CONFIG_WAIT : CONFIG_FAILURE;
		break;

	case CONFIG_WRITE:
		JobIndex = Config.Jobs->NumberOfJobs - 1;
		ConfigTimeoutCounter = 0;
		Config.Step = ACI_Hal_Write_Config_Data( Config.Jobs->JobList[JobIndex].Offset,
				Config.Jobs->JobList[JobIndex].DataSize, Config.Jobs->JobList[JobIndex].DataPtr,
				&Hal_Write_Config_Data_Event, NULL ) ? CONFIG_WAIT : CONFIG_FAILURE;
		break;

	case CONFIG_WAIT:
		Config.Step = TimeBase_DelayMs( &ConfigTimeoutCounter, 500, TRUE ) ? CONFIG_FAILURE : CONFIG_WAIT;
		break;

	case CONFIG_SUCCESS:
		Config.Jobs->NumberOfJobs--;
		if( Config.Jobs->NumberOfJobs == 0 )
		{
			uint8_t* DataPtr = Config.Jobs->JobList[0].DataPtr;
			free( Config.Jobs ); /* free allocated memory */
			ConfigCallBack = ( Config.CallBack == NULL ) ? &Default_VS_Config_CallBack : Config.CallBack;
			ConfigCallBack( DataPtr ); /* Return the first job data pointer */
			Config.Step = CONFIG_FREE;
		}else
		{
			Config.Step = Config.RqtType;
		}
		break;

	case CONFIG_FAILURE:
		free( Config.Jobs ); /* free allocated memory */
		ConfigCallBack = ( Config.CallBack == NULL ) ? &Default_VS_Config_CallBack : Config.CallBack;
		ConfigCallBack( NULL ); /* If pointer passed is NULL, that means operation was not OK */
		Config.Step = CONFIG_FREE;
		break;

	case CONFIG_BLOCKED:
	default:
		Config.Step = CONFIG_BLOCKED;
		break;
	}
}


/****************************************************************/
/* Default_VS_Config_CallBack()        	        				*/
/* Location: 					 								*/
/* Purpose: In the absence of application callback, this one is */
/* called.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Default_VS_Config_CallBack(void* Data)
{

}


/****************************************************************/
/* All_Job_List()        	   			     					*/
/* Location: 					 								*/
/* Purpose: Load Job List for the read/write all function.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static CONFIG_JOBS* All_Job_List( CONFIG_DATA* ConfigData )
{
	CONFIG_JOBS* Jobs = malloc( sizeof(Jobs->NumberOfJobs) + ( sizeof(JOB_LIST) * 6 ) );

	if( Jobs != NULL )
	{
		Jobs->JobList[0].Offset = PUBLIC_ADDRESS_OFFSET;
		Jobs->JobList[0].DataPtr = (uint8_t*)( &(ConfigData->Public_address) );
		Jobs->JobList[0].DataSize = sizeof( ConfigData->Public_address );

		Jobs->JobList[1].Offset = DIV_OFFSET;
		Jobs->JobList[1].DataPtr = (uint8_t*)( &(ConfigData->DIV) );
		Jobs->JobList[1].DataSize = sizeof( ConfigData->DIV );

		Jobs->JobList[2].Offset = ER_OFFSET;
		Jobs->JobList[2].DataPtr = (uint8_t*)( &(ConfigData->ER) );
		Jobs->JobList[2].DataSize = sizeof( ConfigData->ER );

		Jobs->JobList[3].Offset = IR_OFFSET;
		Jobs->JobList[3].DataPtr = (uint8_t*)( &(ConfigData->IR) );
		Jobs->JobList[3].DataSize = sizeof( ConfigData->IR );

		Jobs->JobList[4].Offset = LLWITHOUTHOST_OFFSET;
		Jobs->JobList[4].DataPtr = (uint8_t*)( &(ConfigData->LLWithoutHost) );
		Jobs->JobList[4].DataSize = sizeof( ConfigData->LLWithoutHost );

		Jobs->JobList[5].Offset = ROLE_OFFSET;
		Jobs->JobList[5].DataPtr = (uint8_t*)( &(ConfigData->Role) );
		Jobs->JobList[5].DataSize = sizeof( ConfigData->Role );

		Jobs->NumberOfJobs = 6;
	}

	return ( Jobs );
}


/****************************************************************/
/* Single_Job_List()        	   			     				*/
/* Location: 					 								*/
/* Purpose: Load single job.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static CONFIG_JOBS* Single_Job_List( uint8_t Offset, uint8_t* DataPtr, uint16_t DataSize )
{
	CONFIG_JOBS* Jobs = malloc( sizeof(Jobs->NumberOfJobs) + sizeof(JOB_LIST) );

	if( Jobs != NULL )
	{
		Jobs->JobList[0].Offset = Offset;
		Jobs->JobList[0].DataPtr = DataPtr;
		Jobs->JobList[0].DataSize = DataSize;

		Jobs->NumberOfJobs = 1;
	}

	return ( Jobs );
}


/****************************************************************/
/* Hal_Read_Config_Data_Event()         	      		      	*/
/* Purpose: Vendor Specific Event 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_Read_Config_Data_Event( CONTROLLER_ERROR_CODES Status, uint8_t* DataPtr, uint8_t DataSize )
{
	if ( ( Config.Step == CONFIG_WAIT ) && ( Config.RqtType == CONFIG_READ ) )
	{
		if( Status == COMMAND_SUCCESS )
		{
			int8_t Index = Config.Jobs->NumberOfJobs - 1;
			if( DataSize == Config.Jobs->JobList[Index].DataSize )
			{
				memcpy( Config.Jobs->JobList[Index].DataPtr, DataPtr, DataSize );
				Config.Step = CONFIG_SUCCESS;
			}else
			{
				Config.Step = CONFIG_FAILURE;
			}
		}else
		{
			Config.Step = CONFIG_FAILURE;
		}
	}
}


/****************************************************************/
/* Hal_Write_Config_Data_Event()               			      	*/
/* Purpose: Vendor Specific Event 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_Write_Config_Data_Event( CONTROLLER_ERROR_CODES Status )
{
	if ( ( Config.Step == CONFIG_WAIT ) && ( Config.RqtType == CONFIG_WRITE ) )
	{
		if( Status == COMMAND_SUCCESS )
		{
			Config.Step = CONFIG_SUCCESS;

		}else
		{
			Config.Step = CONFIG_FAILURE;
		}
	}
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
	/* Treat all other modes as blocked. If a different mode after
	* reset is requested, this code must change accordingly */
	Config.Step = ( Code == FIRMWARE_STARTED_PROPERLY ) ? CONFIG_FREE : CONFIG_BLOCKED;
}


/****************************************************************/
/* Read_Config_Data()        	   			     				*/
/* Location: 					 								*/
/* Purpose: Read all configuration fields						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BLE_STATUS Read_Config_Data( CONFIG_DATA* ConfigData, VS_Callback CallBackFun )
{
	if( Config.Step == CONFIG_FREE )
	{
		CONFIG_JOBS* Jobs = All_Job_List( ConfigData );
		if( Jobs != NULL )
		{
			BLE_STATUS status = Request_VS_Config( CONFIG_READ, Jobs, CallBackFun );
			if( status != BLE_TRUE )
			{
				free ( Jobs );
			}
			return ( status );
		}
	}

	return (BLE_FALSE);
}


/****************************************************************/
/* Write_Config_Data()        	   		     					*/
/* Location: 					 								*/
/* Purpose: Write all configuration fields 						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BLE_STATUS Write_Config_Data( CONFIG_DATA* ConfigData, VS_Callback CallBackFun )
{
	if( Config.Step == CONFIG_FREE )
	{
		BLE_STATES BleState = Get_BLE_State();

		if( BleState == VENDOR_SPECIFIC_INIT )
		{
			CONFIG_JOBS* Jobs = All_Job_List( ConfigData );
			if( Jobs != NULL )
			{
				BLE_STATUS status = Request_VS_Config( CONFIG_WRITE, Jobs, CallBackFun );
				if( status != BLE_TRUE )
				{
					free ( Jobs );
				}
				return ( status );
			}
		}
	}

	return (BLE_FALSE);
}


/****************************************************************/
/* Read_Public_Address()        	   		     				*/
/* Location: 					 								*/
/* Purpose: Read public address 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BLE_STATUS Read_Public_Address( BD_ADDR_TYPE* Public_Address, VS_Callback CallBackFun )
{
	if( Config.Step == CONFIG_FREE )
	{
		CONFIG_JOBS* Jobs = Single_Job_List( PUBLIC_ADDRESS_OFFSET, &Public_Address->Bytes[0], sizeof(BD_ADDR_TYPE) );
		if( Jobs != NULL )
		{
			BLE_STATUS status = Request_VS_Config( CONFIG_READ, Jobs, CallBackFun );
			if( status != BLE_TRUE )
			{
				free ( Jobs );
			}
			return ( status );
		}

	}

	return (BLE_FALSE);
}


/****************************************************************/
/* Write_Public_Address()        	   		     				*/
/* Location: 					 								*/
/* Purpose: Write public address 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BLE_STATUS Write_Public_Address( BD_ADDR_TYPE* Public_Address, VS_Callback CallBackFun )
{
	if( Config.Step == CONFIG_FREE )
	{
		BLE_STATES BleState = Get_BLE_State();

		if( BleState == VENDOR_SPECIFIC_INIT )
		{
			CONFIG_JOBS* Jobs = Single_Job_List( PUBLIC_ADDRESS_OFFSET, &Public_Address->Bytes[0], sizeof(BD_ADDR_TYPE) );
			if( Jobs != NULL )
			{
				BLE_STATUS status = Request_VS_Config( CONFIG_WRITE, Jobs, CallBackFun );
				if( status != BLE_TRUE )
				{
					free ( Jobs );
				}
				return ( status );
			}
		}
	}

	return (BLE_FALSE);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
