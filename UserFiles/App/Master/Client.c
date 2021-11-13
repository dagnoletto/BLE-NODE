

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Master.h"
#include "App.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	READ_REMOTE_VERSION_INFO,
	READ_REMOTE_FEATURES,
	SEND_DATA
}CLIENT_STATES;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static void LE_Read_Remote_Features_Complete( CONTROLLER_ERROR_CODES Status,
		uint16_t Connection_Handle, LE_SUPPORTED_FEATURES* LE_Features );
static void LE_Read_Remote_Features_Status( CONTROLLER_ERROR_CODES Status );
static void Read_Remote_VerInfo_Complete( CONTROLLER_ERROR_CODES Status,
		REMOTE_VERSION_INFORMATION* Remote_Version_Information );
static void Read_Remote_VerInfo_Status( CONTROLLER_ERROR_CODES Status );
static void HCI_Disconnect_Status( CONTROLLER_ERROR_CODES Status );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static CLIENT_STATES ClientStateMachine = READ_REMOTE_VERSION_INFO;
static uint32_t NoDataPacketRspTimer = 0;


/****************************************************************/
/* Client()														*/
/* Location: 					 								*/
/* Purpose: Run client code										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Client( void )
{
	switch ( ClientStateMachine )
	{
	case READ_REMOTE_VERSION_INFO:
		NoDataPacketRspTimer = 0;
		Set_Default_Number_Of_HCI_Data_Packets();
		HCI_Read_Remote_Version_Information( SlaveInfo.Connection_Handle, &Read_Remote_VerInfo_Complete, &Read_Remote_VerInfo_Status );
		break;

	case READ_REMOTE_FEATURES:
		HCI_LE_Read_Remote_Features( SlaveInfo.Connection_Handle, &LE_Read_Remote_Features_Complete, &LE_Read_Remote_Features_Status );
		break;

	case SEND_DATA:
	{
		static uint32_t Timer2 = 0;
		//static uint32_t Timer3 = 0;

		if( /* TimeBase_DelayMs( &Timer3, 10, TRUE ) */ 1 )
		{

			HCI_ACL_DATA_PCKT_HEADER ACLDataPacketHeader;

			uint32_t Data;
			ACLDataPacketHeader.Handle = SlaveInfo.Connection_Handle;
			ACLDataPacketHeader.PB_Flag = 0x0;
			ACLDataPacketHeader.BC_Flag = 0x0;
			ACLDataPacketHeader.Data_Total_Length = sizeof(Data);

			Data = HAL_GetTick();

			if( HCI_Host_ACL_Data( &ACLDataPacketHeader, (uint8_t*)&Data ) )
			{
				NoDataPacketRspTimer = 0;
			}else if( TimeBase_DelayMs( &NoDataPacketRspTimer, 500, TRUE ) )
			{
				Set_Default_Number_Of_HCI_Data_Packets();
			}
		}

		if( TimeBase_DelayMs( &Timer2, 5000, TRUE ) )
		{
			HCI_Disconnect( SlaveInfo.Connection_Handle, REMOTE_USER_TERMINATED_CONNECTION, &HCI_Disconnect_Status );
		}
	}
	break;

	default:
		break;
	}
}


/****************************************************************/
/* Reset_Client()												*/
/* Location: 					 								*/
/* Purpose: Reset client code									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Reset_Client( void )
{
	ClientStateMachine = READ_REMOTE_VERSION_INFO;
}


/****************************************************************/
/* Command_Status()     	   									*/
/* Location: 					 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
//static void Command_Status( CONTROLLER_ERROR_CODES Status )
//{
//	if( Status != COMMAND_SUCCESS && Status != COMMAND_DISALLOWED )
//	{
//
//	}
//}


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
		SlaveInfo.Version = *Remote_Version_Information;
	}
	ClientStateMachine = READ_REMOTE_FEATURES;
}


/****************************************************************/
/* Read_Remote_VerInfo_Status()     	   						*/
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
		ClientStateMachine = READ_REMOTE_FEATURES;
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
	if( ( Status == COMMAND_SUCCESS ) && ( SlaveInfo.Connection_Handle == Connection_Handle ) )
	{
		SlaveInfo.SupFeatures = *LE_Features;
	}
	ClientStateMachine = SEND_DATA;
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
		ClientStateMachine = SEND_DATA;
	}
}


/****************************************************************/
/* HCI_Disconnect_Status()     	   								*/
/* Location: 					 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void HCI_Disconnect_Status( CONTROLLER_ERROR_CODES Status )
{
	if( Status != COMMAND_SUCCESS )
	{

	}
}


#if ( BLE_NODE == BLE_MASTER )
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
	//HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_RESET );
	//HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
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

		//HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_SET );
		HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
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
