

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hosted_functions.h"
#include "ble_states.h"
#include "security_manager.h"


/****************************************************************/
/* Defines		                                                */
/****************************************************************/
#define HOSTED_RESOLVING_LIST_SIZE ( sizeof( Hosted_Resolving_List.Entry ) / sizeof( DEVICE_IDENTITY ) )


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static void Transform_Status_To_Command_Event( HCI_EVENT_PCKT* EventPacketPtr );
static CONTROLLER_ERROR_CODES Add_To_Resolving_List( void );
static CONTROLLER_ERROR_CODES Remove_From_Resolving_List( void );


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
static DEVICE_IDENTITY Add_Device;
static IDENTITY_ADDRESS Remove_Device;


/****************************************************************/
/* Hosted_LE_Add_Device_To_Resolving_List()               	    */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Add_Device_To_Resolving_List(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	if( Status == TRANSFER_DONE )
	{
		/* The command parameters are catch in the very request. */
		HCI_SERIAL_COMMAND_PCKT* Packet = (HCI_SERIAL_COMMAND_PCKT*)( DataPtr );

		Add_Device.Peer_Identity_Address.Type = Packet->CmdPacket.Parameter[0];
		memcpy( &Add_Device.Peer_Identity_Address.Address.Bytes[0], &Packet->CmdPacket.Parameter[1], sizeof(BD_ADDR_TYPE) );
		memcpy( &Add_Device.Peer_IRK.Bytes[0], &Packet->CmdPacket.Parameter[sizeof(BD_ADDR_TYPE) + 1], sizeof(IRK_TYPE) );
		memcpy( &Add_Device.Local_IRK.Bytes[0], &Packet->CmdPacket.Parameter[sizeof(IRK_TYPE) + sizeof(BD_ADDR_TYPE) + 1], sizeof(IRK_TYPE) );

		return (TRUE);
	}
	return (FALSE);
}


/****************************************************************/
/* Hosted_LE_Remove_Device_From_Resolving_List()           	    */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Remove_Device_From_Resolving_List(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	if( Status == TRANSFER_DONE )
	{
		/* The command parameters are catch in the very request. */
		HCI_SERIAL_COMMAND_PCKT* Packet = (HCI_SERIAL_COMMAND_PCKT*)( DataPtr );

		Remove_Device.Type = Packet->CmdPacket.Parameter[0];
		memcpy( &Remove_Device.Address.Bytes[0], &Packet->CmdPacket.Parameter[1], sizeof(BD_ADDR_TYPE) );

		return (TRUE);
	}
	return (FALSE);
}


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

	BLE_STATES state = Get_BLE_State();

	/* Those very simple commands can return the status right here */
	/* We must transform a status event packet to a command event packet */
	switch ( OpCode.Val )
	{
	case HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST:

		EventPacketPtr->Parameter_Total_Length = 4;
		Transform_Status_To_Command_Event( EventPacketPtr );

		/* This command shall not be used when address resolution is enabled in the
		Controller and:
		• Advertising (other than periodic advertising) is enabled,
		• Scanning is enabled, or
		• an HCI_LE_Create_Connection, HCI_LE_Extended_Create_Connection, or
		HCI_LE_Periodic_Advertising_Create_Sync command is outstanding.
		This command may be used at any time when address resolution is disabled in
		the Controller.
		The added device shall be set to Network Privacy mode.
		When a Controller cannot add a device to the list because there is no space
		available, it shall return the error code Memory Capacity Exceeded (0x07). */

		if( ( Address_Resol_Controller ) && ( ( state == ADVERTISING_STATE ) || ( state == SCANNING_STATE ) ) )
		{
			EventPacketPtr->Event_Parameter[3] = COMMAND_DISALLOWED;
		}else if( Add_Device.Peer_Identity_Address.Type & 0xFE ) /* Check if parameter values are OK */
		{
			EventPacketPtr->Event_Parameter[3] = INVALID_HCI_COMMAND_PARAMETERS;
		}else if( Hosted_Resolving_List.NumberOfEntries < HOSTED_RESOLVING_LIST_SIZE )
		{
			EventPacketPtr->Event_Parameter[3] = Add_To_Resolving_List();
		}else
		{
			EventPacketPtr->Event_Parameter[3] = MEM_CAPACITY_EXCEEDED;
		}
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );

		break;

	case HCI_LE_REMOVE_DEVICE_FROM_RESOLVING_LIST:

		EventPacketPtr->Parameter_Total_Length = 4;
		Transform_Status_To_Command_Event( EventPacketPtr );

		/* This command shall not be used when address resolution is enabled in the
		Controller and:
		• Advertising (other than periodic advertising) is enabled,
		• Scanning is enabled, or
		• an HCI_LE_Create_Connection, HCI_LE_Extended_Create_Connection, or
		HCI_LE_Periodic_Advertising_Create_Sync command is outstanding.
		This command may be used at any time when address resolution is disabled in
		the Controller.
		When a Controller cannot remove a device from the resolving list because it is
		not found, it shall return the error code Unknown Connection Identifier (0x02). */

		if( ( Address_Resol_Controller ) && ( ( state == ADVERTISING_STATE ) || ( state == SCANNING_STATE ) ) )
		{
			EventPacketPtr->Event_Parameter[3] = COMMAND_DISALLOWED;
		}else if( Remove_Device.Type & 0xFE ) /* Check if parameter values are OK */
		{
			EventPacketPtr->Event_Parameter[3] = INVALID_HCI_COMMAND_PARAMETERS;
		}else if( Hosted_Resolving_List.NumberOfEntries )
		{
			EventPacketPtr->Event_Parameter[3] = Remove_From_Resolving_List();
		}else
		{
			EventPacketPtr->Event_Parameter[3] = UNKNOWN_CONNECTION_ID;
		}
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );

		break;

	case HCI_LE_CLEAR_RESOLVING_LIST:

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

		EventPacketPtr->Parameter_Total_Length = 5;
		Transform_Status_To_Command_Event( EventPacketPtr );
		EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
		EventPacketPtr->Event_Parameter[4] = HOSTED_RESOLVING_LIST_SIZE;
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );

		break;

	case HCI_LE_SET_ADDRESS_RESOLUTION_ENABLE:

		EventPacketPtr->Parameter_Total_Length = 4;
		Transform_Status_To_Command_Event( EventPacketPtr );

		/* This command shall not be used when:
		• Advertising (other than periodic advertising) is enabled,
		• Scanning is enabled, or
		• an HCI_LE_Create_Connection, HCI_LE_Extended_Create_Connection, or
		HCI_LE_Periodic_Advertising_Create_Sync command is outstanding. */

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
/* Add_To_Resolving_List()            			   	            */
/* Purpose: Add new device to resolving list.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static CONTROLLER_ERROR_CODES Add_To_Resolving_List( void )
{
	for ( uint8_t i = 0; i < Hosted_Resolving_List.NumberOfEntries; i++ )
	{
		/* Check if this entry is already in the list */
		if ( memcmp( &Hosted_Resolving_List.Entry[i].Peer_Identity_Address, &Add_Device.Peer_Identity_Address, sizeof(IDENTITY_ADDRESS) ) == 0 )
		{
			return ( INVALID_HCI_COMMAND_PARAMETERS );
		}
	}

	Hosted_Resolving_List.Entry[ Hosted_Resolving_List.NumberOfEntries ] = Add_Device;
	Hosted_Resolving_List.NumberOfEntries++;

	return ( COMMAND_SUCCESS );
}


/****************************************************************/
/* Remove_From_Resolving_List()            		   	            */
/* Purpose: Remove current device from resolving list.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static CONTROLLER_ERROR_CODES Remove_From_Resolving_List( void )
{
	/* TODO: This implementation does not have performance. A lot of data is moved.
	 * Better implementation is pending. */
	for ( uint8_t i = 0; i < Hosted_Resolving_List.NumberOfEntries; i++ )
	{
		/* Check if this entry is in the list */
		if ( memcmp( &Hosted_Resolving_List.Entry[i].Peer_Identity_Address, &Remove_Device, sizeof(IDENTITY_ADDRESS) ) == 0 )
		{
			for ( uint8_t a = i; a < (Hosted_Resolving_List.NumberOfEntries - 1); a++ )
			{
				/* Reorganize the list */
				Hosted_Resolving_List.Entry[a] = Hosted_Resolving_List.Entry[ a + 1 ];
			}
			Hosted_Resolving_List.NumberOfEntries--;
			return ( COMMAND_SUCCESS );
		}
	}

	return ( UNKNOWN_CONNECTION_ID );
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
	/* Generates/resolve device addresses */
	if( Hosted_Resolving_List.NumberOfEntries )
	{

	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
