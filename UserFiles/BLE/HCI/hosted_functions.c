

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hosted_functions.h"
#include "ble_states.h"
#include "security_manager.h"
#include "TimeFunctions.h"
#include "ble_utils.h"


/****************************************************************/
/* Defines		                                                */
/****************************************************************/
#define HOSTED_RESOLVING_LIST_SIZE ( sizeof( Hosted_Resolving_List.Entry ) / sizeof( RESOLVABLE_DESCRIPTOR ) )


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef struct
{
	DEVICE_IDENTITY Id;
	BD_ADDR_TYPE PeerAddr;
	BD_ADDR_TYPE LocalAddr;
	uint8_t PeerAddrValid  :1;
	uint8_t LocalAddrValid :1;
}RESOLVABLE_DESCRIPTOR;


typedef struct
{
	uint8_t NumberOfEntries;
	RESOLVABLE_DESCRIPTOR Entry[MAX_NUMBER_OF_RESOLVING_LIST_ENTRIES];
}HOSTED_RESOLVING_LIST;


#define EVENT_BYTES_SIZE ( sizeof(((HCI_EVENT_PCKT*)NULL)->Event_Code) + \
		sizeof(((HCI_EVENT_PCKT*)NULL)->Parameter_Total_Length) + UINT8_MAX )
typedef struct
{
	HCI_COMMAND_OPCODE OpCode;
	CMD_CALLBACK* CmdCallBack;
	uint8_t ProcessSteps;
	/*  The Host shall be able to accept HCI Event
	  packets with up to 255 octets of data excluding the HCI Event packet header. The
	  The HCI Event packet header is the first 2 octets of the packet. So, this
	  EventBytes[257] should be the ideal. */
	union
	{
		HCI_EVENT_PCKT EventPacket;
		uint8_t EventBytes[EVENT_BYTES_SIZE];
	}__attribute__((packed));
}ASYNC_COMMAND;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static void Transform_Status_To_Command_Event( HCI_EVENT_PCKT* EventPacketPtr );
static CONTROLLER_ERROR_CODES Add_To_Resolving_List( void );
static CONTROLLER_ERROR_CODES Remove_From_Resolving_List( void );
static RESOLVABLE_DESCRIPTOR* Get_Resolvable_Descriptor( IDENTITY_ADDRESS* PtrId );
static RESOLVABLE_DESCRIPTOR* Get_Resolvable_Descriptor_From_Index( uint8_t Index );
static BD_ADDR_TYPE* Get_Peer_Resolvable_Address( IDENTITY_ADDRESS* PtrId );
static BD_ADDR_TYPE* Get_Local_Resolvable_Address( IDENTITY_ADDRESS* PtrId );
static void Check_Private_Addr(uint8_t resolvstatus, CONTROLLER_ERROR_CODES status);


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static uint8_t Address_Resol_Controller_Cmd = FALSE;
static uint8_t Address_Resol_Controller = FALSE;
static HOSTED_RESOLVING_LIST Hosted_Resolving_List;
static DEVICE_IDENTITY Add_Device;
static IDENTITY_ADDRESS Remove_Device;
static IDENTITY_ADDRESS Read_Peer_Resolvable_Device;
static IDENTITY_ADDRESS Read_Local_Resolvable_Device;
static IDENTITY_ADDRESS Update_Device;
static uint16_t RPA_Timeout_Cmd;
static uint16_t RPA_Timeout = 900; /* Default value is 900 seconds (15 minutes) */
static ASYNC_COMMAND CommandToProcess;
static uint32_t TimeCounter = 0;


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
/* Hosted_LE_Read_Peer_Resolvable_Address()         	  	    */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Read_Peer_Resolvable_Address(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	if( Status == TRANSFER_DONE )
	{
		/* The command parameters are catch in the very request. */
		HCI_SERIAL_COMMAND_PCKT* Packet = (HCI_SERIAL_COMMAND_PCKT*)( DataPtr );

		Read_Peer_Resolvable_Device.Type = Packet->CmdPacket.Parameter[0];
		memcpy( &Read_Peer_Resolvable_Device.Address.Bytes[0], &Packet->CmdPacket.Parameter[1], sizeof(BD_ADDR_TYPE) );

		return (TRUE);
	}
	return (FALSE);
}


/****************************************************************/
/* Hosted_LE_Read_Local_Resolvable_Address()         	  	    */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Read_Local_Resolvable_Address(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	if( Status == TRANSFER_DONE )
	{
		/* The command parameters are catch in the very request. */
		HCI_SERIAL_COMMAND_PCKT* Packet = (HCI_SERIAL_COMMAND_PCKT*)( DataPtr );

		Read_Local_Resolvable_Device.Type = Packet->CmdPacket.Parameter[0];
		memcpy( &Read_Local_Resolvable_Device.Address.Bytes[0], &Packet->CmdPacket.Parameter[1], sizeof(BD_ADDR_TYPE) );

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
/* Hosted_LE_Set_Resolvable_Private_Address_Timeout()      	    */
/* Purpose: Intercepter for this command when it is not 		*/
/* supported by the controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_LE_Set_Resolvable_Private_Address_Timeout(void* DataPtr, uint16_t DataSize, TRANSFER_STATUS Status)
{
	if( Status == TRANSFER_DONE )
	{
		HCI_SERIAL_COMMAND_PCKT* Packet = (HCI_SERIAL_COMMAND_PCKT*)( DataPtr );

		/* The command parameters are catch in the very request. */
		RPA_Timeout_Cmd = ( Packet->CmdPacket.Parameter[1] << 8 ) | ( Packet->CmdPacket.Parameter[0] );
		return (TRUE);
	}
	return (FALSE);
}


/****************************************************************/
/* Hosted_Address_Resolution_Status()         		   	        */
/* Purpose: Check if resolution is enabled.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Hosted_Address_Resolution_Status( void )
{
	return ( ( Address_Resol_Controller ) && ( Get_Local_Version_Information()->HCI_Version <= CORE_SPEC_4_1 ) );
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
	case HCI_LE_SET_SCAN_ENABLE:
		/* This command is "faked" and is used to check the advertising report. */
		if( !CommandToProcess.OpCode.Val ) /* Make sure any processing is happening on the list right now */
		{
			/* No callback is needed */
			CommandToProcess.OpCode.Val = HCI_LE_SET_SCAN_ENABLE;
			CommandToProcess.ProcessSteps = 0;
			memcpy( &CommandToProcess.EventPacket.Event_Code, &EventPacketPtr->Event_Code, (uint16_t)( EventPacketPtr->Parameter_Total_Length + 2 ) );
		}
		break;

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
			if( !CommandToProcess.OpCode.Val ) /* We will need additional processing, so make sure it is available */
			{
				EventPacketPtr->Event_Parameter[3] = Add_To_Resolving_List();
			}else
			{
				EventPacketPtr->Event_Parameter[3] = CONTROLLER_BUSY;
			}
		}else
		{
			EventPacketPtr->Event_Parameter[3] = MEM_CAPACITY_EXCEEDED;
		}


		if( EventPacketPtr->Event_Parameter[3] == COMMAND_SUCCESS )
		{
			/* This requires additional processing. So enqueue the parameters. */
			CommandToProcess.OpCode.Val = HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST;
			CommandToProcess.CmdCallBack = CmdCallBack;
			CommandToProcess.ProcessSteps = 0;
			Update_Device = Add_Device.Peer_Identity_Address;
			memcpy( &CommandToProcess.EventPacket.Event_Code, &EventPacketPtr->Event_Code, (uint16_t)( EventPacketPtr->Parameter_Total_Length + 2 ) );
		}else
		{
			Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );
		}

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
			if( !CommandToProcess.OpCode.Val ) /* Make sure any processing is happening on the list right now */
			{
				EventPacketPtr->Event_Parameter[3] = Remove_From_Resolving_List();
			}else
			{
				EventPacketPtr->Event_Parameter[3] = CONTROLLER_BUSY;
			}
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
		}else if( !CommandToProcess.OpCode.Val )
		{
			Hosted_Resolving_List.NumberOfEntries = 0;
			EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
		}else
		{
			EventPacketPtr->Event_Parameter[3] = CONTROLLER_BUSY;
		}
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );

		break;

	case HCI_LE_READ_RESOLVING_LIST_SIZE:

		EventPacketPtr->Parameter_Total_Length = 5;
		Transform_Status_To_Command_Event( EventPacketPtr );
		if( ( CommandToProcess.OpCode.Val != HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST ) &&
				( CommandToProcess.OpCode.Val != HCI_LE_REMOVE_DEVICE_FROM_RESOLVING_LIST ) &&
				( CommandToProcess.OpCode.Val != HCI_LE_CLEAR_RESOLVING_LIST ) )
		{
			EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
			EventPacketPtr->Event_Parameter[4] = HOSTED_RESOLVING_LIST_SIZE;
		}else
		{
			/* There is a command that possibly changes the size, so first wait for it to finish */
			EventPacketPtr->Event_Parameter[3] = CONTROLLER_BUSY;
			EventPacketPtr->Event_Parameter[4] = 0;
		}
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );

		break;

	case HCI_LE_READ_PEER_RESOLVABLE_ADDRESS:
		EventPacketPtr->Parameter_Total_Length = 4 + sizeof( BD_ADDR_TYPE );
		Transform_Status_To_Command_Event( EventPacketPtr );

		/* This command is used to get the current peer Resolvable Private Address being
		used for the corresponding peer Public and Random (static) Identity Address.
		The peer’s resolvable address being used may change after the command is called.
		This command may be used at any time. When a Controller cannot find a Resolvable
		Private Address associated with the Peer Identity Address, or if the Peer Identity
		Address cannot be found in the resolving list, it shall return the error code Unknown
		Connection Identifier (0x02). */

		if ( Read_Peer_Resolvable_Device.Type & 0xFE ) /* Check if parameter values are OK */
		{
			EventPacketPtr->Event_Parameter[3] = INVALID_HCI_COMMAND_PARAMETERS;
		}else
		{
			BD_ADDR_TYPE* AddrPtr = Get_Peer_Resolvable_Address( &Read_Peer_Resolvable_Device );

			if ( AddrPtr != NULL )
			{
				/* Make sure no command is ongoing (and possibly) updating the peer address */
				if( !CommandToProcess.OpCode.Val )
				{
					/* Enqueue an update for the peer address, no callback is needed */
					CommandToProcess.OpCode.Val = HCI_LE_READ_PEER_RESOLVABLE_ADDRESS;
					CommandToProcess.ProcessSteps = 0;
					Update_Device = Read_Peer_Resolvable_Device;
					EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
					memcpy( &( EventPacketPtr->Event_Parameter[4] ), &( AddrPtr->Bytes[0] ), sizeof(BD_ADDR_TYPE) );
				}else
				{
					EventPacketPtr->Event_Parameter[3] = CONTROLLER_BUSY;
				}
			}else
			{
				EventPacketPtr->Event_Parameter[3] = UNKNOWN_CONNECTION_ID;
			}
		}
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );

		break;

	case HCI_LE_READ_LOCAL_RESOLVABLE_ADDRESS:
		EventPacketPtr->Parameter_Total_Length = 4 + sizeof( BD_ADDR_TYPE );
		Transform_Status_To_Command_Event( EventPacketPtr );

		/* This command is used to get the current local Resolvable Private Address being used
		for the corresponding peer Identity Address. The local resolvable address being used
		may change after the command is called. This command may be used at any time.
		When a Controller cannot find a Resolvable Private Address associated with
		the Peer Identity Address, or if the Peer Identity Address cannot be found in the
		resolving list, it shall return the error code Unknown Connection Identifier
		(0x02). */

		if ( Read_Local_Resolvable_Device.Type & 0xFE ) /* Check if parameter values are OK */
		{
			EventPacketPtr->Event_Parameter[3] = INVALID_HCI_COMMAND_PARAMETERS;
		}else
		{
			BD_ADDR_TYPE* AddrPtr = Get_Local_Resolvable_Address( &Read_Local_Resolvable_Device );

			if ( AddrPtr != NULL )
			{
				/* Make sure no command is ongoing (and possibly) updating the local address */
				if( !CommandToProcess.OpCode.Val )
				{
					/* Enqueue an update for the local address, no callback is needed */
					CommandToProcess.OpCode.Val = HCI_LE_READ_LOCAL_RESOLVABLE_ADDRESS;
					CommandToProcess.ProcessSteps = 0;
					Update_Device = Read_Local_Resolvable_Device;
					EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
					memcpy( &( EventPacketPtr->Event_Parameter[4] ), &( AddrPtr->Bytes[0] ), sizeof(BD_ADDR_TYPE) );
				}else
				{
					EventPacketPtr->Event_Parameter[3] = CONTROLLER_BUSY;
				}
			}else
			{
				EventPacketPtr->Event_Parameter[3] = UNKNOWN_CONNECTION_ID;
			}
		}
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
		}else if( !CommandToProcess.OpCode.Val )
		{
			Address_Resol_Controller = Address_Resol_Controller_Cmd;
			EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
		}else
		{
			EventPacketPtr->Event_Parameter[3] = CONTROLLER_BUSY;
		}
		Command_Complete_Handler( OpCode, CmdCallBack, EventPacketPtr );

		break;

	case HCI_LE_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT:

		EventPacketPtr->Parameter_Total_Length = 4;
		Transform_Status_To_Command_Event( EventPacketPtr );

		/* This command set the length of time the Controller uses a Resolvable Private Address
		before a new resolvable private address is generated and starts being used.
		This timeout applies to all resolvable private addresses generated by the Controller. */

		/* Time range: 1 s to 1 hour (3600 s) */
		if( ( RPA_Timeout_Cmd >= 1 ) && ( RPA_Timeout_Cmd <= 3600 ) ) /* Check if parameter values are OK */
		{
			RPA_Timeout = RPA_Timeout_Cmd;
			EventPacketPtr->Event_Parameter[3] = COMMAND_SUCCESS;
		}else
		{
			EventPacketPtr->Event_Parameter[3] = INVALID_HCI_COMMAND_PARAMETERS;
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
		if ( memcmp( &Hosted_Resolving_List.Entry[i].Id.Peer_Identity_Address, &Add_Device.Peer_Identity_Address, sizeof(IDENTITY_ADDRESS) ) == 0 )
		{
			return ( INVALID_HCI_COMMAND_PARAMETERS );
		}
	}

	Hosted_Resolving_List.Entry[ Hosted_Resolving_List.NumberOfEntries ].Id = Add_Device;
	Hosted_Resolving_List.Entry[ Hosted_Resolving_List.NumberOfEntries ].LocalAddrValid = FALSE;
	Hosted_Resolving_List.Entry[ Hosted_Resolving_List.NumberOfEntries ].PeerAddrValid = FALSE;
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
		if ( memcmp( &Hosted_Resolving_List.Entry[i].Id.Peer_Identity_Address, &Remove_Device, sizeof(IDENTITY_ADDRESS) ) == 0 )
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
/* Get_Resolvable_Descriptor()         	    	   		        */
/* Purpose: Return the descriptor from the list.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static RESOLVABLE_DESCRIPTOR* Get_Resolvable_Descriptor( IDENTITY_ADDRESS* PtrId )
{
	for ( uint8_t i = 0; i < Hosted_Resolving_List.NumberOfEntries; i++ )
	{
		/* Check if this entry is in the list */
		if ( memcmp( &Hosted_Resolving_List.Entry[i].Id.Peer_Identity_Address, PtrId, sizeof(IDENTITY_ADDRESS) ) == 0 )
		{
			return ( &Hosted_Resolving_List.Entry[i] );
		}
	}

	return (NULL);
}


/****************************************************************/
/* Get_Resolvable_Descriptor_From_Index()         	 	        */
/* Purpose: Return the descriptor from the list based on passed */
/* index.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static RESOLVABLE_DESCRIPTOR* Get_Resolvable_Descriptor_From_Index( uint8_t Index )
{
	if ( Index < Hosted_Resolving_List.NumberOfEntries )
	{
		return ( &Hosted_Resolving_List.Entry[Index] );
	}

	return (NULL);
}


/****************************************************************/
/* Get_Peer_Resolvable_Address()             	   		        */
/* Purpose: Return peer device from resolving list.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static BD_ADDR_TYPE* Get_Peer_Resolvable_Address( IDENTITY_ADDRESS* PtrId )
{
	RESOLVABLE_DESCRIPTOR* Descriptor = Get_Resolvable_Descriptor(PtrId);

	if( Descriptor != NULL )
	{
		if( Descriptor->PeerAddrValid )
		{
			return ( &Descriptor->PeerAddr );
		}
	}

	return (NULL);
}


/****************************************************************/
/* Get_Local_Resolvable_Address()             	   		        */
/* Purpose: Return peer device from resolving list.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static BD_ADDR_TYPE* Get_Local_Resolvable_Address( IDENTITY_ADDRESS* PtrId )
{
	RESOLVABLE_DESCRIPTOR* Descriptor = Get_Resolvable_Descriptor(PtrId);

	if( Descriptor != NULL )
	{
		if( Descriptor->LocalAddrValid )
		{
			return ( &Descriptor->LocalAddr );
		}
	}

	return (NULL);
}


/****************************************************************/
/* Check_Private_Addr()      									*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Check_Private_Addr(uint8_t resolvstatus, CONTROLLER_ERROR_CODES status)
{
	if( CommandToProcess.ProcessSteps == 4 )
	{
		TimeCounter = 0;
		if( status == COMMAND_SUCCESS )
		{
			CommandToProcess.ProcessSteps = resolvstatus ? 5 : 6; /* 5: status is true / 6: status is false */
		}else
		{
			CommandToProcess.ProcessSteps = 6; /* Could not be resolved due to controller failure */
		}
	}
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
	static uint32_t DelayCounter = 0;
	static RESOLVABLE_DESCRIPTOR* Desc;
	static uint8_t RenewRPAs = FALSE;
	static uint8_t EntriesCounter = 0;
	static uint8_t Num_Reports;
	static uint8_t RecordCounter;
	static uint8_t* Address_Type_Ptr;
	static uint8_t* Event_Type_Ptr;
	static BD_ADDR_TYPE* Address_Ptr;
	BD_ADDR_TYPE* Addr;

	/* Generates/resolve device addresses */
	if( Hosted_Resolving_List.NumberOfEntries )
	{
		if( !RenewRPAs )
		{
			if ( TimeBase_DelayMs( &DelayCounter, (uint32_t)( RPA_Timeout * 1000 ), TRUE ) )
			{
				RenewRPAs = TRUE;
				EntriesCounter = 0;
			}
		}else if( !CommandToProcess.OpCode.Val ) /* Is it free? */
		{
			if( EntriesCounter < Hosted_Resolving_List.NumberOfEntries )
			{
				CommandToProcess.OpCode.Val = TRUE;
				CommandToProcess.ProcessSteps = 0;
				Update_Device = Hosted_Resolving_List.Entry[EntriesCounter].Id.Peer_Identity_Address;
				EntriesCounter++;
				/* TODO: Important note: the Core_v5.2 specification manual, Page 2566, mentioned
				 * that the new RPA "is generated and starts being used". We just generate the new
				 * RPAs here, we do not substitute whatever RPA used by the Controller since for this
				 * we should know the Device Identity entry the Controller is currently using. For
				 * this to happen, we should read the Random Address from the controller and resolve
				 * it (if RPA) and then substitute by the newly generated address. For controller
				 * versions above 4.1, all these hosted simulated functions are not used and this
				 * is not a problem. For version 4.1 and below, the Host shall periodically restart
				 * the advertising (or change its address) to comply with privacy and being constantly
				 * changing its RPA. Making this happens inside this simulated controller functions
				 * is too hard. For example, if we use a direct advertising, we should substitute
				 * the own and peer addresses in the controller. This may be done for own address
				 * by setting HCI_LE_Set_Random_Address but cannot be done for the peer address without
				 * sending again the advertising parameters command. So this implementation only generates
				 * new values just to be different from the previous one, making sure times to time the
				 * RPAs change. */
			}else
			{
				RenewRPAs = FALSE;
			}
		}
	}else
	{
		DelayCounter = 0;
		EntriesCounter = 0;
		RenewRPAs = FALSE;
	}


	/* Terminate commands that need more processing */
	switch( CommandToProcess.OpCode.Val )
	{

	case HCI_LE_SET_SCAN_ENABLE:
	{
		switch( CommandToProcess.ProcessSteps )
		{
		case 0:
			Num_Reports = CommandToProcess.EventPacket.Event_Parameter[1];
			Event_Type_Ptr = &CommandToProcess.EventPacket.Event_Parameter[2];
			Address_Type_Ptr = &(CommandToProcess.EventPacket.Event_Parameter[Num_Reports + 2]);
			Address_Ptr = (BD_ADDR_TYPE*)(&(CommandToProcess.EventPacket.Event_Parameter[ ( Num_Reports * 2 ) + 2 ]) );
			CommandToProcess.ProcessSteps = 1;
			break;

		case 1:
			if( Num_Reports > 0 )
			{
				/* Only RANDOM_DEV_ADDR may be resolved */
				if( ( (ADDRESS_TYPE)( Address_Type_Ptr[Num_Reports - 1] ) ) == RANDOM_DEV_ADDR )
				{
					if( ( Address_Ptr[Num_Reports - 1].Bytes[sizeof(BD_ADDR_TYPE) - 1] & 0xC0 ) == 0x40 )
					{
						/* This is a resolvable private address, we should try to resolve it */
						RecordCounter = 0;
						CommandToProcess.ProcessSteps = 2;
					}else
					{
						/* This is not RPA, so we will not try to resolve it */
						Num_Reports--;
					}
				}else
				{
					/* This address is not random, we don't need to do anything about it */
					Num_Reports--;
				}
			}else
			{
				LE_Advertising_Report_Handler( &CommandToProcess.EventPacket );
				CommandToProcess.OpCode.Val = 0;
			}
			break;

		case 2:
			Desc = Get_Resolvable_Descriptor_From_Index( RecordCounter );
			if( Desc != NULL )
			{
				RecordCounter++;
				if( memcmp( &Address_Ptr[Num_Reports - 1].Bytes[0], &Desc->Id.Peer_Identity_Address.Address.Bytes[0], sizeof(BD_ADDR_TYPE) ) != 0 )
				{
					/* Check if IRK values are NULL */
					if( Event_Type_Ptr[Num_Reports - 1] == ADV_DIRECT_IND_EVT )
					{
						/* This should use local IRK, although a directed RPA should never get here since it would have been blocked
						 * because TargetA is different from this device address. */
						if( !Check_NULL_IRK(&Desc->Id.Local_IRK)  )
						{
							TimeCounter = 0;
							CommandToProcess.ProcessSteps = 3;
						}else
						{
							/* The local IRK is null, so no address resolution is possible
							 * Load the next address from the advertising report */
							Num_Reports--;
							CommandToProcess.ProcessSteps = 1;
						}
					}else if( !Check_NULL_IRK(&Desc->Id.Peer_IRK) )
					{
						/* These shall use peer IRK values */
						TimeCounter = 0;
						CommandToProcess.ProcessSteps = 3;
					}else
					{
						/* The peer IRK is null, so no address resolution is possible
						 * Load the next address from the advertising report */
						Num_Reports--;
						CommandToProcess.ProcessSteps = 1;
					}
				}else
				{
					/* The address is equal to the resolving list entry. There is no need to resolve anything.
					 * Load the next address from the advertising report */
					Num_Reports--;
					CommandToProcess.ProcessSteps = 1;
				}
			}else
			{
				/* Resolving list ended: Load the next address from the advertising report */
				Num_Reports--;
				CommandToProcess.ProcessSteps = 1;
			}
			break;

		case 3:
		{
			/* ADV_DIRECT_IND_EVT should use local IRK, although a directed RPA should never get here since it would have been blocked
			 * because TargetA is different from this device. */
			/* These shall use peer IRK values: */
			/* ADV_IND_EVT */
			/* ADV_SCAN_IND_EVT */
			/* ADV_NONCONN_IND_EVT */
			/* SCAN_RSP_EVT */
			IRK_TYPE* Local_IRK_Ptr = ( Event_Type_Ptr[Num_Reports - 1] == ADV_DIRECT_IND_EVT ) ? &Desc->Id.Local_IRK : &Desc->Id.Peer_IRK;

			CommandToProcess.ProcessSteps = 4;
			if( Resolve_Private_Address( Get_Supported_Commands(), &Address_Ptr[Num_Reports - 1], Local_IRK_Ptr, 1, &Check_Private_Addr ) )
			{
				TimeCounter = 0;
			}else
			{
				Cancel_Private_Address_Resolution();
				/* End resolution for this address */
				CommandToProcess.ProcessSteps = 2;
			}
		}
		break;

		case 4: /* Wait for address resolution to end */
			if( TimeBase_DelayMs( &TimeCounter, 150, TRUE ) )
			{
				Cancel_Private_Address_Resolution();
				/* End resolution for this resolving record */
				CommandToProcess.ProcessSteps = 2;
			}
			break;

		case 5: /* Successful address resolution */
			/* Load the type of identity just resolved */
			Address_Type_Ptr[Num_Reports - 1] = ( Desc->Id.Peer_Identity_Address.Type == PEER_PUBLIC_DEV_ADDR ) ? PUBLIC_IDENTITY_ADDR : RANDOM_IDENTITY_ADDR;
			/* Load the identity into the address itself since we have resolved it */
			Address_Ptr[Num_Reports - 1] = Desc->Id.Peer_Identity_Address.Address;
			/* Load the next address from the advertising report */
			Num_Reports--;
			CommandToProcess.ProcessSteps = 1;
			break;

		case 6: /* Failed resolvable address, that is, the address does not "match" with the resolving record */
			/* Simply go to the next resolving record */
			/* End resolution for this resolving record */
			CommandToProcess.ProcessSteps = 2;
			break;
		}
	}
	break;

	case TRUE:
	case HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST:
	{
		switch ( CommandToProcess.ProcessSteps )
		{
		case 0:
			Desc = Get_Resolvable_Descriptor( &Update_Device );
			CommandToProcess.ProcessSteps = 1;
			break;

		case 1:
			Addr = Generate_Device_Address( Get_Supported_Commands(), RESOLVABLE_PRIVATE, &Desc->Id.Local_IRK, 3 );
			if ( Addr != NULL )
			{
				Desc->LocalAddr = *Addr;
				Desc->LocalAddrValid = TRUE;
				CommandToProcess.ProcessSteps = 2;

				if( Check_NULL_IRK(&Desc->Id.Peer_IRK) )
				{
					/* The peer IRK is null, use the peer's identity for the address */
					Desc->PeerAddr = Desc->Id.Peer_Identity_Address.Address;
					Desc->PeerAddrValid = TRUE;
					if( CommandToProcess.OpCode.Val == HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST )
					{
						Command_Complete_Handler( CommandToProcess.OpCode, CommandToProcess.CmdCallBack, &CommandToProcess.EventPacket );
					}
					CommandToProcess.OpCode.Val = 0;
				}
			}
			break;

		case 2:
			Addr = Generate_Device_Address( Get_Supported_Commands(), RESOLVABLE_PRIVATE, &Desc->Id.Peer_IRK, 4 );
			if ( Addr != NULL )
			{
				Desc->PeerAddr = *Addr;
				Desc->PeerAddrValid = TRUE;
				if( CommandToProcess.OpCode.Val == HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST )
				{
					Command_Complete_Handler( CommandToProcess.OpCode, CommandToProcess.CmdCallBack, &CommandToProcess.EventPacket );
				}
				CommandToProcess.OpCode.Val = 0;
			}
			break;
		}
	}
	break;

	case HCI_LE_READ_PEER_RESOLVABLE_ADDRESS:
	{
		switch ( CommandToProcess.ProcessSteps )
		{
		case 0:
			Desc = Get_Resolvable_Descriptor( &Update_Device );
			CommandToProcess.ProcessSteps = 1;

			if( Check_NULL_IRK(&Desc->Id.Peer_IRK) )
			{
				/* The peer IRK is null, use the peer's identity for the address */
				Desc->PeerAddr = Desc->Id.Peer_Identity_Address.Address;;
				Desc->PeerAddrValid = TRUE;
				CommandToProcess.OpCode.Val = 0;
			}
			break;

		case 1:
			Addr = Generate_Device_Address( Get_Supported_Commands(), RESOLVABLE_PRIVATE, &Desc->Id.Peer_IRK, 5 );
			if ( Addr != NULL )
			{
				Desc->PeerAddr = *Addr;
				Desc->PeerAddrValid = TRUE;
				CommandToProcess.OpCode.Val = 0;
			}
			break;
		}
	}
	break;

	case HCI_LE_READ_LOCAL_RESOLVABLE_ADDRESS:
	{
		switch ( CommandToProcess.ProcessSteps )
		{
		case 0:
			Desc = Get_Resolvable_Descriptor( &Update_Device );
			CommandToProcess.ProcessSteps = 1;
			break;

		case 1:
			Addr = Generate_Device_Address( Get_Supported_Commands(), RESOLVABLE_PRIVATE, &Desc->Id.Local_IRK, 6 );
			if ( Addr != NULL )
			{
				Desc->LocalAddr = *Addr;
				Desc->LocalAddrValid = TRUE;
				CommandToProcess.OpCode.Val = 0;
			}
			break;
		}
	}
	break;

	default:
		CommandToProcess.ProcessSteps = 0;
		CommandToProcess.OpCode.Val = 0;
		break;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
