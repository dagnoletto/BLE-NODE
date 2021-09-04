

#ifndef HOSTED_FUNCTIONS_H_
#define HOSTED_FUNCTIONS_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci_transport_layer.h"
#include "Types.h"


/****************************************************************/
/* Type Defines 						                        */
/****************************************************************/


/****************************************************************/
/* Type Defines (For driver interface)                          */
/****************************************************************/


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/
uint8_t Hosted_LE_Add_Device_To_Resolving_List(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);
uint8_t Hosted_LE_Remove_Device_From_Resolving_List(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);
uint8_t Hosted_LE_Read_Peer_Resolvable_Address(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);
uint8_t Hosted_LE_Read_Local_Resolvable_Address(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);
uint8_t Hosted_LE_Set_Address_Resolution_Enable(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);
uint8_t Hosted_LE_Set_Resolvable_Private_Address_Timeout(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status);
uint8_t Hosted_Address_Resolution_Status( void );
void Delegate_Function_To_Host( HCI_COMMAND_OPCODE OpCode, CMD_CALLBACK* CmdCallBack,
		HCI_EVENT_PCKT* EventPacketPtr );
void Hosted_Functions_Process( void );


#endif /* HOSTED_FUNCTIONS_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
