

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Slave.h"
#include "App.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	READ_REMOTE_VERSION_INFO,
	READ_REMOTE_FEATURES,
	SEND_DATA
}SERVER_STATES;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static void LE_Read_Remote_Features_Complete( CONTROLLER_ERROR_CODES Status,
		uint16_t Connection_Handle, LE_SUPPORTED_FEATURES* LE_Features );
static void LE_Read_Remote_Features_Status( CONTROLLER_ERROR_CODES Status );
static void Read_Remote_VerInfo_Complete( CONTROLLER_ERROR_CODES Status,
		REMOTE_VERSION_INFORMATION* Remote_Version_Information );
static void Read_Remote_VerInfo_Status( CONTROLLER_ERROR_CODES Status );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static SERVER_STATES ServerStateMachine = READ_REMOTE_VERSION_INFO;
static uint32_t NoDataPacketRspTimer = 0;


/****************************************************************/
/* Server()														*/
/* Location: 					 								*/
/* Purpose: Run server code										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Server( void )
{
	switch ( ServerStateMachine )
	{
	case READ_REMOTE_VERSION_INFO:
		NoDataPacketRspTimer = 0;
		Set_Default_Number_Of_HCI_Data_Packets();
		HCI_Read_Remote_Version_Information( MasterInfo.Connection_Handle, &Read_Remote_VerInfo_Complete, &Read_Remote_VerInfo_Status );
		break;

	case READ_REMOTE_FEATURES:
		HCI_LE_Read_Remote_Features( MasterInfo.Connection_Handle, &LE_Read_Remote_Features_Complete, &LE_Read_Remote_Features_Status );
		break;

	case SEND_DATA:
	{
		//static uint32_t Timer = 0;
		static uint32_t Timer2 = 0;

		if( /* TimeBase_DelayMs( &Timer, 100, TRUE ) */ 1 )
		{

			HCI_ACL_DATA_PCKT_HEADER ACLDataPacketHeader;

			uint8_t Data[7] = { 3, 0, 4, 0, 2, 15, 2 };
			ACLDataPacketHeader.Handle = MasterInfo.Connection_Handle;
			ACLDataPacketHeader.PB_Flag = 0x0;
			ACLDataPacketHeader.BC_Flag = 0x0;
			ACLDataPacketHeader.Data_Total_Length = sizeof(Data);

			if( HCI_Host_ACL_Data( &ACLDataPacketHeader, (uint8_t*)&Data ) )
			{
				NoDataPacketRspTimer = 0;
			}else if( TimeBase_DelayMs( &NoDataPacketRspTimer, 500, TRUE ) )
			{
				Set_Default_Number_Of_HCI_Data_Packets();
			}
		}

		if( TimeBase_DelayMs( &Timer2, 2500, TRUE ) )
		{
			//HCI_Disconnect( MasterInfo.Connection_Handle, REMOTE_USER_TERMINATED_CONNECTION, &Command_Status );
		}
	}
	break;

	default:
		break;
	}
}


/****************************************************************/
/* Reset_Server()												*/
/* Location: 					 								*/
/* Purpose: Reset server code									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Reset_Server( void )
{
	ServerStateMachine = READ_REMOTE_VERSION_INFO;
}


/****************************************************************/
/* Read_Remote_VerInfo_Complete()     	   						*/
/* Location: 					 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Remote_VerInfo_Complete( CONTROLLER_ERROR_CODES Status,
		REMOTE_VERSION_INFORMATION* Remote_Version_Information )
{
	if( Status == COMMAND_SUCCESS )
	{
		MasterInfo.Version = *Remote_Version_Information;
	}
	ServerStateMachine = SEND_DATA; //READ_REMOTE_FEATURES; //SEND_DATA; //TODO: READ_REMOTE_FEATURES;
}


/****************************************************************/
/* Read_Remote_VerInfo_Status()     	 						*/
/* Location: 					 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Remote_VerInfo_Status( CONTROLLER_ERROR_CODES Status )
{
	if( Status != COMMAND_SUCCESS )
	{
		ServerStateMachine = SEND_DATA; //READ_REMOTE_FEATURES; //SEND_DATA; //TODO: READ_REMOTE_FEATURES;
	}
}


/****************************************************************/
/* LE_Read_Remote_Features_Complete()     	   					*/
/* Location: 					 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Remote_Features_Complete( CONTROLLER_ERROR_CODES Status,
		uint16_t Connection_Handle, LE_SUPPORTED_FEATURES* LE_Features )
{
	if( ( Status == COMMAND_SUCCESS ) && ( MasterInfo.Connection_Handle == Connection_Handle ) )
	{
		/* TODO: For some reason, the unknown connection ID is returned when the LE_Read_Remote_Features is issued: we should investigate */
		MasterInfo.SupFeatures = *LE_Features;
	}
	ServerStateMachine = SEND_DATA;
}


/****************************************************************/
/* LE_Read_Remote_Features_Status()     	   					*/
/* Location: 					 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Remote_Features_Status( CONTROLLER_ERROR_CODES Status )
{
	if( Status != COMMAND_SUCCESS )
	{
		ServerStateMachine = SEND_DATA;
	}
}


#if ( BLE_NODE == BLE_SLAVE )
/****************************************************************/
/* HCI_Controller_ACL_Data()                					*/
/* Location: 1892 Core_v5.2		 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Controller_ACL_Data( HCI_ACL_DATA_PCKT_HEADER* ACLDataPacketHeader, uint8_t Data[] )
{
	static uint32_t Previous = 0;
	static uint32_t index = 0;
	static uint32_t Diff[100];
	uint32_t actual = ( Data[3] << 24 ) | ( Data[2] << 16 ) | ( Data[1] << 8 ) | Data[0];

	Diff[index] = actual - Previous;

	index++;
	if( index >= ( sizeof(Diff)/sizeof(uint32_t) ) )
	{
		index = 0;
	}

	Previous = actual;

	//HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_RESET );
	HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
}


/****************************************************************/
/* HCI_Number_Of_Completed_Packets()                			*/
/* Location: 2315 Core_v5.2		 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Number_Of_Completed_Packets( uint8_t Num_Handles, uint16_t Connection_Handle[], uint16_t Num_Completed_Packets[] )
{
	if( Num_Handles == 1 )
	{
		NoDataPacketRspTimer = 0;
		//Npackets += Num_Completed_Packets[0];
	}
}


/****************************************************************/
/* HCI_Data_Buffer_Overflow()                					*/
/* Location: 2325 Core_v5.2		 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Data_Buffer_Overflow( uint8_t Link_Type )
{

}
#endif


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
