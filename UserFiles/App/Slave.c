

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Slave.h"
#include "App.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Config_Advertiser(void);
static void Command_Status( CONTROLLER_ERROR_CODES Status );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static MASTER_INFO MasterInfo;


/****************************************************************/
/* SlaveNode()													*/
/* Location: 					 								*/
/* Purpose: Run slave code										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void SlaveNode( void )
{
	static BLE_STATES SlaveStateMachine = CONFIG_STANDBY;
	static uint32_t TimerLED = 0;
	static uint32_t Timer = 0;

	switch( SlaveStateMachine )
	{
	case CONFIG_STANDBY:
		Enter_Standby_Mode();
		if( TimeBase_DelayMs( &TimerLED, 100, TRUE )  )
		{
			HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
		}
		if( ( Get_BLE_State() == STANDBY_STATE ) && ( TimeBase_DelayMs( &Timer, 5000, TRUE ) ) )
		{
			SlaveStateMachine = Config_Advertiser() ? CONFIG_ADVERTISING : CONFIG_STANDBY;
		}
		break;

	case CONFIG_ADVERTISING:
		TimerLED = 0;
		MasterInfo.Connection_Handle = 0xFFFF;
		HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_RESET );
		if( Get_BLE_State() == ADVERTISING_STATE )
		{
			SlaveStateMachine = ADVERTISING_STATE;
		}
		break;

	case ADVERTISING_STATE:
		if( TimeBase_DelayMs( &TimerLED, 500, TRUE )  )
		{
			HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
		}
		if( TimeBase_DelayMs( &Timer, 10000, TRUE ) )
		{
			//SlaveStateMachine = CONFIG_STANDBY;
		}else if( Get_BLE_State() == CONNECTION_STATE )
		{
			SlaveStateMachine = CONNECTION_STATE;
		}
		break;

	case CONNECTION_STATE:
		HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_SET );
//		HCI_ACL_DATA_PCKT_HEADER ACLDataPacketHeader;
//
//		uint8_t Data[] = { 1 };
//		ACLDataPacketHeader.Handle = MasterInfo.Connection_Handle;
//		ACLDataPacketHeader.PB_Flag = 0x0;
//		ACLDataPacketHeader.BC_Flag = 0x0;
//		ACLDataPacketHeader.Data_Total_Length = sizeof(Data)/sizeof(typeof(Data));

		if( TimeBase_DelayMs( &Timer, 100, TRUE ) && ( MasterInfo.Connection_Handle != 0xFFFF ) )
		{
			//Enter_Standby_Mode();
			//SlaveStateMachine = CONFIG_STANDBY;
			//HCI_Disconnect( conhandle, REMOTE_USER_TERMINATED_CONNECTION, &Command_Status );
			//HCI_Host_ACL_Data( ACLDataPacketHeader, &Data[0] );
		}
		break;

	default: break;
	}
}


/****************************************************************/
/* Config_Advertiser()        	     							*/
/* Location: 					 								*/
/* Purpose: Set the operating BLE state in advertising mode		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Config_Advertiser(void)
{
	DEVICE_IDENTITY Id;

	Id.Peer_Identity_Address.Type = PEER_PUBLIC_DEV_ADDR;
	Id.Peer_Identity_Address.Address = MasterPublicAddress;

	memset( &Id.Local_IRK.Bytes[0], 0, sizeof(Id.Local_IRK) );
	Id.Local_IRK.Bytes[0] = 0;
	Id.Local_IRK.Bytes[15] = 15;

	memset( &Id.Peer_IRK.Bytes[0], 0, sizeof(Id.Peer_IRK) );
	Id.Peer_IRK.Bytes[0] = 5;
	Id.Peer_IRK.Bytes[15] = 51;

	RESOLVING_RECORD Record;
	Record.Peer = Id;
	Record.Local_Identity_Address = Get_Identity_Address( PEER_PUBLIC_DEV_ADDR );

	Add_Record_To_Resolving_List( &Record );

	/* ADVERTISING */
	ADVERTISING_PARAMETERS Adv;

	Adv.Advertising_Interval_Min = 160;
	Adv.Advertising_Interval_Max = 320;
	Adv.Advertising_Type = ADV_IND;
	Adv.Own_Address_Type = OWN_RESOL_OR_PUBLIC_ADDR;
	Adv.Own_Random_Address_Type = STATIC_DEVICE_ADDRESS;
	Adv.Peer_Address_Type = PEER_PUBLIC_DEV_ADDR;
	memcpy( &Adv.Peer_Address, &Id.Peer_Identity_Address.Address, sizeof(Adv.Peer_Address) );
	Adv.Advertising_Channel_Map.Val = DEFAULT_LE_ADV_CH_MAP;
	Adv.Advertising_Filter_Policy = 0;
	Adv.connIntervalmin = NO_SPECIFIC_MINIMUM;
	Adv.connIntervalmax = NO_SPECIFIC_MAXIMUM;
	Adv.Privacy = FALSE;
	Adv.Role = PERIPHERAL;
	Adv.DiscoveryMode = GENERAL_DISCOVERABLE_MODE;

	return ( Enter_Advertising_Mode( &Adv ) );
}


/****************************************************************/
/* Slave_Connection_Complete()     	    						*/
/* Location: 					 								*/
/* Purpose: Informs the slave a new connection was established  */
/* with a master												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Slave_Connection_Complete( LEEnhancedConnectionComplete* ConnCpltData )
{
	if( ConnCpltData->Status == COMMAND_SUCCESS )
	{
		MasterInfo.Connection_Handle = ConnCpltData->Connection_Handle;
	}
}


/****************************************************************/
/* Slave_Disconnection_Complete()     	    					*/
/* Location: 					 								*/
/* Purpose: Informs the slave a connection was terminated		*/
/* with the master												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Slave_Disconnection_Complete( DisconnectionComplete* DisConnCpltData )
{
	uint8_t teste = 0;
	if(teste)
	{
		teste = 0;
	}
}


/****************************************************************/
/* Command_Status()     	   									*/
/* Location: 					 								*/
/* Purpose:														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Command_Status( CONTROLLER_ERROR_CODES Status )
{
	uint8_t teste = 0;
	if( Status != COMMAND_SUCCESS && Status != COMMAND_DISALLOWED )
	{
		teste = 0;
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
	static HCI_ACL_DATA_PCKT_HEADER ACLDataPacketH;

	ACLDataPacketH = *ACLDataPacketHeader;
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

}
#endif


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
