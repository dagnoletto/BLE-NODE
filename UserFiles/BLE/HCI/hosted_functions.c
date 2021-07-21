

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hosted_functions.h"
#include "ble_states.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Address_Resol_Controller = FALSE;
static CONTROLLER_ERROR_CODES Address_Resol_Controller_Error_Code;


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static void Transform_Status_To_Command_Event( HCI_EVENT_PCKT* EventPacketPtr );


/****************************************************************/
/* Hosted_LE_Set_Address_Resolution_Enable()               	    */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Set_Address_Resolution_Enable(void* DataPtr, uint16_t DataSize,
		TRANSFER_STATUS Status)
{
	/* For some simple commands, the action can be done in the very request.
	 * Others may need to trigger more processing. */
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = (HCI_SERIAL_COMMAND_PCKT*)DataPtr;
	if( Status == TRANSFER_DONE )
	{
		/* This command shall not be used when:
		• Advertising (other than periodic advertising) is enabled,
		• Scanning is enabled, or
		• an HCI_LE_Create_Connection, HCI_LE_Extended_Create_Connection, or
		HCI_LE_Periodic_Advertising_Create_Sync command is outstanding. */

		BLE_STATES state = Get_BLE_State();
		if( ( state == ADVERTISING_STATE ) || ( state == SCANNING_STATE ) ||
				( Address_Resol_Controller == PcktPtr->CmdPacket.Parameter[0] ) )
		{
			Address_Resol_Controller_Error_Code = COMMAND_DISALLOWED;
		}else if( PcktPtr->CmdPacket.Parameter[0] & 0xFE ) /* Check if parameter values are OK */
		{
			Address_Resol_Controller_Error_Code = INVALID_HCI_COMMAND_PARAMETERS;
		}else
		{
			Address_Resol_Controller = PcktPtr->CmdPacket.Parameter[0];
			Address_Resol_Controller_Error_Code = COMMAND_SUCCESS;
		}
		return (TRUE);
	}
	return (FALSE);
}


/****************************************************************/
/* Hosted_LE_Clear_Resolving_List()               	            */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Clear_Resolving_List(void* DataPtr, uint16_t DataSize,
		TRANSFER_STATUS Status)
{
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = (HCI_SERIAL_COMMAND_PCKT*)DataPtr;
	return (TRUE);
}


/****************************************************************/
/* Delegate_Function_To_Host()            		   	            */
/* Purpose: Check which functions can be delegated to host.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Delegate_Function_To_Host( HCI_COMMAND_OPCODE OpCode, CMD_CALLBACK* CmdCallBack,
		HCI_EVENT_PCKT* EventPacketPtr )
{
	switch ( OpCode.Val )
	{
	case HCI_LE_SET_ADDRESS_RESOLUTION_ENABLE:
		/* Those very simple commands can return the status right here */
		/* We must transform a status event packet to a command event packet */
		EventPacketPtr->Parameter_Total_Length = 4;
		Transform_Status_To_Command_Event( EventPacketPtr );
		EventPacketPtr->Event_Parameter[3] = Address_Resol_Controller_Error_Code;
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );
		break;

	case HCI_LE_CLEAR_RESOLVING_LIST:
		/* Those very simple commands can return the status right here */
		/* We must transform a status event packet to a command event packet */
		EventPacketPtr->Parameter_Total_Length = 4;
		Transform_Status_To_Command_Event( EventPacketPtr );
		EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );
		break;

	default: /* Host cannot perform this function */
		Command_Status_Handler( OpCode, CmdCallBack, EventPacketPtr );
		break;
	}
}


/****************************************************************/
/* Transform_Status_To_Command_Event()            	            */
/* Purpose: Transform the status event packet to a command 		*/
/* event packet.												*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Transform_Status_To_Command_Event( HCI_EVENT_PCKT* EventPacketPtr )
{
	/* Num_HCI_Command_Packets */
	EventPacketPtr->Event_Parameter[0] = EventPacketPtr->Event_Parameter[1];
	/* Command_Opcode */
	EventPacketPtr->Event_Parameter[1] = EventPacketPtr->Event_Parameter[2];
	EventPacketPtr->Event_Parameter[2] = EventPacketPtr->Event_Parameter[3];
}


/****************************************************************/
/* Hosted_Functions_Process()            		   	            */
/* Purpose: Process hosted functions.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Hosted_Functions_Process( void )
{

}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
