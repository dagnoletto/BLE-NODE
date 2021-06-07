

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


static CMD_CALLBACK CMD_CALLBACK_NAME(HCI_LE_CLEAR_WHITE_LIST) = { .Status = FALSE, .FunPtr = NULL };


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
		TransferCallBack CallBack)
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
		if( CmdPacket->PacketType == HCI_COMMAND_PACKET )
		{
			Decrement_HCI_Command_Packets(  );

			if( CallBackPtr != NULL )
			{
				CallBackPtr->Status = TRUE;
				CallBackPtr->FunPtr = NULL; //TODO
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
					CmdCallBackFun = CmdCallBack->FunPtr;
					CmdCallBack->Status = FALSE;
					CmdCallBack->FunPtr = NULL;
				}

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

				case HCI_READ_LOCAL_SUPPORTED_COMMANDS:
					HCI_Read_Local_Supported_Commands_Complete( EventPacketPtr->Event_Parameter[3], (SUPPORTED_COMMANDS*)( &(EventPacketPtr->Event_Parameter[4]) ) );
					break;

				case HCI_READ_LOCAL_SUPPORTED_FEATURES:
					HCI_Read_Local_Supported_Features_Complete( EventPacketPtr->Event_Parameter[3], (SUPPORTED_FEATURES*)( &(EventPacketPtr->Event_Parameter[4]) ) );
					break;

				case HCI_READ_BD_ADDR:
					HCI_Read_BD_ADDR_Complete( EventPacketPtr->Event_Parameter[3], (BD_ADDR_TYPE*)(&(EventPacketPtr->Event_Parameter[4])) );
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

				case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES:
					HCI_LE_Read_Local_Supported_Features_Complete( EventPacketPtr->Event_Parameter[3], (LE_SUPPORTED_FEATURES*)( &(EventPacketPtr->Event_Parameter[4]) ) );
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

				case HCI_LE_ADD_DEVICE_TO_WHITE_LIST:
					HCI_LE_Add_Device_To_White_List_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST:
					HCI_LE_Remove_Device_From_White_List_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION:
					HCI_LE_Set_Host_Channel_Classification_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_READ_CHANNEL_MAP:
					HCI_LE_Read_Channel_Map_Complete( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4], (CHANNEL_MAP*)(&(EventPacketPtr->Event_Parameter[6])) );
					break;

				case HCI_LE_ENCRYPT:
					HCI_LE_Encrypt_Complete( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]) );
					break;

				case HCI_LE_RAND:
					HCI_LE_Rand_Complete( EventPacketPtr->Event_Parameter[3], &(EventPacketPtr->Event_Parameter[4]) );
					break;

				case HCI_LE_LONG_TERM_KEY_REQUEST_REPLY:
					HCI_LE_Long_Term_Key_Request_Reply_Complete( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
					break;

				case HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY:
					HCI_LE_Long_Term_Key_Request_Negative_Reply_Complete( EventPacketPtr->Event_Parameter[3], ( EventPacketPtr->Event_Parameter[5] << 8 ) | EventPacketPtr->Event_Parameter[4] );
					break;

				case HCI_LE_READ_SUPPORTED_STATES:
					HCI_LE_Read_Supported_States_Complete( EventPacketPtr->Event_Parameter[3], (SUPPORTED_LE_STATES*)(&(EventPacketPtr->Event_Parameter[4])) );
					break;

				case HCI_LE_RECEIVER_TEST_V1:
					HCI_LE_Receiver_Test_v1_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_TRANSMITTER_TEST_V1:
					HCI_LE_Transmitter_Test_v1_Complete( EventPacketPtr->Event_Parameter[3] );
					break;

				case HCI_LE_TEST_END:
					HCI_LE_Test_End_Complete( EventPacketPtr->Event_Parameter[3] );
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

				default:
					HCI_Command_Complete( Num_HCI_Command_Packets, OpCode, &( EventPacketPtr->Event_Parameter[3] ) );
					break;

				}
			}
			break;



			/*---------- COMMAND_STATUS_EVT --------------*//* Page 2310 Core_v5.2 */
			case COMMAND_STATUS: {
				OpCode.Val = ( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2];
				uint8_t Num_HCI_Command_Packets = EventPacketPtr->Event_Parameter[1];
				Set_Number_Of_HCI_Command_Packets( Num_HCI_Command_Packets );

				CMD_CALLBACK* CmdCallBack = Get_Command_CallBack( OpCode );
				void* CmdCallBackFun = NULL; /* Function pointer */

				if( CmdCallBack != NULL )
				{
					CmdCallBackFun = CmdCallBack->FunPtr;
					CmdCallBack->Status = FALSE;
					CmdCallBack->FunPtr = NULL;
				}

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

				case HCI_LE_ADD_DEVICE_TO_WHITE_LIST:
					HCI_LE_Add_Device_To_White_List_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST:
					HCI_LE_Remove_Device_From_White_List_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_CONNECTION_UPDATE:
					HCI_LE_Connection_Update_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION:
					HCI_LE_Set_Host_Channel_Classification_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_READ_CHANNEL_MAP:
					HCI_LE_Read_Channel_Map_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_READ_REMOTE_FEATURES:
					HCI_LE_Read_Remote_Features_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_ENCRYPT:
					HCI_LE_Encrypt_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_RAND:
					HCI_LE_Rand_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_ENABLE_ENCRYPTION:
					HCI_LE_Enable_Encryption_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_LONG_TERM_KEY_REQUEST_REPLY:
					HCI_LE_Long_Term_Key_Request_Reply_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY:
					HCI_LE_Long_Term_Key_Request_Negative_Reply_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_READ_SUPPORTED_STATES:
					HCI_LE_Read_Supported_States_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_RECEIVER_TEST_V1:
					HCI_LE_Receiver_Test_v1_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_TRANSMITTER_TEST_V1:
					HCI_LE_Transmitter_Test_v1_Status( EventPacketPtr->Event_Parameter[0] );
					break;

				case HCI_LE_TEST_END:
					HCI_LE_Test_End_Status( EventPacketPtr->Event_Parameter[0] );
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

				default:
					HCI_Command_Status( EventPacketPtr->Event_Parameter[0], Num_HCI_Command_Packets, OpCode );
					break;
				}}
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
		CallBackPtr->FunPtr = NULL;
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
	case HCI_LE_CLEAR_WHITE_LIST:
		return ( &CMD_CALLBACK_NAME(HCI_LE_CLEAR_WHITE_LIST) );
		break;

		/* Not all commands have callback */
	default:
		return (NULL);
		break;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
