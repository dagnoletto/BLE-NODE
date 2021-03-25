

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci_transport_layer.h"
#include "Bluenrg.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Num_HCI_Command_Packets; /* TODO: setar este valor e ver oque ocorre na inicialização */


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
/* HCI_Transmit()                      				            */
/* Purpose: Higher layers put messages to transmit calling		*/
/* this function. This function enqueue messages in the 		*/
/* hardware. 													*/
/* destination	    											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Transmit(void* DataPtr, uint16_t DataSize,
		TRANSFER_CALL_BACK_MODE CallBackMode,
		TransferCallBack CallBack)
{
	int8_t Ntries = 3;

	TRANSFER_DESCRIPTOR TransferDesc;

	TransferDesc.CallBack = CallBack;
	TransferDesc.CallBackMode = CallBackMode;
	TransferDesc.DataPtr = (uint8_t*)DataPtr;
	TransferDesc.DataSize = DataSize;

	/* As the buffer could be blocked awaiting another operation, you should try some times. */
	while( ( Bluenrg_Add_Frame( &TransferDesc, 7 ).EnqueuedAtIndex < 0 ) && ( Ntries > 0 ) )
	{
		Ntries--;
	}

	if( Ntries > 0 )
	{
		return (TRUE);
	}else
	{
		return (FALSE);
	}
}


/****************************************************************/
/* HCI_Receive()                      				            */
/* Purpose: All messages received from the BLE Controller call  */
/* this function where they are multiplexed to correct 			*/
/* destination	    											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_Receive(uint8_t* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	/* TODO: talvez melhorar a decodificação usando vetores indexados (look-up tables)
	 * lembre-se da linguagem P4. */
	if( Status == TRANSFER_DONE )
	{
		HCI_PACKET_TYPE PacketType = *DataPtr;

		switch( PacketType )
		{
		case HCI_ACL_DATA_PACKET:
			break;

		case HCI_SYNCHRONOUS_DATA_PACKET:
			/* Not used in this controller version */
			break;

		case HCI_EVENT_PACKET:
		{
			HCI_EVENT_PCKT* EventPacketPtr = ( HCI_EVENT_PCKT* )( DataPtr + 1 );
			HCI_COMMAND_OPCODE OpCode;

			switch( EventPacketPtr->Event_Code )
			{

			/*---------- HARDWARE_ERROR_EVT ------------*//* Page 2312 Core_v5.2 */
			case HARDWARE_ERROR:
				HCI_Hardware_Error( EventPacketPtr->Event_Parameter[0] );
				break;

				/*---------- COMMAND_COMPLETE_EVT ------------*//* Page 2308 Core_v5.2 */
			case COMMAND_COMPLETE: {
				OpCode.Val = ( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1];
				Num_HCI_Command_Packets = EventPacketPtr->Event_Parameter[0];

				switch( OpCode.Val )
				{
				case HCI_SET_EVENT_MASK:
					HCI_Set_Event_Mask_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_RESET:
					HCI_Reset_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_READ_TRANSMIT_POWER_LEVEL:
					HCI_Read_Transmit_Power_Level_Complete( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
							EventPacketPtr->Event_Parameter[6] );
					break;

				case HCI_READ_LOCAL_VERSION_INFORMATION:
					HCI_Read_Local_Version_Information_Complete( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4],
							( EventPacketPtr->Event_Parameter[6]  << 8 ) | EventPacketPtr->Event_Parameter[5], EventPacketPtr->Event_Parameter[7],
							( EventPacketPtr->Event_Parameter[9]  << 8 ) | EventPacketPtr->Event_Parameter[8],
							( EventPacketPtr->Event_Parameter[11] << 8 ) | EventPacketPtr->Event_Parameter[10] );
					break;

				case HCI_READ_LOCAL_SUPPORTED_COMMANDS: {
					SUPPORTED_COMMANDS Supported_Cmds;
					/* Must be cleared because the Controller may send a structure that is smaller than the data type */
					memset( &Supported_Cmds, 0, sizeof(Supported_Cmds) );

					uint16_t SizeToRead = EventPacketPtr->Parameter_Total_Length - 4;
					if( SizeToRead > sizeof(Supported_Cmds) )
					{
						SizeToRead = sizeof(Supported_Cmds);
					}

					memcpy( &Supported_Cmds, &(EventPacketPtr->Event_Parameter[4]), SizeToRead );
					HCI_Read_Local_Supported_Commands_Complete( EventPacketPtr->Event_Parameter[3], &Supported_Cmds );
				}
				break;

				case HCI_READ_LOCAL_SUPPORTED_FEATURES: {
					SUPPORTED_FEATURES LMP_Features;
					/* Must be cleared because the Controller may send a structure that is smaller than the data type */
					memset( &LMP_Features, 0, sizeof(LMP_Features) );

					uint16_t SizeToRead = EventPacketPtr->Parameter_Total_Length - 4;
					if( SizeToRead > sizeof(LMP_Features) )
					{
						SizeToRead = sizeof(LMP_Features);
					}

					memcpy( &LMP_Features, &(EventPacketPtr->Event_Parameter[4]), SizeToRead );

					HCI_Read_Local_Supported_Features_Complete( EventPacketPtr->Event_Parameter[3], &LMP_Features );
				}
				break;

				case HCI_READ_BD_ADDR: {
					BD_ADDR_TYPE BD_ADDR;

					memcpy( &BD_ADDR, &(EventPacketPtr->Event_Parameter[4]), 6 );

					HCI_Read_BD_ADDR_Complete( EventPacketPtr->Event_Parameter[3], BD_ADDR );
				}
				break;

				case HCI_READ_RSSI:
					HCI_Read_RSSI_Complete( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
							EventPacketPtr->Event_Parameter[6] );
					break;

				case HCI_LE_SET_EVENT_MASK:
					HCI_LE_Set_Event_Mask_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_READ_BUFFER_SIZE:
					HCI_LE_Read_Buffer_Size_Complete( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
							EventPacketPtr->Event_Parameter[6] );
					break;

				case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES: {
					LE_SUPPORTED_FEATURES LE_Features;
					/* Must be cleared because the Controller may send a structure that is smaller than the data type */
					memset( &LE_Features, 0, sizeof(LE_Features) );

					uint16_t SizeToRead = EventPacketPtr->Parameter_Total_Length - 4;
					if( SizeToRead > sizeof(LE_Features) )
					{
						SizeToRead = sizeof(LE_Features);
					}

					memcpy( &LE_Features, &(EventPacketPtr->Event_Parameter[4]), SizeToRead );

					HCI_LE_Read_Local_Supported_Features_Complete( EventPacketPtr->Event_Parameter[3], &LE_Features );
				}
				break;

				case HCI_LE_SET_RANDOM_ADDRESS:
					HCI_LE_Set_Random_Address_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_SET_ADVERTISING_PARAMETERS:
					HCI_LE_Set_Advertising_Parameters_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER:
					HCI_LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4] );
					break;

				case HCI_LE_SET_ADVERTISING_DATA:
					HCI_LE_Set_Advertising_Data_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_SET_SCAN_RESPONSE_DATA:
					HCI_LE_Set_Scan_Response_Data_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_SET_ADVERTISING_ENABLE:
					HCI_LE_Set_Advertising_Enable_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_SET_SCAN_PARAMETERS:
					HCI_LE_Set_Scan_Parameters_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_SET_SCAN_ENABLE:
					HCI_LE_Set_Scan_Enable_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_CREATE_CONNECTION_CANCEL:
					HCI_LE_Create_Connection_Cancel_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_READ_WHITE_LIST_SIZE:
					HCI_LE_Read_White_List_Size_Complete( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4] );
					break;

				case HCI_LE_CLEAR_WHITE_LIST:
					HCI_LE_Clear_White_List_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case VS_ACI_HAL_GET_FW_BUILD_NUMBER:
					ACI_Hal_Get_Fw_Build_Number_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
					break;

				case VS_ACI_HAL_WRITE_CONFIG_DATA:
					ACI_Hal_Write_Config_Data_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3] );
					break;

				case VS_ACI_HAL_READ_CONFIG_DATA:
					ACI_Hal_Read_Config_Data_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]), EventPacketPtr->Parameter_Total_Length - 4 );
					break;

				case VS_ACI_HAL_SET_TX_POWER_LEVEL:
					ACI_Hal_Set_Tx_Power_Level_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3] );
					break;

				case VS_ACI_HAL_DEVICE_STANDBY:
					ACI_Hal_Device_Standby_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3] );
					break;

				case VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER: {
					uint32_t PacketCounter = ( EventPacketPtr->Event_Parameter[7] << 24 ) | ( EventPacketPtr->Event_Parameter[6] << 16 ) |
							( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4];

					ACI_Hal_LE_Tx_Test_Packet_Number_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3], PacketCounter );
				}
				break;

				case VS_ACI_HAL_TONE_START:
					ACI_Hal_Tone_Start_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3] );
					break;

				case VS_ACI_HAL_TONE_STOP:
					ACI_Hal_Tone_Stop_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3] );
					break;

				case VS_ACI_HAL_GET_LINK_STATUS:
					ACI_Hal_Get_Link_Status_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]), &(EventPacketPtr->Event_Parameter[12]) );
					break;

				case VS_ACI_HAL_GET_ANCHOR_PERIOD: {
					uint32_t AnchorInterval = ( EventPacketPtr->Event_Parameter[7] << 24 ) | ( EventPacketPtr->Event_Parameter[6] << 16 ) |
							( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4];

					uint32_t Maxslot = ( EventPacketPtr->Event_Parameter[11] << 24 ) | ( EventPacketPtr->Event_Parameter[10] << 16 ) |
							( EventPacketPtr->Event_Parameter[9] << 8 ) | EventPacketPtr->Event_Parameter[8];

					ACI_Hal_Get_Anchor_Period_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3], AnchorInterval, Maxslot );
				}
				break;
				}}
			break;



			/*---------- COMMAND_STATUS_EVT --------------*//* Page 2310 Core_v5.2 */
			case COMMAND_STATUS: {
				OpCode.Val = ( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2];
				Num_HCI_Command_Packets = EventPacketPtr->Event_Parameter[1];

				switch( OpCode.Val )
				{
				case HCI_DISCONNECT:
					HCI_Disconnect_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_READ_REMOTE_VERSION_INFORMATION:
					HCI_Read_Remote_Version_Information_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_SET_EVENT_MASK:
					HCI_Set_Event_Mask_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_RESET:
					HCI_Reset_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_READ_TRANSMIT_POWER_LEVEL:
					HCI_Read_Transmit_Power_Level_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_READ_LOCAL_VERSION_INFORMATION:
					HCI_Read_Local_Version_Information_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_READ_LOCAL_SUPPORTED_COMMANDS:
					HCI_Read_Local_Supported_Commands_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_READ_LOCAL_SUPPORTED_FEATURES:
					HCI_Read_Local_Supported_Features_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_READ_BD_ADDR:
					HCI_Read_BD_ADDR_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_READ_RSSI:
					HCI_Read_RSSI_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_EVENT_MASK:
					HCI_LE_Set_Event_Mask_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_READ_BUFFER_SIZE:
					HCI_LE_Read_Buffer_Size_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES:
					HCI_LE_Read_Local_Supported_Features_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_RANDOM_ADDRESS:
					HCI_LE_Set_Random_Address_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_ADVERTISING_PARAMETERS:
					HCI_LE_Set_Advertising_Parameters_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER:
					HCI_LE_Read_Advertising_Physical_Channel_Tx_Power_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_ADVERTISING_DATA:
					HCI_LE_Set_Advertising_Data_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_SCAN_RESPONSE_DATA:
					HCI_LE_Set_Scan_Response_Data_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_ADVERTISING_ENABLE:
					HCI_LE_Set_Advertising_Enable_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_SCAN_PARAMETERS:
					HCI_LE_Set_Scan_Parameters_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_SCAN_ENABLE:
					HCI_LE_Set_Scan_Enable_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_CREATE_CONNECTION:
					HCI_LE_Create_Connection_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_CREATE_CONNECTION_CANCEL:
					HCI_LE_Create_Connection_Cancel_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_READ_WHITE_LIST_SIZE:
					HCI_LE_Read_White_List_Size_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_CLEAR_WHITE_LIST:
					HCI_LE_Clear_White_List_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case VS_ACI_HAL_GET_FW_BUILD_NUMBER:
					ACI_Hal_Get_Fw_Build_Number_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0], 0 );
					break;

				case VS_ACI_HAL_WRITE_CONFIG_DATA:
					ACI_Hal_Write_Config_Data_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0] );
					break;

				case VS_ACI_HAL_READ_CONFIG_DATA:
					ACI_Hal_Read_Config_Data_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0], NULL, 0 );
					break;

				case VS_ACI_HAL_SET_TX_POWER_LEVEL:
					ACI_Hal_Set_Tx_Power_Level_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0] );
					break;

				case VS_ACI_HAL_DEVICE_STANDBY:
					ACI_Hal_Device_Standby_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0] );
					break;

				case VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER:
					ACI_Hal_LE_Tx_Test_Packet_Number_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0], 0 );
					break;

				case VS_ACI_HAL_TONE_START:
					ACI_Hal_Tone_Start_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0] );
					break;

				case VS_ACI_HAL_TONE_STOP:
					ACI_Hal_Tone_Stop_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0] );
					break;

				case VS_ACI_HAL_GET_LINK_STATUS:
					ACI_Hal_Get_Link_Status_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0], NULL, NULL );
					break;

				case VS_ACI_HAL_GET_ANCHOR_PERIOD:
					ACI_Hal_Get_Anchor_Period_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0], 0, 0 );
					break;
				}}
			break;




			/*--------- VENDOR_SPECIFIC_EVT -------------*/
			case VENDOR_SPECIFIC: {
				ECODE_Struct Ecode;
				Ecode.ECODE = EventPacketPtr->Event_Parameter[1] << 8 | EventPacketPtr->Event_Parameter[0];

				switch( Ecode.ECODE )
				{
				case EVT_BLUE_INITIALIZED_EVENT_CODE:
					ACI_Blue_Initialized_Event( EventPacketPtr->Event_Parameter[2] );
					break;

				case EVT_BLUE_LOST_EVENTS_CODE:
					ACI_Blue_Lost_Event( &(EventPacketPtr->Event_Parameter[2]) );
					break;

				case FAULT_DATA_EVENT_CODE: {
					uint32_t Registers[9];
					int8_t Index;

					for( int8_t i = 0; i < ( sizeof(Registers)/sizeof(uint32_t) ); i++ )
					{
						Index = i * 4;
						Registers[i] = ( EventPacketPtr->Event_Parameter[6 + Index] << 24 ) | ( EventPacketPtr->Event_Parameter[5 + Index] << 16 ) |
								( EventPacketPtr->Event_Parameter[4 + Index] << 8 ) | EventPacketPtr->Event_Parameter[3 + Index];
					}

					ACI_Fault_Data_Event( EventPacketPtr->Event_Parameter[2], &Registers[0], EventPacketPtr->Event_Parameter[39], &(EventPacketPtr->Event_Parameter[40]) );
				}
				break;
				}
			}
			break;

			default:
				break;
			}
		}
		break;

		case HCI_ISO_DATA_PACKET:
			/* Not used in this controller version */
			break;

		default: /* The controller shall not issue HCI_COMMAND_PACKET packet */
			break;
		}
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
