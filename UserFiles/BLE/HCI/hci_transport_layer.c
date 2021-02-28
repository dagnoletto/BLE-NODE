

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
BLUETOOTH_ERROR_CODES HCI_Transmit(void* DataPtr, uint16_t DataSize,
		TRANSFER_CALL_BACK_MODE CallBackMode,
		TransferCallBack CallBack)
{
	/* TODO: colocar blocking aqui para não pode chamar mais de uma função ao mesmo tempo? */
	int8_t Ntries =  3;

	TRANSFER_DESCRIPTOR TransferDesc;

	TransferDesc.CallBack = CallBack;
	TransferDesc.CallBackMode = CallBackMode;
	TransferDesc.DataPtr = (uint8_t*)DataPtr;
	TransferDesc.DataSize = DataSize;

	/* As the buffer could be blocked waiting another operation, you should try some times. */
	while( ( Bluenrg_Add_Frame( &TransferDesc, 7 ).EnqueuedAtIndex < 0 ) && ( Ntries > 0 ) )
	{
		Ntries--;
	}

	//	{
	//		return (0); //TODO: return erro: finalizar
	//	}

	return (1); //TODO: return OK: finalizar
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
	uint8_t teste = 0;
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
