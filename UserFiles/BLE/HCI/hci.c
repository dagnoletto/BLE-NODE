

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "hci_transport_layer.h"


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
BLUETOOTH_ERROR_CODES LE_Set_Advertising_Data( uint8_t Advertising_Data_Length, uint8_t Advertising_Data[] )
{
	/* TODO: finalizar */
	BLUETOOTH_ERROR_CODES Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + Advertising_Data_Length;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.OGF = 0x08;
	PcktPtr->CmdPacket.OpCode.OCF = 0x0008;
	PcktPtr->CmdPacket.Parameter_Total_Length = Advertising_Data_Length;

	for( uint8_t i = 0; i < Advertising_Data_Length; i++ )
	{
		PcktPtr->CmdPacket.Parameter[i] = Advertising_Data[i];
	}

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_BUFFERED, NULL );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
