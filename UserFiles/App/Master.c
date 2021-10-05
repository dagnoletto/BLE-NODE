

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
	static uint32_t TimeCounter = 0;

	switch( MasterStateMachine )
	{
	case CONFIG_STANDBY:
		MasterStateMachine = ( Get_BLE_State() == STANDBY_STATE ) ? CONFIG_SCANNING : CONFIG_STANDBY;
		break;

	case CONFIG_SCANNING:
		MasterStateMachine = Config_Scanner() ? SCANNING_STATE : CONFIG_SCANNING;
		break;

	case SCANNING_STATE:
		if( ( Get_BLE_State() == SCANNING_STATE ) && ( TimeBase_DelayMs( &TimeCounter, 5000, TRUE ) ) )
		{
			MasterStateMachine = CONFIG_INITIATING;
		}
		break;

	case CONFIG_INITIATING:
		MasterStateMachine = Config_Initiating() ? INITIATING_STATE : CONFIG_INITIATING;
		break;

	case INITIATING_STATE:
		if( ( Get_BLE_State() == INITIATING_STATE ) && ( TimeBase_DelayMs( &TimeCounter, 5000, TRUE ) ) )
		{
			//MasterStateMachine = CONFIG_INITIATING;
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
	Id.Peer_Identity_Address.Address = MasterPublicAddress;

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
	Scan.Privacy = TRUE;
	Scan.Role = OBSERVER;

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
	Id.Peer_Identity_Address.Address = MasterPublicAddress;

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
	Init.Peer_Address_Type = PUBLIC_DEV_ADDR;
	Init.Peer_Address = SlavePublicAddress;
	Init.Own_Address_Type = OWN_PUBLIC_DEV_ADDR;
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
void HCI_LE_Advertising_Report( uint8_t Subevent_Code, uint8_t Num_Reports, uint8_t Event_Type[], uint8_t Address_Type[], BD_ADDR_TYPE Address[],
		uint8_t Data_Length[], uint8_t Data[], int8_t RSSI[] )
{
	static uint8_t AdvData[40];

	/* Sub event code for the HCI_LE_Advertising_Report event: page 2382 Core_v5.2 */
	if( Subevent_Code == 0x02 )
	{
		ADVERTISING_REPORT Report;
		uint16_t Number_Of_Data_Bytes = 0;

		for( uint8_t i = 0; i < Num_Reports; i++ )
		{
			Report.Event_Type = Event_Type[i];
			Report.Address_Type = Address_Type[i];

			if( Report.Address_Type == PUBLIC_IDENTITY_ADDR )
			{
				HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
			}

			Report.Address = Address[i];
			Report.Data_Length = Data_Length[i];
			Report.RSSI = RSSI[i];
			Report.DataPtr = &AdvData[0];

			memcpy( Report.DataPtr, &Data[Number_Of_Data_Bytes], Report.Data_Length );

			Number_Of_Data_Bytes += Report.Data_Length;
		}
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
