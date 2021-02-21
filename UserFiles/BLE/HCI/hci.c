

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "BluenrgMS.h"


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
/* LE_Set_Advertising_Data()                                    */
/* Purpose: The HCI_LE_Set_Advertising_Data command is used to  */
/* set the data used in advertising packets that have a data 	*/
/* field.    		    										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t LE_Set_Advertising_Data( uint8_t Advertising_Data_Length, uint8_t Advertising_Data[] )
{
	/* TODO: isto é apenas um teste */

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + Advertising_Data_Length;
	TRANSFER_DESCRIPTOR TransferDesc;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.OGF = 0x08;
	PcktPtr->CmdPacket.OpCode.OCF = 0x0008;
	PcktPtr->CmdPacket.Parameter_Total_Length = Advertising_Data_Length;

	for( uint8_t i = 0; i < Advertising_Data_Length; i++ )
	{
		PcktPtr->CmdPacket.Parameter[i] = Advertising_Data[i];
	}

	TransferDesc.CallBack = NULL;
	TransferDesc.CallBackMode = CALL_BACK_BUFFERED;
	TransferDesc.DataPtr = (uint8_t*)PcktPtr;
	TransferDesc.DataSize = ByteArraySize;

	BluenrgMS_Add_Message( &TransferDesc, 7 );

	free( PcktPtr );
	return (0); /* TODO: teste */
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
