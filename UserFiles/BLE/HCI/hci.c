

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "hci_transport_layer.h"
#include "Types.h"


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
/* HCI_Disconnect()                         					*/
/* Location: Page 1940 Core_v5.2 								*/
/* Purpose: The HCI_Disconnect command is used to terminate an 	*/
/* existing connection. The Connection_Handle command parameter */
/* indicates which connection is to be disconnected. The Reason */
/* command parameter indicates the reason for ending the 		*/
/* connection and is copied into the error code field of the	*/
/* LMP_DETACH PDU on a BR/EDR connection or the error code field*/
/* of the LL_TERMINATE_IND PDU, or the LL_CIS_TERMINATE_IND PDU */
/* on an LE connection. All SCO, eSCO, and CIS connections on a */
/* physical link should be disconnected before the ACL 			*/
/* connection on the same physical connection is disconnected. 	*/
/* If it does not, they will be implicitly disconnected as part */
/* of the ACL disconnection. If this command is used to 		*/
/* disconnect a CIS, the connection handle of the CIS and the	*/
/* associated data paths of the CIS shall remain valid. If this */
/* command is issued for a CIS before the Controller has		*/
/* generated the HCI_CIS_Established event for that CIS, the 	*/
/* Controller shall return the error code Command Disallowed 	*/
/* (0x0C). Note: The Host can recreate a disconnected CIS at a 	*/
/* later point in time using the same connection handle.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Disconnect( uint16_t Connection_Handle, CONTROLLER_ERROR_CODES Reason )
{
	uint8_t Status = FALSE;

	if( Connection_Handle <= MAX_CONNECTION_HANDLE )
	{

		switch( Reason )
		{
		/* Not sure if other reason codes are supported. In the command specification, it
		 * explicit mentioned those: */
		case AUTHENTICATION_FAILURE:
		case REMOTE_USER_TERMINATED_CONNECTION:
		case REM_DEV_TERM_CONN_LOW_RESOURCES:
		case REM_DEV_TERM_CONN_POWER_OFF:
		case UNSUPPORTED_REMOTE_OR_LMP_FEATURE:
		case PAIRING_WITH_UNIT_KEY_NOT_SUPPORTED:
		case UNACCEPTABLE_CONNECTION_PARAMETERS:
			break;

		default:
			return (Status);
			break;
		}

		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 3;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_DISCONNECT;
		PcktPtr->CmdPacket.Parameter_Total_Length = 3;

		PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
		PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;
		PcktPtr->CmdPacket.Parameter[2] = Reason;

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL );

		free( PcktPtr );

	}

	return (Status);
}


/****************************************************************/
/* HCI_Disconnect_Status()        	        					*/
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Disconnect command and HCI_Disconnection_Complete not	*/
/* issued yet.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Disconnect_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Remote_Version_Information()                        */
/* Location: Page 1974 Core_v5.2 								*/
/* Purpose: This command will obtain the values for the version */
/* information for the remote device identified by the 			*/
/* Connection_Handle parameter. The Connection_Handle shall be 	*/
/* a Connection_Handle for an ACL-U or LE-U logical link.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Read_Remote_Version_Information( uint16_t Connection_Handle )
{
	uint8_t Status = FALSE;

	if( Connection_Handle <= MAX_CONNECTION_HANDLE )
	{
		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 2;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_READ_REMOTE_VERSION_INFORMATION;
		PcktPtr->CmdPacket.Parameter_Total_Length = 2;

		PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
		PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_Read_Remote_Version_Information_Status()          	    */
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Read_Remote_Version_Information command.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Remote_Version_Information_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Set_Event_Mask()                        					*/
/* Location: Page 2074 Core_v5.2 								*/
/* Purpose: The HCI_Set_Event_Mask command is used to control 	*/
/* which events are generated by the HCI for the Host. If the 	*/
/* bit in the Event_Mask is set to a one, then the event 		*/
/* associated with that bit will be enabled. For an LE 			*/
/* Controller, the �LE Meta event� bit in the event_Mask shall 	*/
/* enable or disable all LE events in the LE Meta event			*/
/* (see Section 7.7.65). The event mask allows the Host to 		*/
/* control how much it is interrupted.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Set_Event_Mask( EVENT_MASK Event_Mask )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + sizeof(EVENT_MASK);
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_SET_EVENT_MASK;
	PcktPtr->CmdPacket.Parameter_Total_Length = sizeof(EVENT_MASK);

	for( int8_t i = 0; i < sizeof(EVENT_MASK); i++ )
	{
		PcktPtr->CmdPacket.Parameter[i] = Event_Mask.Bytes[i];
	}

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_Set_Event_Mask_Status()          	    				*/
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Set_Event_Mask command.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Set_Event_Mask_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Set_Event_Mask_Complete()          	    				*/
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Set_Event_Mask command.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Transmit_Power_Level()             					*/
/* Location: Page 2121 Core_v5.2 								*/
/* Purpose: This command reads the values for the TX_Power_Level*/
/* parameter for the specified Connection_Handle. The 			*/
/* Connection_Handle shall be a Connection_Handle for an ACL 	*/
/* connection.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Read_Transmit_Power_Level( uint16_t Connection_Handle, uint8_t Type )
{
	uint8_t Status = FALSE;

	if( ( Connection_Handle <= MAX_CONNECTION_HANDLE ) && ( Type <= 1 ) )
	{
		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 3;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_READ_TRANSMIT_POWER_LEVEL;
		PcktPtr->CmdPacket.Parameter_Total_Length = 3;

		PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
		PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;
		PcktPtr->CmdPacket.Parameter[2] = Type;

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_Read_Transmit_Power_Level_Status()          	    		*/
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Read_Transmit_Power_Level command.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Transmit_Power_Level_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Transmit_Power_Level_Complete()        	    		*/
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Read_Transmit_Power_Level command.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Transmit_Power_Level_Complete( CONTROLLER_ERROR_CODES Status, uint16_t Connection_Handle, int8_t TX_Power_Level )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Reset()                        							*/
/* Location: Page 2077 Core_v5.2 								*/
/* Purpose: The HCI_Reset command will reset the Controller and */
/* the Link Manager on the BR/EDR Controller, the PAL on an AMP */
/* Controller, or the Link Layer on an LE Controller. If the 	*/
/* Controller supports both BR/EDR and LE then the HCI_Reset	*/
/* command shall reset the Link Manager, Baseband and Link 		*/
/* Layer. The HCI_Reset command shall not affect the used HCI 	*/
/* transport layer since the HCI transport layers may have reset*/
/* mechanisms of their own. After the reset is completed, the 	*/
/* current operational state will be lost, the Controller will 	*/
/* enter standby mode and the Controller will automatically 	*/
/* revert to the default values for the parameters for which 	*/
/* default values are defined in the specification. Note: The 	*/
/* HCI_Reset command will not necessarily perform a hardware 	*/
/* reset. This is implementation defined. The Host shall not 	*/
/* send additional HCI commands before the HCI_Command_Complete */
/* event related to the HCI_Reset command has been received.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Reset( void )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_RESET;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL );

	return (Status);
}


/****************************************************************/
/* HCI_Reset_Status()          	    							*/
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Reset command.											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Reset_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Reset_Complete()          	    						*/
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Reset command.											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Reset_Complete( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Local_Version_Information()                         */
/* Location: Page 2223 Core_v5.2 								*/
/* Purpose: The HCI_Read_Local_Version_Information command 		*/
/* reads the values for the version information for the local	*/
/* Controller.   		    									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Read_Local_Version_Information( void )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_READ_LOCAL_VERSION_INFORMATION;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL );

	return (Status);
}


/****************************************************************/
/* HCI_Read_Local_Version_Information_Status()          	    */
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Read_Local_Version_Information command.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Local_Version_Information_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Local_Version_Information_Complete()                */
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Read_Local_Version_Information command.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Local_Version_Information_Complete( CONTROLLER_ERROR_CODES Status,
		HCI_VERSION HCI_Version, uint16_t HCI_Revision,
		uint8_t LMP_PAL_Version, uint16_t Manufacturer_Name,
		uint16_t LMP_PAL_Subversion)
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Local_Supported_Commands()                          */
/* Location: Page 2225 Core_v5.2 								*/
/* Purpose: This command reads the list of HCI commands 		*/
/* supported for the local Controller. This command shall 		*/
/* return the Supported_Commands configuration parameter. It is */
/* implied that if a command is listed as supported, the 		*/
/* feature underlying that command is also supported.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Read_Local_Supported_Commands( void )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_READ_LOCAL_SUPPORTED_COMMANDS;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL );

	return (Status);
}


/****************************************************************/
/* HCI_Read_Local_Supported_Commands_Status()          		    */
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Read_Local_Supported_Commands command.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Local_Supported_Commands_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Local_Supported_Commands_Complete()        		    */
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_Read_Local_Supported_Commands command.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Local_Supported_Commands_Complete( CONTROLLER_ERROR_CODES Status, SUPPORTED_COMMANDS* Supported_Commands )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Local_Supported_Features()                          */
/* Location: Page 2226 Core_v5.2 								*/
/* Purpose: This command requests a list of the supported 		*/
/* features for the local BR/EDR Controller. This command will 	*/
/* return a list of the LMP features. For details see [Vol 2] 	*/
/* Part C, Link Manager Protocol Specification.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Read_Local_Supported_Features( void )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_READ_LOCAL_SUPPORTED_FEATURES;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL );

	return (Status);
}


/****************************************************************/
/* HCI_Read_Local_Supported_Features_Status()                   */
/* Location: 					 								*/
/* Purpose: Event generated by 									*/
/* the HCI_Read_Local_Supported_Features command.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Local_Supported_Features_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_Local_Supported_Features_Complete()                 */
/* Location: 					 								*/
/* Purpose: Event generated by 									*/
/* the HCI_Read_Local_Supported_Features command.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_Local_Supported_Features_Complete( CONTROLLER_ERROR_CODES Status, SUPPORTED_FEATURES* LMP_Features )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_BD_ADDR()                          					*/
/* Location: Page 2231 Core_v5.2 								*/
/* Purpose: On a BR/EDR Controller, this command reads the 		*/
/* Bluetooth Controller address (BD_ADDR). (See [Vol 2] Part B, */
/* Section 1.2 and [Vol 3] Part C, Section 3.2.1). On an LE 	*/
/* Controller, this command shall read the Public Device Address*/
/* as defined in [Vol 6] Part B, Section 1.3. If this Controller*/
/* does not have a Public Device Address, the value 			*/
/* 0x000000000000 shall be returned. On a BR/EDR/LE Controller, */
/* the public address shall be the same as the BD_ADDR.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Read_BD_ADDR( void )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_READ_BD_ADDR;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL );

	return (Status);
}


/****************************************************************/
/* HCI_Read_BD_ADDR_Status()                					*/
/* Location: 					 								*/
/* Purpose: Event generated by 									*/
/* the HCI_Read_BD_ADDR command.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_BD_ADDR_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_BD_ADDR_Complete()                					*/
/* Location: 					 								*/
/* Purpose: Event generated by 									*/
/* the HCI_Read_BD_ADDR command.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_BD_ADDR_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE BD_ADDR )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_RSSI()                          					*/
/* Location: Page 2249 Core_v5.2 								*/
/* Purpose: This command reads the Received Signal Strength 	*/
/* Indication (RSSI) value from a Controller.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Read_RSSI( uint16_t Handle )
{
	uint8_t Status = FALSE;

	if( Handle <= MAX_CONNECTION_HANDLE )
	{
		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 2;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_READ_RSSI;
		PcktPtr->CmdPacket.Parameter_Total_Length = 2;

		PcktPtr->CmdPacket.Parameter[0] = Handle & 0xFF;
		PcktPtr->CmdPacket.Parameter[1] = ( Handle >> 8 ) & 0xFF;

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_Read_RSSI_Status()                						*/
/* Location: 					 								*/
/* Purpose: Event generated by the HCI_Read_RSSI command.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_RSSI_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Read_RSSI_Complete()                						*/
/* Location: 					 								*/
/* Purpose: Event generated by the HCI_Read_RSSI command.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Read_RSSI_Complete( CONTROLLER_ERROR_CODES Status, uint16_t Handle, int8_t RSSI )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Set_Event_Mask()                   		            */
/* Location: Page 2473 Core_v5.2								*/
/* Purpose: The HCI_LE_Set_Event_Mask command is used to control*/
/* which LE events are generated by the HCI for the Host. If the*/
/* bit in the LE_Event_Mask is set to a one, then the event 	*/
/* associated with that bit will be enabled. The event mask 	*/
/* allows the Host to control which events will interrupt it. 	*/
/* The Controller shall ignore those bits which are reserved for*/
/* future use or represent events which it does not support. If */
/* the Host sets any of these bits to 1, the Controller shall 	*/
/* act as if they were set to 0. For LE events to be generated, */
/* the LE Meta event bit in the Event_Mask shall also be set. 	*/
/* If that bit is not set, then LE events shall not be			*/
/* generated, regardless of how the LE_Event_Mask is set.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Event_Mask( LE_EVENT_MASK LE_Event_Mask )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + sizeof(LE_EVENT_MASK);
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_EVENT_MASK;
	PcktPtr->CmdPacket.Parameter_Total_Length = sizeof(LE_EVENT_MASK);

	for( int8_t i = 0; i < sizeof(LE_EVENT_MASK); i++ )
	{
		PcktPtr->CmdPacket.Parameter[i] = LE_Event_Mask.Bytes[i];
	}

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Event_Mask_Status()                				*/
/* Location: 					 								*/
/* Purpose: Event generated by the HCI_LE_Set_Event_Mask		*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Set_Event_Mask_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Set_Event_Mask_Complete()                				*/
/* Location: 					 								*/
/* Purpose: Event generated by the HCI_LE_Set_Event_Mask		*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Set_Event_Mask_Complete( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Read_Buffer_Size()                  		            */
/* Location: Page 2476 Core_v5.2								*/
/* Purpose: This command is used to read the maximum size of the*/
/* data portion of ACL data packets and isochronous data packets*/
/* sent from the Host to the Controller. The Host shall segment */
/* the data transmitted to the Controller according to these 	*/
/* values so that the HCI Data packets and isochronous data 	*/
/* packets will contain data up to this size. The 				*/
/* HCI_LE_Read_Buffer_Size command also returns the total number*/
/* of HCI LE ACL Data packets and isochronous data packets that */
/* can be stored in the data buffers of the Controller. The 	*/
/* HCI_LE_Read_Buffer_Size command shall be issued by the Host 	*/
/* before it sends any data to an LE Controller 				*/
/* (see Section 4.1.1). If the Controller supports HCI ISO Data */
/* packets, it shall return non-zero values for the 			*/
/* ISO_Data_Packet_Length and Total_Num_ISO_Data_Packets 		*/
/* parameters. If the Controller returns a length value of zero */
/* for ACL data packets, the Host shall use the 				*/
/* HCI_Read_Buffer_Size command to determine the size of the 	*/
/* data buffers (shared between BR/EDR and LE transports). Note:*/
/* Both the HCI_Read_Buffer_Size command and the 				*/
/* HCI_LE_Read_Buffer_Size command may return buffer length and */
/* number of packets parameter values that are nonzero. This 	*/
/* allows a Controller to offer different buffers and number of */
/* buffers for BR/EDR data packets and LE data packets. The 	*/
/* LE_ACL_Data_Packet_Length return parameter shall be used to	*/
/* determine the maximum size of the L2CAP PDU segments that are*/
/* contained in ACL data packets, and which are transferred from*/
/* the Host to the Controller to be broken up into packets by 	*/
/* the Link Layer. The Total_Num_LE_ACL_Data_Packets return 	*/
/* parameter contains the total number of HCI ACL Data packets 	*/
/* that can be stored in the data buffers of the Controller. 	*/
/* The Host determines how to divide the buffers between 		*/
/* different connection handles. The ISO_Data_Packet_Length 	*/
/* return parameter shall be used to determine the maximum size */
/* of the SDU segments that are contained in isochronous data 	*/
/* packets, and which are transferred from the Host to the 		*/
/* Controller. The Total_Num_ISO_Data_Packets return parameter 	*/
/* contains the total number of isochronous data packets that 	*/
/* can be stored in the data buffers of the Controller. The Host*/
/* determines how to divide the buffers between different 		*/
/* connection handle(s). Note: The LE_ACL_Data_Packet_Length and*/
/* ISO_Data_Packet_Length return parameters do not include the 	*/
/* length of the HCI Data packet header or the HCI ISO Data 	*/
/* packet header respectively.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Buffer_Size( void )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_READ_BUFFER_SIZE;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Buffer_Size_Status()                				*/
/* Location: 					 								*/
/* Purpose: Event generated by the HCI_LE_Read_Buffer_Size		*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Read_Buffer_Size_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Read_Buffer_Size_Complete()              				*/
/* Location: 					 								*/
/* Purpose: Event generated by the 								*/
/* HCI_LE_Read_Buffer_Size command.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Read_Buffer_Size_Complete( CONTROLLER_ERROR_CODES Status, uint16_t LE_ACL_Data_Packet_Length,
		uint8_t Total_Num_LE_ACL_Data_Packets )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Set_Advertising_Data()                                */
/* Location: Page 2487 Core_v5.2								*/
/* Purpose: The HCI_LE_Set_Advertising_Data command is used to  */
/* set the data used in advertising packets that have a data 	*/
/* field. Only the significant part of the Advertising_Data 	*/
/* should be transmitted in the advertising packets, as defined */
/* in [Vol 3] Part C, Section 11. If advertising is currently 	*/
/* enabled, the Controller shall use the new data in subsequent */
/* advertising events. If an advertising event is in progress 	*/
/* when this command is issued, the Controller may use the old 	*/
/* or new data for that event. If advertising is currently 		*/
/* disabled, the data shall be kept by the Controller and used 	*/
/* once advertising is enabled.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Advertising_Data( uint8_t Advertising_Data_Length, uint8_t Advertising_Data[] )
{
	uint8_t Status = FALSE;

	if( Advertising_Data_Length <= MAX_ADVERTISING_DATA_LENGTH )
	{

		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + Advertising_Data_Length;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_ADVERTISING_DATA;
		PcktPtr->CmdPacket.Parameter_Total_Length = Advertising_Data_Length;

		for( uint8_t i = 0; i < Advertising_Data_Length; i++ )
		{
			PcktPtr->CmdPacket.Parameter[i] = Advertising_Data[i];
		}

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Advertising_Data_Status()                         */
/* Location: 					 								*/
/* Purpose: Event generated by the HCI_LE_Set_Advertising_Data	*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Set_Advertising_Data_Status( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Set_Advertising_Data_Complete()                       */
/* Location: 					 								*/
/* Purpose: Event generated by the HCI_LE_Set_Advertising_Data	*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Set_Advertising_Data_Complete( CONTROLLER_ERROR_CODES Status )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Hardware_Error()                							*/
/* Location: 					 								*/
/* Purpose: Event used to notify the Host that a hardware		*/
/* failure has occurred in the Controller.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Hardware_Error( BLE_HW_ERROR_CODE Hardware_Code )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
