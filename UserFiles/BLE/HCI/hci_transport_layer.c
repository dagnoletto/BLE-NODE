

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

			/*---------- COMMAND_COMPLETE_EVT ------------*/
			case COMMAND_COMPLETE: {
				OpCode.Val = ( EventPacketPtr->Event_Parameter[2] << 8 ) | EventPacketPtr->Event_Parameter[1];

				switch( OpCode.Val )
				{
				case HCI_LE_SET_ADV_DATA_OPCODE:
					HCI_LE_Set_Advertising_Data_Event( COMMAND_COMPLETE, EventPacketPtr->Event_Parameter[3] );
					break;
				}}
			break;

			/*---------- COMMAND_STATUS_EVT --------------*/
			case COMMAND_STATUS: {
				OpCode.Val = ( EventPacketPtr->Event_Parameter[3] << 8 ) | EventPacketPtr->Event_Parameter[2];
				uint8_t teste = EventPacketPtr->Event_Parameter[1]; /* TODO: teste (apagar depois) */

				switch( OpCode.Val )
				{
				case HCI_LE_SET_ADV_DATA_OPCODE:
					HCI_LE_Set_Advertising_Data_Event( COMMAND_STATUS, EventPacketPtr->Event_Parameter[0] );
					break;
				}}
			break;

			/*--------- VENDOR_SPECIFIC_EVT -------------*/
			case VENDOR_SPECIFIC:
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
