

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
uint8_t Config_Advertiser(void);


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
uint8_t Config_Advertiser(void)
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
	Adv.Own_Address_Type = OWN_PUBLIC_DEV_ADDR; //OWN_RESOL_OR_PUBLIC_ADDR;
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
/* End of file	                                                */
/****************************************************************/
