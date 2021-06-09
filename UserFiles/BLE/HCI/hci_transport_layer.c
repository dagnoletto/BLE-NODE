

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
static void Set_Number_Of_HCI_Command_Packets( uint8_t Num_HCI_Cmd_Packets );
static uint8_t Check_Command_Packets_Available( void );
static void Decrement_HCI_Command_Packets( void );
static void Clear_Command_CallBack( HCI_COMMAND_OPCODE OpCode );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
/* Assumes the controller can handle at least one HCI Command
 * before the first Set_Number_Of_HCI_Command_Packets() is called */
static uint8_t Num_HCI_Command_Packets = 1;


/* Command callback (not all commands have callback). At least one
 * callback per command is available. That means only one command
 * type can be sent at any given time. That is generally ok, but
 * if more than one command of the same type wants to be enqueued,
 * the command callback must be modified to become and array */
#define CMD_CALLBACK_NAME(OpcodeVal) OpcodeVal ## _CMD_CALLBACK


static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_DISCONNECT);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_READ_REMOTE_VERSION_INFORMATION);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_SET_EVENT_MASK);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_CLEAR_WHITE_LIST);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_READ_TRANSMIT_POWER_LEVEL);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_RESET);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_READ_LOCAL_VERSION_INFORMATION);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_READ_LOCAL_SUPPORTED_COMMANDS);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_READ_LOCAL_SUPPORTED_FEATURES);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_READ_BD_ADDR);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_READ_RSSI);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_EVENT_MASK);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_READ_BUFFER_SIZE);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_READ_LOCAL_SUPPORTED_FEATURES);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_RANDOM_ADDRESS);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_PARAMETERS);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_DATA);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_RESPONSE_DATA);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_ENABLE);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_PARAMETERS);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_ENABLE);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_CREATE_CONNECTION);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_CREATE_CONNECTION_CANCEL);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_READ_WHITE_LIST_SIZE);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_ADD_DEVICE_TO_WHITE_LIST);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_CONNECTION_UPDATE);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_READ_CHANNEL_MAP);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_READ_REMOTE_FEATURES);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_ENCRYPT);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_RAND);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_ENABLE_ENCRYPTION);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_LONG_TERM_KEY_REQUEST_REPLY);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_READ_SUPPORTED_STATES);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_RECEIVER_TEST_V1);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_TRANSMITTER_TEST_V1);
static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_TEST_END);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_GET_FW_BUILD_NUMBER);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_WRITE_CONFIG_DATA);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_READ_CONFIG_DATA);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_SET_TX_POWER_LEVEL);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_DEVICE_STANDBY);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_TONE_START);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_TONE_STOP);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_GET_LINK_STATUS);
static CMD_CALLBACK CMD_CALLBACK_NAME(VS_ACI_HAL_GET_ANCHOR_PERIOD);


/****************************************************************/
/* Set_Number_Of_HCI_Command_Packets()         					*/
/* Location: 					 								*/
/* Purpose: Set the Num_HCI_Command_Packets	used for commands	*/
/* flow control (see Page 1882 Core_v5.2).						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Set_Number_Of_HCI_Command_Packets( uint8_t Num_HCI_Cmd_Packets )
{
	/* This assignment must not be interrupted */
	EnterCritical();

	Num_HCI_Command_Packets = Num_HCI_Cmd_Packets;

	ExitCritical();
}


/****************************************************************/
/* Check_Command_Packets_Available()         					*/
/* Location: 					 								*/
/* Purpose: Verify if controller can handle more commands.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Command_Packets_Available( void )
{
	uint8_t status;

	EnterCritical();

	/* If there is room for commands */
	status = Num_HCI_Command_Packets ? TRUE : FALSE;

	ExitCritical();

	return (status);
}


/****************************************************************/
/* Decrement_HCI_Command_Packets()         						*/
/* Location: 					 								*/
/* Purpose: Decrement available HCI packets.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Decrement_HCI_Command_Packets( void )
{
	EnterCritical();

	if ( Num_HCI_Command_Packets )
	{
		Num_HCI_Command_Packets--;
	}

	ExitCritical();
}


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
		TransferCallBack CallBack, CMD_CALLBACK* CmdCallBack)
{
	FRAME_ENQUEUE_STATUS Status;
	HCI_SERIAL_COMMAND_PCKT* CmdPacket = (HCI_SERIAL_COMMAND_PCKT*)( DataPtr ); /* Assume it is a command packet */
	CMD_CALLBACK* CallBackPtr;

	int8_t Ntries = 3;

	TRANSFER_DESCRIPTOR TransferDesc;

	TransferDesc.CallBack = CallBack;
	TransferDesc.CallBackMode = CallBackMode;
	TransferDesc.DataPtr = (uint8_t*)DataPtr;
	TransferDesc.DataSize = DataSize;

	if ( CmdPacket->PacketType == HCI_COMMAND_PACKET )
	{
		if( !Check_Command_Packets_Available() )
		{
			return (FALSE);
		}else
		{
			CallBackPtr = Get_Command_CallBack( CmdPacket->CmdPacket.OpCode );
			if( CallBackPtr != NULL )
			{
				if( CallBackPtr->Status )
				{
					return (FALSE); /* This command was already loaded and is waiting for conclusion */
				}
			}
		}
	}

	do
	{
		/* As the buffer could be blocked awaiting another operation, you should try some times. */
		Status = Bluenrg_Add_Frame( &TransferDesc, 7 );
		Ntries--;
	}while( ( Status.EnqueuedAtIndex < 0 ) &&  ( Ntries > 0 ) );


	if( Status.EnqueuedAtIndex >= 0 )
	{
		if( CmdPacket->PacketType == HCI_COMMAND_PACKET ) //TODO: verificar como as que não tem opcode podem ser chamadas por aqui
		{
			Decrement_HCI_Command_Packets(  );

			if( CallBackPtr != NULL )
			{
				CallBackPtr->Status = TRUE;
				CallBackPtr->CmdCompleteCallBack = CmdCallBack->CmdCompleteCallBack;
				CallBackPtr->CmdStatusCallBack = CmdCallBack->CmdStatusCallBack;
			}
		}

		/* This is the first successfully enqueued write message, so, request transmission */
		if( ( Status.EnqueuedAtIndex == 0 ) && ( Status.NumberOfEnqueuedFrames == 1 ) )
		{
			/* Request transmission */
			Request_Frame();
		}

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

	HCI_PACKET_TYPE PacketType = *DataPtr;

	if( Status == TRANSFER_DONE )
	{
		switch( PacketType )
		{
		case HCI_ACL_DATA_PACKET: {
			HCI_ACL_DATA_PCKT_HEADER Header = *( ( HCI_ACL_DATA_PCKT_HEADER* )( DataPtr + 1 ) );
			HCI_Controller_ACL_Data( Header, (uint8_t*)( DataPtr + 5 ) );
		}
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
			/*---------- DISCONNECTION_COMPLETE_EVT ------------*//* Page 2296 Core_v5.2 */
			case DISCONNECTION_COMPLETE:
				HCI_Disconnection_Complete( EventPacketPtr->Event_Parameter[0],
						( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1],
						EventPacketPtr->Event_Parameter[3] );
				break;

				/*---------- ENCRYPTION_CHANGE_EVT ------------*//* Page 2299 Core_v5.2 */
			case ENCRYPTION_CHANGE:
				HCI_Encryption_Change( EventPacketPtr->Event_Parameter[0],
						( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1],
						EventPacketPtr->Event_Parameter[3] );
				break;

				/*---------- READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVT ------------*//* Page 2304 Core_v5.2 */
			case READ_REMOTE_VERSION_INFORMATION_COMPLETE:
				HCI_Read_Remote_Version_Information_Complete( EventPacketPtr->Event_Parameter[0],
						( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1],
						EventPacketPtr->Event_Parameter[3],
						( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
						( EventPacketPtr->Event_Parameter[7] << 8 ) | EventPacketPtr->Event_Parameter[6] );
				break;

				/*---------- COMMAND_COMPLETE_EVT ------------*//* Page 2308 Core_v5.2 */
			case COMMAND_COMPLETE: {
				OpCode.Val = ( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1];
				uint8_t Num_HCI_Command_Packets = EventPacketPtr->Event_Parameter[0];
				Set_Number_Of_HCI_Command_Packets( Num_HCI_Command_Packets );

				CMD_CALLBACK* CmdCallBack = Get_Command_CallBack( OpCode );
				void* CmdCallBackFun = NULL; /* Function pointer */

				if( CmdCallBack != NULL )
				{
					CmdCallBackFun = CmdCallBack->CmdCompleteCallBack;
					CmdCallBack->Status = FALSE;
					CmdCallBack->CmdCompleteCallBack = NULL;
					CmdCallBack->CmdStatusCallBack = NULL;
				}

				switch( OpCode.Val )
				{
				case HCI_SET_EVENT_MASK:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_RESET:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_READ_TRANSMIT_POWER_LEVEL:
					if( CmdCallBackFun != NULL )
					{
						( (TxPwrLvlComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
								EventPacketPtr->Event_Parameter[6] );
					}
					break;

				case HCI_READ_LOCAL_VERSION_INFORMATION:
					if( CmdCallBackFun != NULL )
					{
						( (ReadLocalVerInfoComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4],
								( EventPacketPtr->Event_Parameter[6]  << 8 ) | EventPacketPtr->Event_Parameter[5], EventPacketPtr->Event_Parameter[7],
								( EventPacketPtr->Event_Parameter[9]  << 8 ) | EventPacketPtr->Event_Parameter[8],
								( EventPacketPtr->Event_Parameter[11] << 8 ) | EventPacketPtr->Event_Parameter[10] );
					}
					break;

				case HCI_READ_LOCAL_SUPPORTED_COMMANDS:
					if( CmdCallBackFun != NULL )
					{
						( (ReadLocalSupCmdsComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], (SUPPORTED_COMMANDS*)( &(EventPacketPtr->Event_Parameter[4]) ) );
					}
					break;

				case HCI_READ_LOCAL_SUPPORTED_FEATURES:
					if( CmdCallBackFun != NULL )
					{
						( (ReadLocalSupFeaturesComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], (SUPPORTED_FEATURES*)( &(EventPacketPtr->Event_Parameter[4]) ) );
					}
					break;

				case HCI_READ_BD_ADDR:
					if( CmdCallBackFun != NULL )
					{
						( (ReadBDADDRComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], (BD_ADDR_TYPE*)(&(EventPacketPtr->Event_Parameter[4])) );
					}
					break;

				case HCI_READ_RSSI:
					if( CmdCallBackFun != NULL )
					{
						( (ReadRSSIComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
								EventPacketPtr->Event_Parameter[6] );
					}
					break;

				case HCI_LE_SET_EVENT_MASK:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_READ_BUFFER_SIZE:
					if( CmdCallBackFun != NULL )
					{
						( (LEReadBufferSizeComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
								EventPacketPtr->Event_Parameter[6] );
					}
					break;

				case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES:
					if( CmdCallBackFun != NULL )
					{
						( (LEReadLocalSuppFeaturesComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], (LE_SUPPORTED_FEATURES*)( &(EventPacketPtr->Event_Parameter[4]) ) );
					}
					break;

				case HCI_LE_SET_RANDOM_ADDRESS:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_SET_ADVERTISING_PARAMETERS:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER:
					if( CmdCallBackFun != NULL )
					{
						( (LEReadAdvPhyChannelTxPowerComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4] );
					}
					break;

				case HCI_LE_SET_ADVERTISING_DATA:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_SET_SCAN_RESPONSE_DATA:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_SET_ADVERTISING_ENABLE:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_SET_SCAN_PARAMETERS:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_SET_SCAN_ENABLE:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_CREATE_CONNECTION_CANCEL:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_READ_WHITE_LIST_SIZE:
					if( CmdCallBackFun != NULL )
					{
						( (LEReadWhiteListSizeComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4] );
					}
					break;

				case HCI_LE_CLEAR_WHITE_LIST:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_ADD_DEVICE_TO_WHITE_LIST:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_READ_CHANNEL_MAP:
					if( CmdCallBackFun != NULL )
					{
						( ( LEReadChannelMapComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4], (CHANNEL_MAP*)(&(EventPacketPtr->Event_Parameter[6])) );
					}
					break;

				case HCI_LE_ENCRYPT:
					if( CmdCallBackFun != NULL )
					{
						( (LEEncryptComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]) );
					}
					break;

				case HCI_LE_RAND:
					if( CmdCallBackFun != NULL )
					{
						( (LERandComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]) );
					}
					break;

				case HCI_LE_LONG_TERM_KEY_REQUEST_REPLY:
					if( CmdCallBackFun != NULL )
					{
						( (LELongTermKeyRqtReplyComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
					}
					break;

				case HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY:
					if( CmdCallBackFun != NULL )
					{
						( (LELongTermKeyRqtNegReplyComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
					}
					break;

				case HCI_LE_READ_SUPPORTED_STATES:
					if( CmdCallBackFun != NULL )
					{
						( (LEReadSupportedStatesComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], (SUPPORTED_LE_STATES*)(&(EventPacketPtr->Event_Parameter[4])) );
					}
					break;

				case HCI_LE_RECEIVER_TEST_V1:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_TRANSMITTER_TEST_V1:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case HCI_LE_TEST_END:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case VS_ACI_HAL_GET_FW_BUILD_NUMBER:
					if( CmdCallBackFun != NULL )
					{
						( (HalGetFwBuildNumberComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
					}
					break;

				case VS_ACI_HAL_WRITE_CONFIG_DATA:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case VS_ACI_HAL_READ_CONFIG_DATA:
					if( CmdCallBackFun != NULL )
					{
						( (HalReadConfigDataComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]), EventPacketPtr->Parameter_Total_Length - 4 );
					}
					break;

				case VS_ACI_HAL_SET_TX_POWER_LEVEL:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case VS_ACI_HAL_DEVICE_STANDBY:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER:
					if( CmdCallBackFun != NULL )
					{
						uint32_t PacketCounter = ( EventPacketPtr->Event_Parameter[7] << 24 ) | ( EventPacketPtr->Event_Parameter[6] << 16 ) |
								( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4];
						( (HalLETxTestPacketNumberComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], PacketCounter );
					}
					break;

				case VS_ACI_HAL_TONE_START:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case VS_ACI_HAL_TONE_STOP:
					if( CmdCallBackFun != NULL )
					{
						( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
					}
					break;

				case VS_ACI_HAL_GET_LINK_STATUS:
					if( CmdCallBackFun != NULL )
					{
						( (HalGetLinkStatusComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]), &(EventPacketPtr->Event_Parameter[12]) );
					}
					break;

				case VS_ACI_HAL_GET_ANCHOR_PERIOD:
					if( CmdCallBackFun != NULL )
					{
						uint32_t AnchorInterval = ( EventPacketPtr->Event_Parameter[7] << 24 ) | ( EventPacketPtr->Event_Parameter[6] << 16 ) |
								( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4];

						uint32_t Maxslot = ( EventPacketPtr->Event_Parameter[11] << 24 ) | ( EventPacketPtr->Event_Parameter[10] << 16 ) |
								( EventPacketPtr->Event_Parameter[9] << 8 ) | EventPacketPtr->Event_Parameter[8];

						( (HalGetAnchorPeriodComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3], AnchorInterval, Maxslot );
					}
					break;

				default:
					HCI_Command_Complete( Num_HCI_Command_Packets, OpCode, &( EventPacketPtr->Event_Parameter[3] ) );
					break;

				}
			}
			break;



			/*---------- COMMAND_STATUS_EVT --------------*//* Page 2310 Core_v5.2 */
			case COMMAND_STATUS: {
				uint8_t Num_HCI_Command_Packets = EventPacketPtr->Event_Parameter[1];
				Set_Number_Of_HCI_Command_Packets( Num_HCI_Command_Packets );

				OpCode.Val = ( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2];
				CMD_CALLBACK* CmdCallBack = Get_Command_CallBack( OpCode );

				CONTROLLER_ERROR_CODES Status = EventPacketPtr->Event_Parameter[0];
				void* CmdCallBackFun = ( CmdCallBack != NULL ) ? CmdCallBack->CmdStatusCallBack : NULL; /* Function pointer */

				if( CmdCallBackFun != NULL )
				{
					( (DefCmdStatus)CmdCallBackFun )( Status );
				}else
				{
					HCI_Command_Status( Status, Num_HCI_Command_Packets, OpCode );
				}
			}
			break;

			/*---------- HARDWARE_ERROR_EVT ------------*//* Page 2312 Core_v5.2 */
			case HARDWARE_ERROR:
				HCI_Hardware_Error( EventPacketPtr->Event_Parameter[0] );
				break;

				/*---------- NUMBER_OF_COMPLETED_PACKETS_EVT ------------*//* Page 2315 Core_v5.2 */
			case NUMBER_OF_COMPLETED_PACKETS: {
				uint16_t Offset = EventPacketPtr->Event_Parameter[0] * 2;

				HCI_Number_Of_Completed_Packets( EventPacketPtr->Event_Parameter[0], (uint16_t*)( &EventPacketPtr->Event_Parameter[1] ), (uint16_t*)( &EventPacketPtr->Event_Parameter[Offset + 1] ) );
			}
			break;

			/*---------- DATA_BUFFER_OVERFLOW_EVT ------------*//* Page 2325 Core_v5.2 */
			case DATA_BUFFER_OVERFLOW:
				HCI_Data_Buffer_Overflow( EventPacketPtr->Event_Parameter[0] );
				break;

				/*---------- ENCRYPTION_KEY_REFRESH_COMPLETE_EVT ------------*//* Page 2349 Core_v5.2 */
			case ENCRYPTION_KEY_REFRESH_COMPLETE:
				HCI_Encryption_Key_Refresh_Complete( EventPacketPtr->Event_Parameter[0], ( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1] );
				break;

				/*---------- LE_META_EVT ------------*//* Page 2379 Core_v5.2 */
			case LE_META:
				switch( EventPacketPtr->Event_Parameter[0] )
				{

				case LE_CONNECTION_COMPLETE:
					HCI_LE_Connection_Complete( EventPacketPtr->Event_Parameter[1], ( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2],
							EventPacketPtr->Event_Parameter[4], EventPacketPtr->Event_Parameter[5], (BD_ADDR_TYPE*)(&(EventPacketPtr->Event_Parameter[6])), ( EventPacketPtr->Event_Parameter[13] << 8 ) | EventPacketPtr->Event_Parameter[12],
							( EventPacketPtr->Event_Parameter[15] << 8 ) | EventPacketPtr->Event_Parameter[14], ( EventPacketPtr->Event_Parameter[17] << 8 ) | EventPacketPtr->Event_Parameter[16],
							EventPacketPtr->Event_Parameter[18] );
					break;

				case LE_ADVERTISING_REPORT: {
					uint8_t Num_Reports = EventPacketPtr->Event_Parameter[1];
					uint16_t Data_Length_OffSet = ( Num_Reports * 8 ) + 2;
					uint16_t Number_Of_Data_Bytes = 0;

					for( uint8_t i = 0; i < Num_Reports; i++ )
					{
						Number_Of_Data_Bytes += EventPacketPtr->Event_Parameter[Data_Length_OffSet + i];
					}

					HCI_LE_Advertising_Report( Num_Reports, &(EventPacketPtr->Event_Parameter[2]), &(EventPacketPtr->Event_Parameter[Num_Reports + 2]), (BD_ADDR_TYPE*)(&(EventPacketPtr->Event_Parameter[ ( Num_Reports * 2 ) + 2 ]) ),
							&(EventPacketPtr->Event_Parameter[Data_Length_OffSet]), &( EventPacketPtr->Event_Parameter[Data_Length_OffSet + Num_Reports] ), (int8_t*)(&(EventPacketPtr->Event_Parameter[( Num_Reports * 9 ) + Number_Of_Data_Bytes + 2])) );
				}
				break;

				case LE_CONNECTION_UPDATE_COMPLETE:
					HCI_LE_Connection_Update_Complete( EventPacketPtr->Event_Parameter[1],
							( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2],
							( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
							( EventPacketPtr->Event_Parameter[7] << 8 ) | EventPacketPtr->Event_Parameter[6],
							( EventPacketPtr->Event_Parameter[9] << 8 ) | EventPacketPtr->Event_Parameter[8] );
					break;

				case LE_READ_REMOTE_FEATURES_COMPLETE:
					HCI_LE_Read_Remote_Features_Complete( EventPacketPtr->Event_Parameter[1],
							( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2],
							(LE_SUPPORTED_FEATURES*)( &(EventPacketPtr->Event_Parameter[4]) ) );
					break;

				case LE_LONG_TERM_KEY_REQUEST:
					HCI_LE_Long_Term_Key_Request( ( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1],
							&(EventPacketPtr->Event_Parameter[3]),
							( EventPacketPtr->Event_Parameter[12] << 8 ) | EventPacketPtr->Event_Parameter[11]);
					break;

				}
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
	}else if( PacketType == HCI_EVENT_PACKET )
	{
		HCI_EVENT_PCKT* EventPacketPtr = ( HCI_EVENT_PCKT* )( DataPtr + 1 );
		HCI_COMMAND_OPCODE OpCode;

		switch ( EventPacketPtr->Event_Code )
		{
		case COMMAND_COMPLETE:
		case COMMAND_STATUS:
			Clear_Command_CallBack( OpCode );
			break;
		}
	}
}


/****************************************************************/
/* Clear_Command_CallBack()                    		            */
/* Purpose: Clear the command callback from the Opcode.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Clear_Command_CallBack( HCI_COMMAND_OPCODE OpCode )
{
	CMD_CALLBACK* CallBackPtr = Get_Command_CallBack( OpCode );

	if( CallBackPtr != NULL )
	{
		CallBackPtr->Status = FALSE;
		CallBackPtr->CmdCompleteCallBack = NULL;
		CallBackPtr->CmdStatusCallBack = NULL;
	}
}


/****************************************************************/
/* Get_Command_CallBack()                    		            */
/* Purpose: Get the command callback pointer from the Opcode.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
CMD_CALLBACK* Get_Command_CallBack( HCI_COMMAND_OPCODE OpCode )
{
	switch ( OpCode.Val )
	{
	case HCI_DISCONNECT: return ( &CMD_CALLBACK_NAME(HCI_DISCONNECT) );

	case HCI_READ_REMOTE_VERSION_INFORMATION: return ( &CMD_CALLBACK_NAME(HCI_READ_REMOTE_VERSION_INFORMATION) );

	case HCI_SET_EVENT_MASK: return ( &CMD_CALLBACK_NAME(HCI_SET_EVENT_MASK) );

	case HCI_LE_CLEAR_WHITE_LIST: return ( &CMD_CALLBACK_NAME(HCI_LE_CLEAR_WHITE_LIST) );

	case HCI_READ_TRANSMIT_POWER_LEVEL: return ( &CMD_CALLBACK_NAME(HCI_READ_TRANSMIT_POWER_LEVEL) );

	case HCI_RESET: return ( &CMD_CALLBACK_NAME(HCI_RESET) );

	case HCI_READ_LOCAL_VERSION_INFORMATION: return ( &CMD_CALLBACK_NAME(HCI_READ_LOCAL_VERSION_INFORMATION) );

	case HCI_READ_LOCAL_SUPPORTED_COMMANDS: return ( &CMD_CALLBACK_NAME(HCI_READ_LOCAL_SUPPORTED_COMMANDS) );

	case HCI_READ_LOCAL_SUPPORTED_FEATURES: return ( &CMD_CALLBACK_NAME(HCI_READ_LOCAL_SUPPORTED_FEATURES) );

	case HCI_READ_BD_ADDR: return ( &CMD_CALLBACK_NAME(HCI_READ_BD_ADDR) );

	case HCI_READ_RSSI: return ( &CMD_CALLBACK_NAME(HCI_READ_RSSI) );

	case HCI_LE_SET_EVENT_MASK: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_EVENT_MASK) );

	case HCI_LE_READ_BUFFER_SIZE: return ( &CMD_CALLBACK_NAME(HCI_LE_READ_BUFFER_SIZE) );

	case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES: return ( &CMD_CALLBACK_NAME(HCI_LE_READ_LOCAL_SUPPORTED_FEATURES) );

	case HCI_LE_SET_RANDOM_ADDRESS: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_RANDOM_ADDRESS) );

	case HCI_LE_SET_ADVERTISING_PARAMETERS: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_PARAMETERS) );

	case HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER: return ( &CMD_CALLBACK_NAME(HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER) );

	case HCI_LE_SET_ADVERTISING_DATA: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_DATA) );

	case HCI_LE_SET_SCAN_RESPONSE_DATA: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_RESPONSE_DATA) );

	case HCI_LE_SET_ADVERTISING_ENABLE: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_ENABLE) );

	case HCI_LE_SET_SCAN_PARAMETERS: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_PARAMETERS) );

	case HCI_LE_SET_SCAN_ENABLE: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_ENABLE) );

	case HCI_LE_CREATE_CONNECTION: return ( &CMD_CALLBACK_NAME(HCI_LE_CREATE_CONNECTION) );

	case HCI_LE_CREATE_CONNECTION_CANCEL: return ( &CMD_CALLBACK_NAME(HCI_LE_CREATE_CONNECTION_CANCEL) );

	case HCI_LE_READ_WHITE_LIST_SIZE: return ( &CMD_CALLBACK_NAME(HCI_LE_READ_WHITE_LIST_SIZE) );

	case HCI_LE_ADD_DEVICE_TO_WHITE_LIST: return ( &CMD_CALLBACK_NAME(HCI_LE_ADD_DEVICE_TO_WHITE_LIST) );

	case HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST: return ( &CMD_CALLBACK_NAME(HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST) );

	case HCI_LE_CONNECTION_UPDATE: return ( &CMD_CALLBACK_NAME(HCI_LE_CONNECTION_UPDATE) );

	case HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION: return ( &CMD_CALLBACK_NAME(HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION) );

	case HCI_LE_READ_CHANNEL_MAP: return ( &CMD_CALLBACK_NAME(HCI_LE_READ_CHANNEL_MAP) );

	case HCI_LE_READ_REMOTE_FEATURES: return ( &CMD_CALLBACK_NAME(HCI_LE_READ_REMOTE_FEATURES) );

	case HCI_LE_ENCRYPT: return ( &CMD_CALLBACK_NAME(HCI_LE_ENCRYPT) );

	case HCI_LE_RAND: return ( &CMD_CALLBACK_NAME(HCI_LE_RAND) );

	case HCI_LE_ENABLE_ENCRYPTION: return ( &CMD_CALLBACK_NAME(HCI_LE_ENABLE_ENCRYPTION) );

	case HCI_LE_LONG_TERM_KEY_REQUEST_REPLY: return ( &CMD_CALLBACK_NAME(HCI_LE_LONG_TERM_KEY_REQUEST_REPLY) );

	case HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY: return ( &CMD_CALLBACK_NAME(HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY) );

	case HCI_LE_READ_SUPPORTED_STATES: return ( &CMD_CALLBACK_NAME(HCI_LE_READ_SUPPORTED_STATES) );

	case HCI_LE_RECEIVER_TEST_V1: return ( &CMD_CALLBACK_NAME(HCI_LE_RECEIVER_TEST_V1) );

	case HCI_LE_TRANSMITTER_TEST_V1: return ( &CMD_CALLBACK_NAME(HCI_LE_TRANSMITTER_TEST_V1) );

	case HCI_LE_TEST_END: return ( &CMD_CALLBACK_NAME(HCI_LE_TEST_END) );

	case VS_ACI_HAL_GET_FW_BUILD_NUMBER: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_GET_FW_BUILD_NUMBER) );

	case VS_ACI_HAL_WRITE_CONFIG_DATA: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_WRITE_CONFIG_DATA) );

	case VS_ACI_HAL_READ_CONFIG_DATA: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_READ_CONFIG_DATA) );

	case VS_ACI_HAL_SET_TX_POWER_LEVEL: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_SET_TX_POWER_LEVEL) );

	case VS_ACI_HAL_DEVICE_STANDBY: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_DEVICE_STANDBY) );

	case VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER) );

	case VS_ACI_HAL_TONE_START: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_TONE_START) );

	case VS_ACI_HAL_TONE_STOP: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_TONE_STOP) );

	case VS_ACI_HAL_GET_LINK_STATUS: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_GET_LINK_STATUS) );

	case VS_ACI_HAL_GET_ANCHOR_PERIOD: return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_GET_ANCHOR_PERIOD) );

	/* Not all commands have callback */
	default: return (NULL);
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
