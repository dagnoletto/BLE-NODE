

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
		}else if( Get_BLE_State() == CONNECTION_STATE )
		{
			MasterStateMachine = CONNECTION_STATE;
		}
		break;

	case CONNECTION_STATE:
		HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_SET );
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
/* Advertising_Report()     	    							*/
/* Location: 					 								*/
/* Purpose: Informs the master that new reports arrived. 		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Advertising_Report( uint8_t Num_Reports, uint8_t Event_Type[], uint8_t Address_Type[], BD_ADDR_TYPE Address[],
		uint8_t Data_Length[], uint8_t Data[], int8_t RSSI[] )
{
	//TODO: In the future this function may fill-up a list and the higher layers
	//could retrieve devices from this list

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
/* Master_Connection_Complete()     	    					*/
/* Location: 					 								*/
/* Purpose: Informs the master a new connection was established */
/* with a slave													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Master_Connection_Complete( LEEnhancedConnectionComplete* ConnCpltData )
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
