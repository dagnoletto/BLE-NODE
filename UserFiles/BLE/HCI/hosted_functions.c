

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hosted_functions.h"
#include "ble_states.h"
#include "security_manager.h"


/****************************************************************/
/* Defines		                                                */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static void Transform_Status_To_Command_Event( HCI_EVENT_PCKT* EventPacketPtr );


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef struct
{
	uint8_t NumberOfEntries;
	DEVICE_IDENTITY Entry[MAX_NUMBER_OF_RESOLVING_LIST_ENTRIES];
}HOSTED_RESOLVING_LIST;


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static uint8_t Address_Resol_Controller_Cmd = FALSE;
static uint8_t Address_Resol_Controller = FALSE;
static HOSTED_RESOLVING_LIST Hosted_Resolving_List;


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
	if( Status == TRANSFER_DONE )
	{
		/* The command parameters are catch in the very request. */
		Address_Resol_Controller_Cmd = ( (HCI_SERIAL_COMMAND_PCKT*)DataPtr )->CmdPacket.Parameter[0];
		return (TRUE);
	}
	return (FALSE);
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
	BLE_STATES state;

	switch ( OpCode.Val )
	{
	case HCI_LE_SET_ADDRESS_RESOLUTION_ENABLE:
		/* Those very simple commands can return the status right here */
		/* We must transform a status event packet to a command event packet */
		EventPacketPtr->Parameter_Total_Length = 4;
		Transform_Status_To_Command_Event( EventPacketPtr );

		/* This command shall not be used when:
		• Advertising (other than periodic advertising) is enabled,
		• Scanning is enabled, or
		• an HCI_LE_Create_Connection, HCI_LE_Extended_Create_Connection, or
		HCI_LE_Periodic_Advertising_Create_Sync command is outstanding. */

		state = Get_BLE_State();
		if( ( state == ADVERTISING_STATE ) || ( state == SCANNING_STATE ) ||
				( Address_Resol_Controller == Address_Resol_Controller_Cmd ) )
		{
			EventPacketPtr->Event_Parameter[3] = COMMAND_DISALLOWED;
		}else if( Address_Resol_Controller_Cmd & 0xFE ) /* Check if parameter values are OK */
		{
			EventPacketPtr->Event_Parameter[3] = INVALID_HCI_COMMAND_PARAMETERS;
		}else
		{
			Address_Resol_Controller = Address_Resol_Controller_Cmd;
			EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
		}
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );
		break;

	case HCI_LE_CLEAR_RESOLVING_LIST:
		/* Those very simple commands can return the status right here */
		/* We must transform a status event packet to a command event packet */
		EventPacketPtr->Parameter_Total_Length = 4;
		Transform_Status_To_Command_Event( EventPacketPtr );

		/* This command shall not be used when address resolution is enabled in the
		Controller and:
		• Advertising (other than periodic advertising) is enabled,
		• Scanning is enabled, or
		• an HCI_LE_Create_Connection, HCI_LE_Extended_Create_Connection, or
		HCI_LE_Periodic_Advertising_Create_Sync command is outstanding.
		This command may be used at any time when address resolution is disabled in
		the Controller. */

		state = Get_BLE_State();
		if( ( Address_Resol_Controller ) && ( ( state == ADVERTISING_STATE ) || ( state == SCANNING_STATE ) ) )
		{
			EventPacketPtr->Event_Parameter[3] = COMMAND_DISALLOWED;
		}else
		{
			Hosted_Resolving_List.NumberOfEntries = 0;
			EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
		}
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );
		break;

	case HCI_LE_READ_RESOLVING_LIST_SIZE:
		/* Those very simple commands can return the status right here */
		/* We must transform a status event packet to a command event packet */
		EventPacketPtr->Parameter_Total_Length = 5;
		Transform_Status_To_Command_Event( EventPacketPtr );
		EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
		EventPacketPtr->Event_Parameter[4] = sizeof( Hosted_Resolving_List.Entry ) / sizeof( DEVICE_IDENTITY );
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
