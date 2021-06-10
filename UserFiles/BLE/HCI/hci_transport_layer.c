

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

static void Command_Status_Handler( HCI_EVENT_PCKT* EventPacketPtr );
static void Command_Complete_Handler( TRANSFER_STATUS Status, HCI_EVENT_PCKT* EventPacketPtr );

static void Command_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Read_Transmit_Power_Level_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Read_Local_Version_Information_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Read_Local_Supported_Commands_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Read_Local_Supported_Features_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Read_BD_ADDR_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Read_RSSI_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Read_Buffer_Size_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Read_Local_Supported_Features_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Read_White_List_Size_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Read_Channel_Map_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Encrypt_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Rand_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Long_Term_Key_Request_Reply_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Long_Term_Key_Request_Negative_Reply_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void LE_Read_Supported_States_Command( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Hal_Get_Fw_Build_Number_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Hal_Read_Config_Data_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Hal_LE_Tx_Test_Packet_Number_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Hal_Get_Link_Status_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );
static void Hal_Get_Anchor_Period_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr );

static void LE_Advertising_Report_Handler( HCI_EVENT_PCKT* EventPacketPtr );
static void Fault_Data_Event_Handler( HCI_EVENT_PCKT* EventPacketPtr );


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
#define CMD_CALLBACK_NAME_HANDLER(OpcodeVal, handler) OpcodeVal ## _CMD_CALLBACK = { .CmdCompleteHandler = handler }


static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_DISCONNECT, 						NULL);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_READ_REMOTE_VERSION_INFORMATION, 	NULL);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_SET_EVENT_MASK, 					&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_CLEAR_WHITE_LIST, 				&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_READ_TRANSMIT_POWER_LEVEL, 			&Read_Transmit_Power_Level_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_RESET, 								&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_READ_LOCAL_VERSION_INFORMATION,		&Read_Local_Version_Information_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_READ_LOCAL_SUPPORTED_COMMANDS, 		&Read_Local_Supported_Commands_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_READ_LOCAL_SUPPORTED_FEATURES, 		&Read_Local_Supported_Features_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_READ_BD_ADDR, 						&Read_BD_ADDR_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_READ_RSSI, 							&Read_RSSI_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_EVENT_MASK, 					&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_READ_BUFFER_SIZE, 				&LE_Read_Buffer_Size_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_READ_LOCAL_SUPPORTED_FEATURES, 	&LE_Read_Local_Supported_Features_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_RANDOM_ADDRESS, 				&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_ADVERTISING_PARAMETERS, 		&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER, 	&LE_Read_Advertising_Physical_Channel_Tx_Power_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_ADVERTISING_DATA, 			&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_SCAN_RESPONSE_DATA, 			&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_ADVERTISING_ENABLE, 			&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_SCAN_PARAMETERS, 			&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_SCAN_ENABLE, 				&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_CREATE_CONNECTION, 				NULL);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_CREATE_CONNECTION_CANCEL, 		&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_READ_WHITE_LIST_SIZE, 			&LE_Read_White_List_Size_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_ADD_DEVICE_TO_WHITE_LIST, 		&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST, 	&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_CONNECTION_UPDATE, 				NULL);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION, &Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_READ_CHANNEL_MAP, 				&LE_Read_Channel_Map_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_READ_REMOTE_FEATURES, 			NULL);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_ENCRYPT, 						&LE_Encrypt_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_RAND, 							&LE_Rand_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_ENABLE_ENCRYPTION, 				NULL);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_LONG_TERM_KEY_REQUEST_REPLY, 	&LE_Long_Term_Key_Request_Reply_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY, 	&LE_Long_Term_Key_Request_Negative_Reply_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_READ_SUPPORTED_STATES, 			&LE_Read_Supported_States_Command);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_RECEIVER_TEST_V1, 				&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_TRANSMITTER_TEST_V1, 			&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	HCI_LE_TEST_END, 						&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_GET_FW_BUILD_NUMBER, 		&Hal_Get_Fw_Build_Number_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_WRITE_CONFIG_DATA, 			&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_READ_CONFIG_DATA, 			&Hal_Read_Config_Data_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_SET_TX_POWER_LEVEL, 			&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_DEVICE_STANDBY, 				&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER, 	&Hal_LE_Tx_Test_Packet_Number_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_TONE_START, 					&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_TONE_STOP, 					&Command_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_GET_LINK_STATUS, 			&Hal_Get_Link_Status_Complete);
static CMD_CALLBACK CMD_CALLBACK_NAME_HANDLER(	VS_ACI_HAL_GET_ANCHOR_PERIOD, 			&Hal_Get_Anchor_Period_Complete);


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

#define RETURN_ON_FAULT(comstatus) if( comstatus != TRANSFER_DONE ){ return; }

	HCI_PACKET_TYPE PacketType = *DataPtr;

	switch( PacketType )
	{
	case HCI_ACL_DATA_PACKET:
		RETURN_ON_FAULT(Status);
		{
			HCI_ACL_DATA_PCKT_HEADER Header = *( ( HCI_ACL_DATA_PCKT_HEADER* )( DataPtr + 1 ) );
			HCI_Controller_ACL_Data( Header, (uint8_t*)( DataPtr + 5 ) );
		}
		break;

	case HCI_SYNCHRONOUS_DATA_PACKET: /* Not used in this controller version */
		RETURN_ON_FAULT(Status);
		break;

	case HCI_ISO_DATA_PACKET: /* Not used in this controller version */
		RETURN_ON_FAULT(Status);
		break;

	case HCI_EVENT_PACKET:
	{
		HCI_EVENT_PCKT* EventPacketPtr = ( HCI_EVENT_PCKT* )( DataPtr + 1 );

		switch( EventPacketPtr->Event_Code )
		{
		/*---------- DISCONNECTION_COMPLETE_EVT ------------*//* Page 2296 Core_v5.2 */
		case DISCONNECTION_COMPLETE:
			RETURN_ON_FAULT(Status);
			HCI_Disconnection_Complete( EventPacketPtr->Event_Parameter[0],
					( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1],
					EventPacketPtr->Event_Parameter[3] );
			break;

			/*---------- ENCRYPTION_CHANGE_EVT ------------*//* Page 2299 Core_v5.2 */
		case ENCRYPTION_CHANGE:
			RETURN_ON_FAULT(Status);
			HCI_Encryption_Change( EventPacketPtr->Event_Parameter[0],
					( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1],
					EventPacketPtr->Event_Parameter[3] );
			break;

			/*---------- READ_REMOTE_VERSION_INFORMATION_COMPLETE_EVT ------------*//* Page 2304 Core_v5.2 */
		case READ_REMOTE_VERSION_INFORMATION_COMPLETE:
			RETURN_ON_FAULT(Status);
			HCI_Read_Remote_Version_Information_Complete( EventPacketPtr->Event_Parameter[0],
					( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1],
					EventPacketPtr->Event_Parameter[3],
					( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
					( EventPacketPtr->Event_Parameter[7] << 8 ) | EventPacketPtr->Event_Parameter[6] );
			break;

			/*---------- COMMAND_COMPLETE_EVT ------------*//* Page 2308 Core_v5.2 */
		case COMMAND_COMPLETE:
			Command_Complete_Handler( Status, EventPacketPtr );
			break;

			/*---------- COMMAND_STATUS_EVT --------------*//* Page 2310 Core_v5.2 */
		case COMMAND_STATUS:
			RETURN_ON_FAULT(Status);
			Command_Status_Handler( EventPacketPtr );
			break;

			/*---------- HARDWARE_ERROR_EVT ------------*//* Page 2312 Core_v5.2 */
		case HARDWARE_ERROR:
			RETURN_ON_FAULT(Status);
			HCI_Hardware_Error( EventPacketPtr->Event_Parameter[0] );
			break;

			/*---------- NUMBER_OF_COMPLETED_PACKETS_EVT ------------*//* Page 2315 Core_v5.2 */
		case NUMBER_OF_COMPLETED_PACKETS:
			RETURN_ON_FAULT(Status);
			{
				uint16_t Offset = EventPacketPtr->Event_Parameter[0] * 2;

				HCI_Number_Of_Completed_Packets( EventPacketPtr->Event_Parameter[0], (uint16_t*)( &EventPacketPtr->Event_Parameter[1] ), (uint16_t*)( &EventPacketPtr->Event_Parameter[Offset + 1] ) );
			}
			break;

			/*---------- DATA_BUFFER_OVERFLOW_EVT ------------*//* Page 2325 Core_v5.2 */
		case DATA_BUFFER_OVERFLOW:
			RETURN_ON_FAULT(Status);
			HCI_Data_Buffer_Overflow( EventPacketPtr->Event_Parameter[0] );
			break;

			/*---------- ENCRYPTION_KEY_REFRESH_COMPLETE_EVT ------------*//* Page 2349 Core_v5.2 */
		case ENCRYPTION_KEY_REFRESH_COMPLETE:
			RETURN_ON_FAULT(Status);
			HCI_Encryption_Key_Refresh_Complete( EventPacketPtr->Event_Parameter[0], ( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1] );
			break;

			/*---------- LE_META_EVT ------------*//* Page 2379 Core_v5.2 */
		case LE_META:
			RETURN_ON_FAULT(Status);
			switch( EventPacketPtr->Event_Parameter[0] )
			{

			case LE_CONNECTION_COMPLETE:
				HCI_LE_Connection_Complete( EventPacketPtr->Event_Parameter[1], ( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2],
						EventPacketPtr->Event_Parameter[4], EventPacketPtr->Event_Parameter[5], (BD_ADDR_TYPE*)(&(EventPacketPtr->Event_Parameter[6])), ( EventPacketPtr->Event_Parameter[13] << 8 ) | EventPacketPtr->Event_Parameter[12],
						( EventPacketPtr->Event_Parameter[15] << 8 ) | EventPacketPtr->Event_Parameter[14], ( EventPacketPtr->Event_Parameter[17] << 8 ) | EventPacketPtr->Event_Parameter[16],
						EventPacketPtr->Event_Parameter[18] );
				break;

			case LE_ADVERTISING_REPORT:
				LE_Advertising_Report_Handler( EventPacketPtr );
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
			case VENDOR_SPECIFIC:
				RETURN_ON_FAULT(Status);
				{
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

					case FAULT_DATA_EVENT_CODE:
						Fault_Data_Event_Handler( EventPacketPtr );
						break;
					}
				}
				break;

			default:
				break;
		}
	}
	break;

	default: /* The controller shall not issue HCI_COMMAND_PACKET packet */
		break;
	}
}


/****************************************************************/
/* Command_Complete_Handler()              		  		        */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Command_Complete_Handler( TRANSFER_STATUS Status, HCI_EVENT_PCKT* EventPacketPtr )
{
	HCI_COMMAND_OPCODE OpCode;
	OpCode.Val = ( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1];
	CMD_CALLBACK* CmdCallBack = Get_Command_CallBack( OpCode );

	if( Status == TRANSFER_DONE )
	{
		Set_Number_Of_HCI_Command_Packets( EventPacketPtr->Event_Parameter[0] );
	}else if( CmdCallBack != NULL )
	{
		/* Message reception failed: clear callback functions */
		CmdCallBack->Status = FALSE;
		CmdCallBack->CmdCompleteCallBack = NULL;
		CmdCallBack->CmdStatusCallBack = NULL;
		return;
	}else
	{
		/* Message reception failed: just returns. */
		return;
	}

	void* CmdCallBackFun = NULL; /* Function pointer */

	if( CmdCallBack != NULL )
	{
		/* clear the callback variable */
		CmdCallBackFun = CmdCallBack->CmdCompleteCallBack;
		CmdCallBack->Status = FALSE;
		CmdCallBack->CmdCompleteCallBack = NULL;
		CmdCallBack->CmdStatusCallBack = NULL;
		if( CmdCallBackFun != NULL ) /* We have handler at application side? */
		{
			if( CmdCallBack->CmdCompleteHandler != NULL ) /* We have local handler? */
			{
				CmdCallBack->CmdCompleteHandler( CmdCallBackFun, EventPacketPtr );
			}
		}
	}else
	{
		/* Treat unknown command */
		HCI_Command_Complete( EventPacketPtr->Event_Parameter[0], OpCode, &( EventPacketPtr->Event_Parameter[3] ) );
	}
}


/****************************************************************/
/* Command_Complete()       	         		  		        */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Command_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	( (DefCmdComplete)CmdCallBackFun )( EventPacketPtr->Event_Parameter[3] );
}


/****************************************************************/
/* Read_Transmit_Power_Level_Complete()            		        */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Transmit_Power_Level_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((TxPwrLvlComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
			EventPacketPtr->Event_Parameter[6] );
}


/****************************************************************/
/* Read_Local_Version_Information_Complete()       		        */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Version_Information_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((ReadLocalVerInfoComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4],
			( EventPacketPtr->Event_Parameter[6]  << 8 ) | EventPacketPtr->Event_Parameter[5], EventPacketPtr->Event_Parameter[7],
			( EventPacketPtr->Event_Parameter[9]  << 8 ) | EventPacketPtr->Event_Parameter[8],
			( EventPacketPtr->Event_Parameter[11] << 8 ) | EventPacketPtr->Event_Parameter[10] );
}


/****************************************************************/
/* Read_Local_Supported_Commands_Complete()       		        */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Supported_Commands_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((ReadLocalSupCmdsComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], (SUPPORTED_COMMANDS*)( &(EventPacketPtr->Event_Parameter[4]) ) );
}


/****************************************************************/
/* Read_Local_Supported_Features_Complete()   		 		    */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Supported_Features_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((ReadLocalSupFeaturesComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], (SUPPORTED_FEATURES*)( &( EventPacketPtr->Event_Parameter[4]) ) );
}


/****************************************************************/
/* Read_BD_ADDR_Complete()   		 		   					*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_BD_ADDR_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((ReadBDADDRComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], (BD_ADDR_TYPE*)(&(EventPacketPtr->Event_Parameter[4])) );
}


/****************************************************************/
/* Read_RSSI_Complete()   		 		   						*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Read_RSSI_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((ReadRSSIComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
			EventPacketPtr->Event_Parameter[6] );
}


/****************************************************************/
/* LE_Read_Buffer_Size_Complete()   	   						*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Buffer_Size_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LEReadBufferSizeComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
			EventPacketPtr->Event_Parameter[6] );
}


/****************************************************************/
/* LE_Read_Local_Supported_Features_Complete()  				*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Local_Supported_Features_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LEReadLocalSuppFeaturesComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], (LE_SUPPORTED_FEATURES*)( &(EventPacketPtr->Event_Parameter[4]) ) );
}


/****************************************************************/
/* LE_Read_Advertising_Physical_Channel_Tx_Power_Complete()  	*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LEReadAdvPhyChannelTxPowerComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4] );
}


/****************************************************************/
/* LE_Read_White_List_Size_Complete()  							*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_White_List_Size_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LEReadWhiteListSizeComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], EventPacketPtr->Event_Parameter[4] );
}


/****************************************************************/
/* LE_Read_Channel_Map_Complete()  								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Channel_Map_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LEReadChannelMapComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4],
			(CHANNEL_MAP*)(&(EventPacketPtr->Event_Parameter[6]) ) );
}


/****************************************************************/
/* LE_Encrypt_Complete()  										*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Encrypt_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LEEncryptComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]) );
}


/****************************************************************/
/* LE_Rand_Complete()  											*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Rand_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LERandComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]) );
}


/****************************************************************/
/* LE_Long_Term_Key_Request_Reply_Complete() 					*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Long_Term_Key_Request_Reply_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LELongTermKeyRqtReplyComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
}


/****************************************************************/
/* LE_Long_Term_Key_Request_Negative_Reply_Complete() 			*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Long_Term_Key_Request_Negative_Reply_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LELongTermKeyRqtNegReplyComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
}


/****************************************************************/
/* LE_Read_Supported_States_Command() 							*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Supported_States_Command( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((LEReadSupportedStatesComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], (SUPPORTED_LE_STATES*)(&(EventPacketPtr->Event_Parameter[4])) );
}


/****************************************************************/
/* Hal_Get_Fw_Build_Number_Complete() 							*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_Get_Fw_Build_Number_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((HalGetFwBuildNumberComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
}


/****************************************************************/
/* Hal_Read_Config_Data_Complete() 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_Read_Config_Data_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((HalReadConfigDataComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]), EventPacketPtr->Parameter_Total_Length - 4 );
}


/****************************************************************/
/* Hal_LE_Tx_Test_Packet_Number_Complete() 						*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_LE_Tx_Test_Packet_Number_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	uint32_t PacketCounter = ( EventPacketPtr->Event_Parameter[7] << 24 ) | ( EventPacketPtr->Event_Parameter[6] << 16 ) |
			( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4];
	((HalLETxTestPacketNumberComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], PacketCounter );
}


/****************************************************************/
/* Hal_Get_Link_Status_Complete() 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_Get_Link_Status_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	((HalGetLinkStatusComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]), &(EventPacketPtr->Event_Parameter[12]) );
}


/****************************************************************/
/* Hal_Get_Anchor_Period_Complete() 							*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hal_Get_Anchor_Period_Complete( void* CmdCallBackFun, HCI_EVENT_PCKT* EventPacketPtr )
{
	uint32_t AnchorInterval = ( EventPacketPtr->Event_Parameter[7] << 24 ) | ( EventPacketPtr->Event_Parameter[6] << 16 ) |
			( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4];

	uint32_t Maxslot = ( EventPacketPtr->Event_Parameter[11] << 24 ) | ( EventPacketPtr->Event_Parameter[10] << 16 ) |
			( EventPacketPtr->Event_Parameter[9] << 8 ) | EventPacketPtr->Event_Parameter[8];

	((HalGetAnchorPeriodComplete)CmdCallBackFun)( EventPacketPtr->Event_Parameter[3], AnchorInterval, Maxslot );
}


/****************************************************************/
/* Command_Status_Handler()       	             		        */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Command_Status_Handler( HCI_EVENT_PCKT* EventPacketPtr )
{
	HCI_COMMAND_OPCODE OpCode;

	Set_Number_Of_HCI_Command_Packets( EventPacketPtr->Event_Parameter[1] );

	OpCode.Val = ( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2];
	CMD_CALLBACK* CmdCallBack = Get_Command_CallBack( OpCode );

	void* CmdCallBackFun = ( CmdCallBack != NULL ) ? CmdCallBack->CmdStatusCallBack : NULL; /* Function pointer */

	if( CmdCallBackFun != NULL ) /* Do we have application handler? */
	{
		( (DefCmdStatus)CmdCallBackFun )( EventPacketPtr->Event_Parameter[0] );
	}else if( CmdCallBack == NULL )
	{
		/* Call unknown OpMode */
		HCI_Command_Status( EventPacketPtr->Event_Parameter[0], EventPacketPtr->Event_Parameter[1], OpCode );
	}
}


/****************************************************************/
/* LE_Advertising_Report_Handler()                    	        */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Advertising_Report_Handler( HCI_EVENT_PCKT* EventPacketPtr )
{
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


/****************************************************************/
/* Fault_Data_Event_Handler()                	    	        */
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Fault_Data_Event_Handler( HCI_EVENT_PCKT* EventPacketPtr )
{
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


/****************************************************************/
/* Get_Command_CallBack()                    		            */
/* Purpose: Get the command callback pointer from the Opcode.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
CMD_CALLBACK* Get_Command_CallBack( HCI_COMMAND_OPCODE OpCode )
{
	/* TODO: implement a way to to this decision faster (use a vector, maybe?) */
	switch ( OpCode.Val )
	{
	case HCI_DISCONNECT: 							return ( &CMD_CALLBACK_NAME(HCI_DISCONNECT	) );

	case HCI_READ_REMOTE_VERSION_INFORMATION: 		return ( &CMD_CALLBACK_NAME(HCI_READ_REMOTE_VERSION_INFORMATION) );

	case HCI_SET_EVENT_MASK: 						return ( &CMD_CALLBACK_NAME(HCI_SET_EVENT_MASK) );

	case HCI_LE_CLEAR_WHITE_LIST: 					return ( &CMD_CALLBACK_NAME(HCI_LE_CLEAR_WHITE_LIST) );

	case HCI_READ_TRANSMIT_POWER_LEVEL: 			return ( &CMD_CALLBACK_NAME(HCI_READ_TRANSMIT_POWER_LEVEL) );

	case HCI_RESET: 								return ( &CMD_CALLBACK_NAME(HCI_RESET) );

	case HCI_READ_LOCAL_VERSION_INFORMATION: 		return ( &CMD_CALLBACK_NAME(HCI_READ_LOCAL_VERSION_INFORMATION) );

	case HCI_READ_LOCAL_SUPPORTED_COMMANDS: 		return ( &CMD_CALLBACK_NAME(HCI_READ_LOCAL_SUPPORTED_COMMANDS) );

	case HCI_READ_LOCAL_SUPPORTED_FEATURES: 		return ( &CMD_CALLBACK_NAME(HCI_READ_LOCAL_SUPPORTED_FEATURES) );

	case HCI_READ_BD_ADDR: 							return ( &CMD_CALLBACK_NAME(HCI_READ_BD_ADDR) );

	case HCI_READ_RSSI: 							return ( &CMD_CALLBACK_NAME(HCI_READ_RSSI) );

	case HCI_LE_SET_EVENT_MASK: 					return ( &CMD_CALLBACK_NAME(HCI_LE_SET_EVENT_MASK) );

	case HCI_LE_READ_BUFFER_SIZE: 					return ( &CMD_CALLBACK_NAME(HCI_LE_READ_BUFFER_SIZE) );

	case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES: 		return ( &CMD_CALLBACK_NAME(HCI_LE_READ_LOCAL_SUPPORTED_FEATURES) );

	case HCI_LE_SET_RANDOM_ADDRESS: 				return ( &CMD_CALLBACK_NAME(HCI_LE_SET_RANDOM_ADDRESS) );

	case HCI_LE_SET_ADVERTISING_PARAMETERS: 		return ( &CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_PARAMETERS) );

	case HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER: 		return ( &CMD_CALLBACK_NAME(HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER) );

	case HCI_LE_SET_ADVERTISING_DATA: 				return ( &CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_DATA) );

	case HCI_LE_SET_SCAN_RESPONSE_DATA: 			return ( &CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_RESPONSE_DATA) );

	case HCI_LE_SET_ADVERTISING_ENABLE: 			return ( &CMD_CALLBACK_NAME(HCI_LE_SET_ADVERTISING_ENABLE) );

	case HCI_LE_SET_SCAN_PARAMETERS: 				return ( &CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_PARAMETERS) );

	case HCI_LE_SET_SCAN_ENABLE: 					return ( &CMD_CALLBACK_NAME(HCI_LE_SET_SCAN_ENABLE) );

	case HCI_LE_CREATE_CONNECTION: 					return ( &CMD_CALLBACK_NAME(HCI_LE_CREATE_CONNECTION) );

	case HCI_LE_CREATE_CONNECTION_CANCEL: 			return ( &CMD_CALLBACK_NAME(HCI_LE_CREATE_CONNECTION_CANCEL) );

	case HCI_LE_READ_WHITE_LIST_SIZE: 				return ( &CMD_CALLBACK_NAME(HCI_LE_READ_WHITE_LIST_SIZE) );

	case HCI_LE_ADD_DEVICE_TO_WHITE_LIST: 			return ( &CMD_CALLBACK_NAME(HCI_LE_ADD_DEVICE_TO_WHITE_LIST) );

	case HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST: 		return ( &CMD_CALLBACK_NAME(HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST) );

	case HCI_LE_CONNECTION_UPDATE: 					return ( &CMD_CALLBACK_NAME(HCI_LE_CONNECTION_UPDATE) );

	case HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION: 	return ( &CMD_CALLBACK_NAME(HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION) );

	case HCI_LE_READ_CHANNEL_MAP: 					return ( &CMD_CALLBACK_NAME(HCI_LE_READ_CHANNEL_MAP) );

	case HCI_LE_READ_REMOTE_FEATURES: 				return ( &CMD_CALLBACK_NAME(HCI_LE_READ_REMOTE_FEATURES) );

	case HCI_LE_ENCRYPT: 							return ( &CMD_CALLBACK_NAME(HCI_LE_ENCRYPT) );

	case HCI_LE_RAND: 								return ( &CMD_CALLBACK_NAME(HCI_LE_RAND) );

	case HCI_LE_ENABLE_ENCRYPTION: 					return ( &CMD_CALLBACK_NAME(HCI_LE_ENABLE_ENCRYPTION) );

	case HCI_LE_LONG_TERM_KEY_REQUEST_REPLY: 		return ( &CMD_CALLBACK_NAME(HCI_LE_LONG_TERM_KEY_REQUEST_REPLY) );

	case HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY: 		return ( &CMD_CALLBACK_NAME(HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY) );

	case HCI_LE_READ_SUPPORTED_STATES: 				return ( &CMD_CALLBACK_NAME(HCI_LE_READ_SUPPORTED_STATES) );

	case HCI_LE_RECEIVER_TEST_V1: 					return ( &CMD_CALLBACK_NAME(HCI_LE_RECEIVER_TEST_V1) );

	case HCI_LE_TRANSMITTER_TEST_V1: 				return ( &CMD_CALLBACK_NAME(HCI_LE_TRANSMITTER_TEST_V1) );

	case HCI_LE_TEST_END: 							return ( &CMD_CALLBACK_NAME(HCI_LE_TEST_END) );

	case VS_ACI_HAL_GET_FW_BUILD_NUMBER: 			return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_GET_FW_BUILD_NUMBER) );

	case VS_ACI_HAL_WRITE_CONFIG_DATA: 				return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_WRITE_CONFIG_DATA) );

	case VS_ACI_HAL_READ_CONFIG_DATA: 				return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_READ_CONFIG_DATA) );

	case VS_ACI_HAL_SET_TX_POWER_LEVEL: 			return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_SET_TX_POWER_LEVEL) );

	case VS_ACI_HAL_DEVICE_STANDBY: 				return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_DEVICE_STANDBY) );

	case VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER: 		return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER) );

	case VS_ACI_HAL_TONE_START: 					return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_TONE_START) );

	case VS_ACI_HAL_TONE_STOP: 						return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_TONE_STOP) );

	case VS_ACI_HAL_GET_LINK_STATUS: 				return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_GET_LINK_STATUS) );

	case VS_ACI_HAL_GET_ANCHOR_PERIOD: 				return ( &CMD_CALLBACK_NAME(VS_ACI_HAL_GET_ANCHOR_PERIOD) );

	/* Not all commands have callback */
	default: return (NULL);
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
