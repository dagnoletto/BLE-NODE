

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Master.h"
#include "App.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
uint8_t Config_Scanner(void);
uint8_t Config_Initiating(void);


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static SLAVE_ADV_INFO SlaveInfo;


/****************************************************************/
/* MasterNode()													*/
/* Location: 					 								*/
/* Purpose: Run master code										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void MasterNode( void )
{
	static BLE_STATES MasterStateMachine = CONFIG_STANDBY;
	static uint32_t TimerLED = 0;
	static uint32_t Timer = 0;

	switch( MasterStateMachine )
	{
	case CONFIG_STANDBY:
		Enter_Standby_Mode();
		if( TimeBase_DelayMs( &TimerLED, 100, TRUE )  )
		{
			HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
		}
		if( ( Get_BLE_State() == STANDBY_STATE ) && ( TimeBase_DelayMs( &Timer, 5000, TRUE ) ) )
		{
			if( SlaveInfo.AdvData.Size && SlaveInfo.ScanRspData.Size )
			{
				MasterStateMachine = Config_Initiating() ? CONFIG_INITIATING : CONFIG_STANDBY;
			}else
			{
				MasterStateMachine = Config_Scanner() ? CONFIG_SCANNING : CONFIG_STANDBY;
			}
		}
		break;

	case CONFIG_SCANNING:
		TimerLED = 0;
		HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_RESET );
		if( Get_BLE_State() == SCANNING_STATE )
		{
			SlaveInfo.AdvData.Size = 0;
			SlaveInfo.ScanRspData.Size = 0;
			MasterStateMachine = SCANNING_STATE;
		}
		break;

	case SCANNING_STATE:
		if( TimeBase_DelayMs( &TimerLED, 500, TRUE )  )
		{
			//HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
		}
		if( TimeBase_DelayMs( &Timer, 5000, TRUE ) )
		{
			//			SlaveInfo.AdvData.Size = 25;
			//			SlaveInfo.ScanRspData.Size = 17;
			//			MasterStateMachine = CONFIG_STANDBY;
		}
		if( SlaveInfo.AdvData.Size && SlaveInfo.ScanRspData.Size )
		{
			MasterStateMachine = CONFIG_STANDBY;
		}
		break;

	case CONFIG_INITIATING:
		TimerLED = 0;
		HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_RESET );
		if( Get_BLE_State() == INITIATING_STATE )
		{
			Timer = 0;
			SlaveInfo.AdvData.Size = 0;
			SlaveInfo.ScanRspData.Size = 0;
			MasterStateMachine = INITIATING_STATE;
		}
		break;

	case INITIATING_STATE:
		if( TimeBase_DelayMs( &TimerLED, 1000, TRUE ) )
		{
			HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
		}
		if( TimeBase_DelayMs( &Timer, 5000, TRUE ) )
		{
			MasterStateMachine = CONFIG_STANDBY;
		}
		break;

	default: break;
	}
}


/****************************************************************/
/* Config_Scanner()        	     								*/
/* Location: 					 								*/
/* Purpose: Set the operating BLE state in scanning mode		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Config_Scanner(void)
{
	DEVICE_IDENTITY Id;

	Id.Peer_Identity_Address.Type = PEER_PUBLIC_DEV_ADDR;
	Id.Peer_Identity_Address.Address = SlavePublicAddress;

	memset( &Id.Local_IRK.Bytes[0], 0, sizeof(Id.Local_IRK) );
	Id.Local_IRK.Bytes[0] = 5;
	Id.Local_IRK.Bytes[15] = 51;

	memset( &Id.Peer_IRK.Bytes[0], 0, sizeof(Id.Peer_IRK) );
	Id.Peer_IRK.Bytes[0] = 0;
	Id.Peer_IRK.Bytes[15] = 15;

	RESOLVING_RECORD Record;
	Record.Peer = Id;
	Record.Local_Identity_Address = Get_Identity_Address( PEER_PUBLIC_DEV_ADDR );

	Add_Record_To_Resolving_List( &Record );

	/* SCANNING */
	SCANNING_PARAMETERS Scan;

	Scan.LE_Scan_Type = ACTIVE_SCANNING;
	Scan.LE_Scan_Interval = 320;
	Scan.LE_Scan_Window = 320;
	Scan.Own_Address_Type = OWN_RANDOM_DEV_ADDR;
	Scan.Own_Random_Address_Type = NON_RESOLVABLE_PRIVATE;
	Scan.PeerId = Record.Peer.Peer_Identity_Address;
	Scan.Scanning_Filter_Policy = 0;
	Scan.Filter_Duplicates = 0;
	//Scan.Privacy = TRUE; /* Due to the fact in this 4.1 controller the address resolution is done in the Host, some scan response packets are lost */
	/* because the controller cannot unload the responses while servicing other Host's requests. The controller itself discards some responses */
	Scan.Privacy = FALSE;
	Scan.Role = CENTRAL;

	return ( Enter_Scanning_Mode( &Scan ) );
}


/****************************************************************/
/* Config_Initiating()     	     								*/
/* Location: 					 								*/
/* Purpose: Set the operating BLE state in initiating mode		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Config_Initiating(void)
{
	DEVICE_IDENTITY Id;

	Id.Peer_Identity_Address.Type = PEER_PUBLIC_DEV_ADDR;
	Id.Peer_Identity_Address.Address = SlavePublicAddress;

	memset( &Id.Local_IRK.Bytes[0], 0, sizeof(Id.Local_IRK) );
	Id.Local_IRK.Bytes[0] = 5;
	Id.Local_IRK.Bytes[15] = 51;

	memset( &Id.Peer_IRK.Bytes[0], 0, sizeof(Id.Peer_IRK) );
	Id.Peer_IRK.Bytes[0] = 0;
	Id.Peer_IRK.Bytes[15] = 15;

	RESOLVING_RECORD Record;
	Record.Peer = Id;
	Record.Local_Identity_Address = Get_Identity_Address( PEER_PUBLIC_DEV_ADDR );

	Add_Record_To_Resolving_List( &Record );

	/* INITIATING */
	INITIATING_PARAMETERS Init;

	Init.LE_Scan_Interval = 320;
	Init.LE_Scan_Window = 320;
	Init.Initiator_Filter_Policy = 0;
	Init.Peer_Address_Type = SlaveInfo.Address_Type; //PUBLIC_DEV_ADDR;
	Init.Peer_Address = SlaveInfo.Address; //SlavePublicAddress;
	Init.Own_Address_Type = OWN_RESOL_OR_PUBLIC_ADDR;//OWN_PUBLIC_DEV_ADDR;
	Init.Own_Random_Address_Type = NON_RESOLVABLE_PRIVATE;
	Init.Connection_Interval_Min = 80; /* 80 * 1.25ms = 100ms */
	Init.Connection_Interval_Max = 80; /* 80 * 1.25ms = 100ms */
	Init.Connection_Latency = 0;
	Init.Supervision_Timeout = 80; /* 80 * 10ms = 800ms */
	Init.Min_CE_Length = 80; /* 80 * 1.25ms = 100ms */
	Init.Max_CE_Length = 80; /* 80 * 1.25ms = 100ms */
	Init.Privacy = FALSE;

	return ( Enter_Initiating_Mode( &Init ) );
}


/****************************************************************/
/* HCI_LE_Advertising_Report()                					*/
/* Location: 2382 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Advertising_Report event indicates that 	*/
/* one or more Bluetooth devices have responded to an active 	*/
/* scan or have broadcast advertisements that were received 	*/
/* during a passive scan. The Controller may queue these 		*/
/* advertising reports and send information from multiple 		*/
/* devices in one HCI_LE_Advertising_Report event. This event 	*/
/* shall only be generated if scanning was enabled using the	*/
/* HCI_LE_Set_Scan_Enable command. It only reports advertising  */
/* events that used legacy advertising PDUs.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_LE_Advertising_Report( uint8_t Num_Reports, uint8_t Event_Type[], uint8_t Address_Type[], BD_ADDR_TYPE Address[],
		uint8_t Data_Length[], uint8_t Data[], int8_t RSSI[] )
{
	//static uint8_t AdvData[40];

	/* Sub event code for the HCI_LE_Advertising_Report event: page 2382 Core_v5.2 */
	ADVERTISING_REPORT Report;
	uint16_t Number_Of_Data_Bytes = 0;

	for( uint8_t i = 0; i < Num_Reports; i++ )
	{
		Report.Event_Type = Event_Type[i];
		Report.Address_Type = Address_Type[i];

		Report.Address = Address[i];
		Report.Data_Length = Data_Length[i];
		Report.RSSI = RSSI[i];
		//Report.DataPtr = &AdvData[0];

		//memcpy( Report.DataPtr, &Data[Number_Of_Data_Bytes], Report.Data_Length );

		if( ( ( Report.Data_Length == 25 ) && ( Report.Event_Type == ADV_IND_EVT ) ) || ( ( Report.Data_Length == 17 ) && ( Report.Event_Type == SCAN_RSP_EVT ) ) /* memcmp( &SlavePublicAddress, &Report.Address, sizeof(Report.Address) ) == 0 */ )
		{
			if( ( SlaveInfo.AdvData.Size ) && ( SlaveInfo.Address_Type == Report.Address_Type ) )
			{
				SlaveInfo.Address_Type = Report.Address_Type;
				SlaveInfo.Address = Report.Address;
				SlaveInfo.RSSI = Report.RSSI;
				if ( Report.Event_Type == SCAN_RSP_EVT )
				{
					SlaveInfo.ScanRspData.Size = MIN( Report.Data_Length, sizeof(SlaveInfo.ScanRspData.Bytes) );
					memcpy( &SlaveInfo.ScanRspData.Bytes[0], &Data[Number_Of_Data_Bytes], SlaveInfo.ScanRspData.Size );
					HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
				}else
				{
					SlaveInfo.AdvData.Size = MIN( Report.Data_Length, sizeof(SlaveInfo.AdvData.Bytes) );
					memcpy( &SlaveInfo.AdvData.Bytes[0], &Data[Number_Of_Data_Bytes], SlaveInfo.AdvData.Size );
				}
				//HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_SET );
			}else
			{
				SlaveInfo.Address_Type = Report.Address_Type;
				SlaveInfo.Address = Report.Address;
				SlaveInfo.RSSI = Report.RSSI;
				SlaveInfo.ScanRspData.Size = 0;
				if ( Report.Event_Type != SCAN_RSP_EVT )
				{
					SlaveInfo.AdvData.Size = MIN( Report.Data_Length, sizeof(SlaveInfo.AdvData.Bytes) );
					memcpy( &SlaveInfo.AdvData.Bytes[0], &Data[Number_Of_Data_Bytes], SlaveInfo.AdvData.Size );
				}
			}
			//HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
		}

		Number_Of_Data_Bytes += Report.Data_Length;

	}
}


/****************************************************************/
/* HCI_LE_Connection_Complete()                					*/
/* Location: 2379 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Connection_Complete event indicates to 	*/
/* both of the Hosts forming the connection that a new 			*/
/* connection has been created. Upon the creation of the		*/
/* connection a Connection_Handle shall be assigned by the		*/
/* Controller, and passed to the Host in this event. If the 	*/
/* connection creation fails this event shall be provided to 	*/
/* the Host that had issued the HCI_LE_Create_Connection 		*/
/* command. This event indicates to the Host which issued an 	*/
/* HCI_LE_Create_Connection command and received an 			*/
/* HCI_Command_Status event if the connection creation failed 	*/
/* or was successful. The Master_Clock_Accuracy parameter is 	*/
/* only valid for a slave. On a master, this parameter shall be */
/* set to 0x00. Note: This event is not sent if the 			*/
/* HCI_LE_Enhanced_Connection_Complete event 					*/
/* (see Section 7.7.65.10) is unmasked.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_LE_Connection_Complete( CONTROLLER_ERROR_CODES Status, uint16_t Connection_Handle, uint8_t Role, PEER_ADDR_TYPE Peer_Address_Type,
		BD_ADDR_TYPE* Peer_Address, uint16_t Connection_Interval, uint16_t Connection_Latency,
		uint16_t Supervision_Timeout, uint8_t Master_Clock_Accuracy )
{
	uint8_t teste = 0;
	if(teste)
	{
		teste = 0;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
