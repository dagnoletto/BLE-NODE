

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "hci_transport_layer.h"
#include "hosted_functions.h"
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
uint8_t HCI_Disconnect( uint16_t Connection_Handle, CONTROLLER_ERROR_CODES Reason, DefCmdStatus StatusCallBack )
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

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = NULL, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );

	}

	return (Status);
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
uint8_t HCI_Read_Remote_Version_Information( uint16_t Connection_Handle,
		ReadRemoteVerInfoComplete CompleteCallBack, DefCmdStatus StatusCallBack )
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

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_Set_Event_Mask()                        					*/
/* Location: Page 2074 Core_v5.2 								*/
/* Purpose: The HCI_Set_Event_Mask command is used to control 	*/
/* which events are generated by the HCI for the Host. If the 	*/
/* bit in the Event_Mask is set to a one, then the event 		*/
/* associated with that bit will be enabled. For an LE 			*/
/* Controller, the “LE Meta event” bit in the event_Mask shall 	*/
/* enable or disable all LE events in the LE Meta event			*/
/* (see Section 7.7.65). The event mask allows the Host to 		*/
/* control how much it is interrupted.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Set_Event_Mask( EVENT_MASK Event_Mask, DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
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

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
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
uint8_t HCI_Read_Transmit_Power_Level( uint16_t Connection_Handle, uint8_t Type,
		TxPwrLvlComplete CompleteCallBack, DefCmdStatus StatusCallBack )
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

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );
	}

	return (Status);
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
uint8_t HCI_Reset( DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_RESET;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
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
uint8_t HCI_Read_Local_Version_Information( ReadLocalVerInfoComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_READ_LOCAL_VERSION_INFORMATION;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
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
uint8_t HCI_Read_Local_Supported_Commands( ReadLocalSupCmdsComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_READ_LOCAL_SUPPORTED_COMMANDS;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
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
uint8_t HCI_Read_Local_Supported_Features( ReadLocalSupFeaturesComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_READ_LOCAL_SUPPORTED_FEATURES;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
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
uint8_t HCI_Read_BD_ADDR( ReadBDADDRComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_READ_BD_ADDR;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
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
uint8_t HCI_Read_RSSI( uint16_t Handle, ReadRSSIComplete CompleteCallBack, DefCmdStatus StatusCallBack )
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

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );
	}

	return (Status);
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
uint8_t HCI_LE_Set_Event_Mask( LE_EVENT_MASK LE_Event_Mask, DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
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

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
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
uint8_t HCI_LE_Read_Buffer_Size( LEReadBufferSizeComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_READ_BUFFER_SIZE;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Local_Supported_Features()                       */
/* Location: Page 2479 Core_v5.2								*/
/* Purpose: This command requests the list of the supported LE 	*/
/* features for the Controller.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Local_Supported_Features( LEReadLocalSuppFeaturesComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_READ_LOCAL_SUPPORTED_FEATURES;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Random_Address()                               	*/
/* Location: Page 2480 Core_v5.2								*/
/* Purpose: The HCI_LE_Set_Random_Address command is used by 	*/
/* the Host to set the LE Random Device Address in the 			*/
/* Controller (see [Vol 6] Part B, Section 1.3). If this 		*/
/* command is used to change the address, the new random address*/
/* shall take effect for advertising no later than the next 	*/
/* successful HCI_LE_Set_Advertising_Enable command, for 		*/
/* scanning no later than the next successful 					*/
/* HCI_LE_Set_Scan_Enable command or 							*/
/* HCI_LE_Set_Extended_Scan_Enable command, and for initiating	*/
/* no later than the next successful HCI_LE_Create_Connection 	*/
/* command or HCI_LE_Extended_Create_Connection command. Note: 	*/
/* If the extended advertising commands are in use, this command*/
/* only affects the address used for scanning and initiating. 	*/
/* The addresses used for advertising are set by the 			*/
/* HCI_LE_Set_Advertising_Set_Random_Address command (see 		*/
/* Section 7.8.52). If the Host issues this command when any of */
/* advertising (created using legacy advertising commands), 	*/
/* scanning, or initiating are enabled, the Controller shall	*/
/* return the error code Command Disallowed (0x0C).				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Random_Address( BD_ADDR_TYPE Random_Address, DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + sizeof(Random_Address);
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_RANDOM_ADDRESS;
	PcktPtr->CmdPacket.Parameter_Total_Length = sizeof(Random_Address);

	for( int8_t i = 0; i < sizeof(Random_Address); i++ )
	{
		PcktPtr->CmdPacket.Parameter[i] = Random_Address.Bytes[i];
	}

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Advertising_Parameters()                          */
/* Location: Page 2482 Core_v5.2								*/
/* Purpose:	The HCI_LE_Set_Advertising_Parameters command is 	*/
/* used by the Host to set the advertising parameters. The 		*/
/* Advertising_Interval_Min shall be less than or equal to the	*/
/* Advertising_Interval_Max. The Advertising_Interval_Min and	*/
/* Advertising_Interval_Max should not be the same value to 	*/
/* enable the Controller to determine the best advertising 		*/
/* interval given other activities. For high duty cycle directed*/
/* advertising, i.e. when Advertising_Type is 0x01 				*/
/* (ADV_DIRECT_IND, high duty cycle), the 						*/
/* Advertising_Interval_Min and Advertising_Interval_Max 		*/
/* parameters are not used and shall be ignored. The 			*/
/* Advertising_Type is used to determine the packet type that is*/
/* used for advertising when advertising is enabled. 			*/
/* Own_Address_Type parameter indicates the type of address 	*/
/* being used in the advertising packets. If Own_Address_Type 	*/
/* equals 0x02 or 0x03, the Peer_Address parameter contains the */
/* peer’s Identity Address and the Peer_Address_Type parameter	*/
/* contains the Peer’s Identity Type (i.e. 0x00 or 0x01). These */
/* parameters are used to locate the corresponding local IRK in */
/* the resolving list; this IRK is used to generate the own 	*/
/* address used in the advertisement. If directed advertising is*/
/* performed, i.e. when Advertising_Type is set to 0x01			*/
/* (ADV_DIRECT_IND, high duty cycle) or 0x04 (ADV_DIRECT_IND, 	*/
/* low duty cycle mode), then the Peer_Address_Type and 		*/
/* Peer_Address shall be valid. If Own_Address_Type equals 0x02 */
/* or 0x03, the Controller generates the peer’s Resolvable 		*/
/* Private Address using the peer’s IRK corresponding to the	*/
/* peer’s Identity Address contained in the Peer_Address		*/
/* parameter and peer’s Identity Address Type (i.e. 0x00 or 	*/
/* 0x01) contained in the Peer_Address_Type	parameter.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Advertising_Parameters( uint16_t Advertising_Interval_Min, uint16_t Advertising_Interval_Max, ADVERTISING_TYPE Advertising_Type,
		OWN_ADDR_TYPE Own_Address_Type, PEER_ADDR_TYPE Peer_Address_Type, BD_ADDR_TYPE Peer_Address, ADV_CHANNEL_MAP Advertising_Channel_Map,
		uint8_t Advertising_Filter_Policy, DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 15;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_ADVERTISING_PARAMETERS;
	PcktPtr->CmdPacket.Parameter_Total_Length = 15;

	PcktPtr->CmdPacket.Parameter[0] = Advertising_Interval_Min & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( Advertising_Interval_Min >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[2] = Advertising_Interval_Max & 0xFF;
	PcktPtr->CmdPacket.Parameter[3] = ( Advertising_Interval_Max >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[4] = Advertising_Type;

	PcktPtr->CmdPacket.Parameter[5] = Own_Address_Type;

	PcktPtr->CmdPacket.Parameter[6] = Peer_Address_Type;

	for( int8_t i = 0; i < sizeof(Peer_Address); i++ )
	{
		PcktPtr->CmdPacket.Parameter[i + 7] = Peer_Address.Bytes[i];
	}

	PcktPtr->CmdPacket.Parameter[13] = Advertising_Channel_Map.Val;

	PcktPtr->CmdPacket.Parameter[14] = Advertising_Filter_Policy;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Advertising_Physical_Channel_Tx_Power()          */
/* Location: Page 2486 Core_v5.2								*/
/* Purpose: The 												*/
/* HCI_LE_Read_Advertising_Physical_Channel_Tx_Power command is */
/* used by the Host to read the transmit power level used for 	*/
/* LE advertising physical channel packets.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Advertising_Physical_Channel_Tx_Power( LEReadAdvPhyChannelTxPowerComplete CompleteCallBack,
		DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_READ_ADV_PHY_CHANNEL_TX_POWER;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
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
uint8_t HCI_LE_Set_Advertising_Data( uint8_t Advertising_Data_Length, uint8_t Advertising_Data[],
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status = FALSE;

	if( Advertising_Data_Length <= MAX_ADVERTISING_DATA_LENGTH )
	{

		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + MAX_ADVERTISING_DATA_LENGTH + 1;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_ADVERTISING_DATA;
		PcktPtr->CmdPacket.Parameter_Total_Length = MAX_ADVERTISING_DATA_LENGTH + 1;

		memset( &( PcktPtr->CmdPacket.Parameter[0] ), 0, PcktPtr->CmdPacket.Parameter_Total_Length );

		PcktPtr->CmdPacket.Parameter[0] = Advertising_Data_Length;

		memcpy( &( PcktPtr->CmdPacket.Parameter[1] ), &Advertising_Data[0], Advertising_Data_Length );

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Scan_Response_Data()                              */
/* Location: Page 2489 Core_v5.2								*/
/* Purpose: This command is used to provide data used in 		*/
/* Scanning Packets that have a data field. Only the significant*/
/* part of the Scan_Response_Data should be transmitted in the	*/
/* Scanning Packets, as defined in [Vol 3] Part C, Section 11.	*/
/* If advertising is currently enabled, the Controller shall use*/
/* the new data in subsequent advertising events. If an 		*/
/* advertising event is in progress when this command is issued,*/
/* the Controller may use the old or new data for that event. If*/
/* advertising is currently disabled, the data shall be kept by */
/* the Controller and used once advertising is enabled.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Scan_Response_Data( uint8_t Scan_Response_Data_Length, uint8_t Scan_Response_Data[],
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status = FALSE;

	if( Scan_Response_Data_Length <= MAX_SCAN_RESPONSE_DATA_LENGTH )
	{

		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + MAX_SCAN_RESPONSE_DATA_LENGTH + 1;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_SCAN_RESPONSE_DATA;
		PcktPtr->CmdPacket.Parameter_Total_Length = MAX_SCAN_RESPONSE_DATA_LENGTH + 1;

		memset( &( PcktPtr->CmdPacket.Parameter[0] ), 0, PcktPtr->CmdPacket.Parameter_Total_Length );

		PcktPtr->CmdPacket.Parameter[0] = Scan_Response_Data_Length;

		memcpy( &( PcktPtr->CmdPacket.Parameter[1] ), &Scan_Response_Data[0], Scan_Response_Data_Length );

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Advertising_Enable()                              */
/* Location: Page 2491 Core_v5.2								*/
/* Purpose: The HCI_LE_Set_Advertising_Enable command is used to*/
/* request the Controller to start or stop advertising. The 	*/
/* Controller manages the timing of advertisements as per the 	*/
/* advertising parameters given in the 							*/
/* HCI_LE_Set_Advertising_Parameters command. The Controller 	*/
/* shall continue advertising until the Host issues an			*/
/* HCI_LE_Set_Advertising_Enable command with Advertising_Enable*/
/* set to 0x00 (Advertising is disabled) or until a connection 	*/
/* is created or until the Advertising is timed out due to high */
/* duty cycle Directed Advertising. In these cases, advertising */
/* is then disabled. If Advertising_Enable is set to 0x01, the  */
/* advertising parameters' Own_Address_Type parameter is set to */
/* 0x01, and the random address for the device has not been 	*/
/* initialized, the Controller shall return the error code 		*/
/* Invalid HCI Command Parameters (0x12). If Advertising_Enable */
/* is set to 0x01, the advertising parameters' Own_Address_Type */
/* parameter is set to 0x03, the controller's resolving list did*/
/* not contain a matching entry, and the random address for the */
/* device has not been initialized, the Controller shall return */
/* the error code Invalid HCI Command Parameters (0x12). 		*/
/* Enabling advertising when it is already enabled can cause the*/
/* random address to change. Disabling advertising when it is 	*/
/* already disabled has no effect.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Advertising_Enable( uint8_t Advertising_Enable, DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + sizeof(Advertising_Enable);
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_ADVERTISING_ENABLE;
	PcktPtr->CmdPacket.Parameter_Total_Length = sizeof(Advertising_Enable);

	PcktPtr->CmdPacket.Parameter[0] = Advertising_Enable;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Scan_Parameters()                    		        */
/* Location: Page 2493 Core_v5.2								*/
/* Purpose: The HCI_LE_Set_Scan_Parameters command is used to 	*/
/* set the scan parameters. The LE_Scan_Type parameter controls */
/* the type of scan to perform. The LE_Scan_Interval and 		*/
/* LE_Scan_Window parameters are recommendations from the Host 	*/
/* on how long (LE_Scan_Window) and how frequently 				*/
/* (LE_Scan_Interval) the Controller should scan (See [Vol 6] 	*/
/* Part B, Section 4.4.3). The LE_Scan_Window parameter shall 	*/
/* always be set to a value smaller or equal to the value set	*/
/* for the LE_Scan_Interval parameter. If they are set to the 	*/
/* same value scanning should be run continuously. 				*/
/* Own_Address_Type parameter indicates the type of address 	*/
/* being used in the scan request packets. The Host shall not 	*/
/* issue this command when scanning is enabled in the 			*/
/* Controller if it is the Command Disallowed error code shall 	*/
/* be used.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Scan_Parameters( LE_SCAN_TYPE LE_Scan_Type, uint16_t LE_Scan_Interval, uint16_t LE_Scan_Window,
		OWN_ADDR_TYPE Own_Address_Type, uint8_t Scanning_Filter_Policy, DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 7;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_SCAN_PARAMETERS;
	PcktPtr->CmdPacket.Parameter_Total_Length = 7;

	PcktPtr->CmdPacket.Parameter[0] = LE_Scan_Type;

	PcktPtr->CmdPacket.Parameter[1] = LE_Scan_Interval & 0xFF;
	PcktPtr->CmdPacket.Parameter[2] = ( LE_Scan_Interval >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[3] = LE_Scan_Window & 0xFF;
	PcktPtr->CmdPacket.Parameter[4] = ( LE_Scan_Window >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[5] = Own_Address_Type;

	PcktPtr->CmdPacket.Parameter[6] = Scanning_Filter_Policy;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Scan_Enable()                	    		        */
/* Location: Page 2496 Core_v5.2								*/
/* Purpose: The HCI_LE_Set_Scan_Enable command is used to start */
/* and stop scanning. Scanning is used to discover advertising  */
/* devices nearby. The Filter_Duplicates parameter controls 	*/
/* whether the Link Layer should filter out duplicate 			*/
/* advertising reports (Filtering_Enabled) to the Host, or if 	*/
/* the Link Layer should generate advertising reports for each 	*/
/* packet received (Filtering_Disabled). See [Vol 6] Part B, 	*/
/* Section 4.4.3.5. If LE_Scan_Enable is set to 0x00 then 		*/
/* Filter_Duplicates shall be ignored. If LE_Scan_Enable is set */
/* to 0x01, the scanning parameters' Own_Address_Type parameter */
/* is set to 0x01 or 0x03, and the random address for the device*/
/* has not been initialized, the Controller shall return the 	*/
/* error code Invalid HCI Command Parameters (0x12). If the 	*/
/* LE_Scan_Enable parameter is set to 0x01 and scanning is 		*/
/* already enabled, any change to the Filter_Duplicates setting */
/* shall take effect. Disabling scanning when it is disabled has*/
/* no effect.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Scan_Enable( uint8_t LE_Scan_Enable, uint8_t Filter_Duplicates,
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 2;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_SCAN_ENABLE;
	PcktPtr->CmdPacket.Parameter_Total_Length = 2;

	PcktPtr->CmdPacket.Parameter[0] = LE_Scan_Enable;
	PcktPtr->CmdPacket.Parameter[1] = Filter_Duplicates;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Create_Connection()                	    		    */
/* Location: Page 2498 Core_v5.2								*/
/* Purpose: The HCI_LE_Create_Connection command is used to 	*/
/* create an ACL connection to a connectable advertiser. The 	*/
/* LE_Scan_Interval and LE_Scan_Window parameters are recommen-	*/
/* dations from the Host on how long (LE_Scan_Window) and how	*/
/* frequently (LE_Scan_Interval) the Controller should scan. The*/
/* LE_Scan_Window parameter shall be set to a value smaller or  */
/* equal to the value set for the LE_Scan_Interval parameter. 	*/
/* If both are set to the same value, scanning should run 		*/
/* continuously. The Initiator_Filter_Policy is used to 		*/
/* determine whether the White List is used. If the White List  */
/* is not used, the Peer_Address_Type and the Peer_Address 		*/
/* parameters specify the address type and address of the 		*/
/* advertising device to connect to. Peer_Address_Type parameter*/
/* indicates the type of address used in the connectable 		*/
/* advertisement sent by the peer. The Host shall not set		*/
/* Peer_Address_Type to either 0x02 or 0x03 if both the Host 	*/
/* and the Controller support the HCI_LE_Set_Privacy_Mode 		*/
/* command. If a Controller that supports the 					*/
/* HCI_LE_Set_Privacy_Mode command receives the					*/
/* HCI_LE_Create_Connection command with Peer_Address_Type set 	*/
/* to either 0x02 or 0x03, it may use either device privacy 	*/
/* mode or network privacy mode for that peer device. 			*/
/* Peer_Address parameter indicates the Peer’s Public 			*/
/* Device Address, Random (static) Device Address, 				*/
/* Non-Resolvable Private Address or Resolvable	Private Address */
/* depending on the Peer_Address_Type parameter. 				*/
/* Own_Address_Type parameter indicates the type of address 	*/
/* being used in the connection request packets. The 			*/
/* Connection_Interval_Min and Connection_Interval_Max 			*/
/* parameters define the minimum and maximum allowed connection */
/* interval. The Connection_Interval_Min parameter shall not be */
/* greater than the Connection_Interval_Max parameter. The 		*/
/* Connection_Latency parameter defines the maximum allowed 	*/
/* connection latency (see [Vol 6] Part B, Section 4.5.1).		*/
/* The Supervision_Timeout parameter defines the link 			*/
/* supervision timeout for the connection. The 					*/
/* Supervision_Timeout in milliseconds shall be larger than		*/
/* (1 + Connection_Latency) * Connection_Interval_Max * 2, where*/
/* Connection_Interval_Max is given in milliseconds. (See		*/
/* [Vol 6] Part B, Section 4.5.2). The Min_CE_Length and 		*/
/* Max_CE_Length parameters are informative	parameters providing*/
/* the Controller with the expected minimum and maximum length 	*/
/* of the connection events. The Min_CE_Length parameter shall 	*/
/* be less than or equal to the Max_CE_Length parameter. If the */
/* Host issues this command when another 						*/
/* HCI_LE_Create_Connection command is pending in the 			*/
/* Controller, the Controller shall return the error code		*/
/* Command Disallowed (0x0C). If the Own_Address_Type parameter */
/* is set to 0x01 and the random address for the device has not */
/* been initialized, the Controller shall return the error code */
/* Invalid HCI Command Parameters (0x12). If the 				*/
/* Own_Address_Type parameter is set to 0x03, the 				*/
/* Initiator_Filter_Policy parameter is set to 0x00, the 		*/
/* controller's resolving list did not contain a matching entry,*/
/* and the random address for the device has not been 			*/
/* initialized, the Controller shall return the error code		*/
/* Invalid HCI Command Parameters (0x12). If the				*/
/* Own_Address_Type parameter is set to 0x03, the 				*/
/* Initiator_Filter_Policy parameter is set to 0x01, and the 	*/
/* random address for the device has not been initialized, the 	*/
/* Controller shall return the error code Invalid HCI Command	*/
/* Parameters (0x12).											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Create_Connection( uint16_t LE_Scan_Interval, uint16_t LE_Scan_Window, uint8_t Initiator_Filter_Policy,
		ADDRESS_TYPE Peer_Address_Type, BD_ADDR_TYPE Peer_Address, OWN_ADDR_TYPE Own_Address_Type,
		uint16_t Connection_Interval_Min, uint16_t Connection_Interval_Max, uint16_t Connection_Latency,
		uint16_t Supervision_Timeout, uint16_t Min_CE_Length, uint16_t Max_CE_Length, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 25;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_CREATE_CONNECTION;
	PcktPtr->CmdPacket.Parameter_Total_Length = 25;

	PcktPtr->CmdPacket.Parameter[0] = LE_Scan_Interval & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( LE_Scan_Interval >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[2] = LE_Scan_Window & 0xFF;
	PcktPtr->CmdPacket.Parameter[3] = ( LE_Scan_Window >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[4] = Initiator_Filter_Policy;

	PcktPtr->CmdPacket.Parameter[5] = Peer_Address_Type;

	for( int8_t i = 0; i < sizeof(Peer_Address); i++ )
	{
		PcktPtr->CmdPacket.Parameter[i + 6] = Peer_Address.Bytes[i];
	}

	PcktPtr->CmdPacket.Parameter[12] = Own_Address_Type;

	PcktPtr->CmdPacket.Parameter[13] = Connection_Interval_Min & 0xFF;
	PcktPtr->CmdPacket.Parameter[14] = ( Connection_Interval_Min >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[15] = Connection_Interval_Max & 0xFF;
	PcktPtr->CmdPacket.Parameter[16] = ( Connection_Interval_Max >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[17] = Connection_Latency & 0xFF;
	PcktPtr->CmdPacket.Parameter[18] = ( Connection_Latency >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[19] = Supervision_Timeout & 0xFF;
	PcktPtr->CmdPacket.Parameter[20] = ( Supervision_Timeout >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[21] = Min_CE_Length & 0xFF;
	PcktPtr->CmdPacket.Parameter[22] = ( Min_CE_Length >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[23] = Max_CE_Length & 0xFF;
	PcktPtr->CmdPacket.Parameter[24] = ( Max_CE_Length >> 8 ) & 0xFF;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = NULL, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Create_Connection_Cancel()                            */
/* Location: Page 2503 Core_v5.2								*/
/* Purpose: The HCI_LE_Create_Connection_Cancel command is used */
/* to cancel the HCI_LE_Create_Connection or 					*/
/* HCI_LE_Extended_Create_Connection commands. This command 	*/
/* shall only be issued after the HCI_LE_Create_Connection or   */
/* HCI_LE_Extended_Create_Connection commands have been issued, */
/* an HCI_Command_Status event has been received for the 		*/
/* HCI_LE_Create_Connection or HCI_LE_Extended_Create_Connection*/
/* commands, and before the HCI_LE_Connection_Complete or 		*/
/* HCI_LE_Enhanced_Connection_Complete events.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Create_Connection_Cancel( DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_CREATE_CONNECTION_CANCEL;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_White_List_Size()                            	*/
/* Location: Page 2505 Core_v5.2								*/
/* Purpose: The HCI_LE_Read_White_List_Size command is used to 	*/
/* read the total number of White List entries that can be 		*/
/* stored in the Controller. Note: The number of entries that 	*/
/* can be stored is not fixed and the Controller can change it  */
/* at any time (e.g. because the memory used to store the White */
/* List can also be used for other purposes).					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_White_List_Size( LEReadWhiteListSizeComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_READ_WHITE_LIST_SIZE;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Clear_White_List()                            		*/
/* Location: Page 2506 Core_v5.2								*/
/* Purpose: The HCI_LE_Clear_White_List command is used to clear*/
/* the White List stored in the Controller. This command shall  */
/* not be used when:											*/
/* • any advertising filter policy uses the White List and 		*/
/* advertising is enabled,										*/
/* • the scanning filter policy uses the White List and scanning*/
/* is enabled, or												*/
/* • the initiator filter policy uses the White List and an	    */
/* HCI_LE_Create_Connection or HCI_LE_Extended_Create_Connection*/
/* command is outstanding.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Clear_White_List( DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_CLEAR_WHITE_LIST;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Add_Device_To_White_List()                            */
/* Location: Page 2507 Core_v5.2								*/
/* Purpose: The HCI_LE_Add_Device_To_White_List command is used */
/* to add a single device to the White List stored in the 		*/
/* Controller. This command shall not be used when:				*/
/* - any advertising filter policy uses the White List and 		*/
/* advertising is enabled,										*/
/* - the scanning filter policy uses the White List and scanning*/
/* is enabled, or												*/
/* - the initiator filter policy uses the White List and an		*/
/* HCI_LE_Create_Connection or HCI_LE_Extended_Create_Connection*/
/* command is outstanding. When a Controller cannot add a device*/
/* to the White List because there is no space available, it 	*/
/* shall return the error code Memory Capacity Exceeded (0x07). */
/* If the device is already in the White List, the Controller 	*/
/* should not add the device to the White List again and should */
/* return success. Address shall be ignored when Address_Type is*/
/* set to 0xFF.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Add_Device_To_White_List( uint8_t Address_Type, BD_ADDR_TYPE Address,
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 7;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_ADD_DEVICE_TO_WHITE_LIST;
	PcktPtr->CmdPacket.Parameter_Total_Length = 7;

	PcktPtr->CmdPacket.Parameter[0] = Address_Type;

	for( int8_t i = 0; i < sizeof(Address); i++ )
	{
		PcktPtr->CmdPacket.Parameter[i + 1] = Address.Bytes[i];
	}

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Remove_Device_From_White_List()                       */
/* Location: Page 2509 Core_v5.2								*/
/* Purpose: The HCI_LE_Remove_Device_From_White_List command is */
/* used to remove a single device from the White List stored in */
/* the Controller. This command shall not be used when:			*/
/* - any advertising filter policy uses the White List and 		*/
/* advertising is enabled,										*/
/* - the scanning filter policy uses the White List and scanning*/
/* is enabled, or												*/
/* - the initiator filter policy uses the White List and an		*/
/* HCI_LE_Create_Connection or HCI_LE_Extended_Create_Connection*/
/* command is outstanding.										*/
/* Address shall be ignored when Address_Type is set to 0xFF.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Remove_Device_From_White_List( uint8_t Address_Type, BD_ADDR_TYPE Address,
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 7;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_REMOVE_DEVICE_FROM_WHITE_LIST;
	PcktPtr->CmdPacket.Parameter_Total_Length = 7;

	PcktPtr->CmdPacket.Parameter[0] = Address_Type;

	for( int8_t i = 0; i < sizeof(Address); i++ )
	{
		PcktPtr->CmdPacket.Parameter[i + 1] = Address.Bytes[i];
	}

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Host_Channel_Classification()                     */
/* Location: Page 2514 Core_v5.2								*/
/* Purpose: The HCI_LE_Set_Host_Channel_Classification command  */
/* allows the Host to specify a channel classification for the 	*/
/* data, secondary advertising, periodic, and isochronous 		*/
/* physical channels based on its “local information”. This		*/
/* classification persists until overwritten with a subsequent  */
/* HCI_LE_Set_Host_Channel_Classification command or until the  */
/* Controller is reset using the HCI_Reset command (see [Vol 6] */
/* Part B, Section 4.5.8.1). If this command is used, the Host  */
/* should send it within 10 seconds of knowing that the channel */
/* classification has changed. The interval between two			*/
/* successive commands sent shall be at least one second. This  */
/* command shall only be used when the local device supports the*/
/* Master role, supports extended advertising in the Advertising*/
/* state, or supports the Isochronous Broadcaster role.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Host_Channel_Classification( CHANNEL_MAP Channel_Map,
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + sizeof(Channel_Map);
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_HOST_CHANNEL_CLASSIFICATION;
	PcktPtr->CmdPacket.Parameter_Total_Length = sizeof(Channel_Map);

	for( int8_t i = 0; i < sizeof(Channel_Map); i++ )
	{
		PcktPtr->CmdPacket.Parameter[i] = Channel_Map.Bytes[i];
	}

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Connection_Update()                     				*/
/* Location: Page 2511 Core_v5.2								*/
/* Purpose: The HCI_LE_Connection_Update command is used to 	*/
/* change the ACL connection parameters. This command may be 	*/
/* issued on both the master and slave. The 					*/
/* Connection_Interval_Min and Connection_Interval_Max 			*/
/* parameters are used to define the minimum and maximum 		*/
/* allowed connection interval. The Connection_Interval_Min 	*/
/* parameter shall not be greater than the 						*/
/* Connection_Interval_Max parameter. The Connection_Latency 	*/
/* parameter shall define the maximum allowed connection 		*/
/* latency. The Supervision_Timeout parameter shall define the 	*/
/* link supervision timeout for the LE link. The 				*/
/* Supervision_Timeout in milliseconds shall be larger than (1	*/
/* + Connection_Latency) * Connection_Interval_Max * 2, where	*/
/* Connection_Interval_Max is given in milliseconds. The 		*/
/* Min_CE_Length and Max_CE_Length are information parameters	*/
/* providing the Controller with a hint about the expected 		*/
/* minimum and maximum length of the connection events. The 	*/
/* Min_CE_Length shall be less than or equal to the 			*/
/* Max_CE_Length. The actual parameter values selected by the 	*/
/* Link Layer may be different from the parameter values 		*/
/* provided by the Host through this command.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Connection_Update( uint16_t Connection_Handle, uint16_t Connection_Interval_Min, uint16_t Connection_Interval_Max,
		uint16_t Connection_Latency, uint16_t Supervision_Timeout, uint16_t Min_CE_Length, uint16_t Max_CE_Length, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 14;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_CONNECTION_UPDATE;
	PcktPtr->CmdPacket.Parameter_Total_Length = 14;

	PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[2] = Connection_Interval_Min & 0xFF;
	PcktPtr->CmdPacket.Parameter[3] = ( Connection_Interval_Min >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[4] = Connection_Interval_Max & 0xFF;
	PcktPtr->CmdPacket.Parameter[5] = ( Connection_Interval_Max >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[6] = Connection_Latency & 0xFF;
	PcktPtr->CmdPacket.Parameter[7] = ( Connection_Latency >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[8] = Supervision_Timeout & 0xFF;
	PcktPtr->CmdPacket.Parameter[9] = ( Supervision_Timeout >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[10] = Min_CE_Length & 0xFF;
	PcktPtr->CmdPacket.Parameter[11] = ( Min_CE_Length >> 8 ) & 0xFF;

	PcktPtr->CmdPacket.Parameter[12] = Max_CE_Length & 0xFF;
	PcktPtr->CmdPacket.Parameter[13] = ( Max_CE_Length >> 8 ) & 0xFF;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = NULL, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Channel_Map()                     				*/
/* Location: Page 2516 Core_v5.2								*/
/* Purpose: The HCI_LE_Read_Channel_Map command returns the 	*/
/* current Channel_Map for the specified Connection_Handle. 	*/
/* The returned value indicates the state of the Channel_Map 	*/
/* specified by the last transmitted or received Channel_Map	*/
/* (in a CONNECT_IND or LL_CHANNEL_MAP_IND message) for the 	*/
/* specified Connection_Handle, regardless of whether the 		*/
/* Master has received an acknowledgment.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Channel_Map( uint16_t Connection_Handle,
		LEReadChannelMapComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 2;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_READ_CHANNEL_MAP;
	PcktPtr->CmdPacket.Parameter_Total_Length = 2;

	PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Remote_Features()                     			*/
/* Location: Page 2518 Core_v5.2								*/
/* Purpose: This command requests, from the remote device 		*/
/* identified by the Connection_Handle, the features used on the*/
/* connection and the features supported by the remote device.  */
/* For details see [Vol 6] Part B, Section 4.6. This command may*/
/* be issued on both the master and slave. Note: If a connection*/
/* already exists between the two devices and the features have */
/* already been fetched on that connection, the Controller may  */
/* use a cached copy of the features.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Remote_Features( uint16_t Connection_Handle,
		LEReadRemoteFeaturesComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 2;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_READ_REMOTE_FEATURES;
	PcktPtr->CmdPacket.Parameter_Total_Length = 2;

	PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Encrypt()                     						*/
/* Location: Page 2519 Core_v5.2								*/
/* Purpose: The HCI_LE_Encrypt command is used to request the 	*/
/* Controller to encrypt the Plaintext_Data in the command using*/
/* the Key given in the command and returns the Encrypted_Data 	*/
/* to the Host. The AES-128 bit block cypher is defined in NIST */
/* Publication FIPS-197 										*/
/* (http://csrc.nist.gov/publications/fips/fips197/fips-197.pdf)*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Encrypt( uint8_t Key[16], uint8_t Plaintext_Data[16],
		LEEncryptComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 32;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_ENCRYPT;
	PcktPtr->CmdPacket.Parameter_Total_Length = 32;

	memcpy( &(PcktPtr->CmdPacket.Parameter[0]), &Key[0], 16 );
	memcpy( &(PcktPtr->CmdPacket.Parameter[16]), &Plaintext_Data[0], 16 );

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Rand()                     							*/
/* Location: Page 2521 Core_v5.2								*/
/* Purpose: The HCI_LE_Rand command is used to request the 		*/
/* Controller to generate 8 octets of random data to be sent to */
/* the Host. The Random_Number shall be generated according to 	*/
/* [Vol 2] Part H, Section 2 if the LE Feature (LE Encryption) 	*/
/* is supported.												*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Rand( LERandComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_RAND;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Enable_Encryption()                     				*/
/* Location: Page 2522 Core_v5.2								*/
/* Purpose: The HCI_LE_Enable_Encryption command is used to 	*/
/* authenticate the given encryption key associated with the 	*/
/* remote device specified by the Connection_Handle, and once 	*/
/* authenticated will encrypt the connection. The parameters 	*/
/* are as defined in [Vol 3] Part H, Section 2.4.4. If the 		*/
/* connection is already encrypted then the Controller shall 	*/
/* pause connection encryption before attempting to 			*/
/* authenticate the given encryption key, and then re-encrypt 	*/
/* the connection. While encryption is paused no user data 		*/
/* shall be transmitted. If the Connection_Handle parameter 	*/
/* identifies an ACL with an associated CIS that has been 		*/
/* created, the Controller shall return the error code Command	*/
/* Disallowed (0x0C). On an authentication failure, the 		*/
/* connection shall be automatically disconnected by the Link 	*/
/* Layer. If this command succeeds, then the connection shall	*/
/* be encrypted. This command shall only be used when the local */
/* device’s role is Master.										*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Enable_Encryption( uint16_t Connection_Handle, uint8_t Random_Number[8],
		uint16_t Encrypted_Diversifier, uint8_t Long_Term_Key[16], DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 28;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_ENABLE_ENCRYPTION;
	PcktPtr->CmdPacket.Parameter_Total_Length = 28;

	PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;

	memcpy( &(PcktPtr->CmdPacket.Parameter[2]), &Random_Number[0], 8 );

	PcktPtr->CmdPacket.Parameter[10] = Encrypted_Diversifier & 0xFF;
	PcktPtr->CmdPacket.Parameter[11] = ( Encrypted_Diversifier >> 8 ) & 0xFF;

	memcpy( &(PcktPtr->CmdPacket.Parameter[12]), &Long_Term_Key[0], 16 );

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = NULL, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Long_Term_Key_Request_Reply()                     	*/
/* Location: Page 2524 Core_v5.2								*/
/* Purpose: The HCI_LE_Long_Term_Key_Request_Reply command is 	*/
/* used to reply to an HCI_LE_Long_Term_Key_Request event from 	*/
/* the Controller, and specifies the Long_Term_Key parameter 	*/
/* that shall be used for this Connection_Handle. The 			*/
/* Long_Term_Key is used as defined in [Vol 6] Part B, 			*/
/* Section 5.1.3. This command shall only be used when the 		*/
/* local device’s role is Slave.								*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Long_Term_Key_Request_Reply( uint16_t Connection_Handle, uint8_t Long_Term_Key[16],
		LELongTermKeyRqtReplyComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 18;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_LONG_TERM_KEY_REQUEST_REPLY;
	PcktPtr->CmdPacket.Parameter_Total_Length = 18;

	PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;

	memcpy( &(PcktPtr->CmdPacket.Parameter[2]), &Long_Term_Key[0], 16 );

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Long_Term_Key_Request_Negative_Reply()               	*/
/* Location: Page 2526 Core_v5.2								*/
/* Purpose: The HCI_LE_Long_Term_Key_Request_Negative_Reply 	*/
/* command is used to reply to an HCI_LE_Long_Term_Key_Request 	*/
/* event from the Controller if the Host cannot provide a Long 	*/
/* Term Key for this Connection_Handle. This command shall only */
/* be used when the local device’s role is Slave.				*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Long_Term_Key_Request_Negative_Reply( uint16_t Connection_Handle,
		LELongTermKeyRqtNegReplyComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 2;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_LONG_TERM_KEY_RQT_NEG_REPLY;
	PcktPtr->CmdPacket.Parameter_Total_Length = 2;

	PcktPtr->CmdPacket.Parameter[0] = Connection_Handle & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( Connection_Handle >> 8 ) & 0xFF;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Supported_States()               				*/
/* Location: Page 2527 Core_v5.2								*/
/* Purpose: The HCI_LE_Read_Supported_States command reads the 	*/
/* states and state combinations that the Link Layer supports. 	*/
/* See [Vol 6] Part B, Section 1.1.1. LE_States is an 8-octet 	*/
/* bit field. If a bit is set to 1 then this state or state		*/
/* combination is supported by the Controller. Multiple bits in */
/* LE_States may be set to 1 to indicate support for multiple 	*/
/* state and state combinations. Note: This command only 		*/
/* provides information about the supported states that can be 	*/
/* used with legacy advertising. It does not provide 			*/
/* information about states, and combinations of states, that 	*/
/* can only be used with the extended advertising commands		*/
/* (see Section 3.1.1).											*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Supported_States( LEReadSupportedStatesComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_READ_SUPPORTED_STATES;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Receiver_Test_v1()               						*/
/* Location: Page 2530 Core_v5.2								*/
/* Purpose: This command is used to start a test where the DUT	*/
/* receives test reference packets at a fixed interval. The 	*/
/* tester generates the test reference packets. The RX_Channel 	*/
/* and PHY parameters specify the RF channel and PHY to be used */
/* by the receiver. If the Host sets the PHY parameter to a PHY */
/* that the Controller does not support, including a value that */
/* is reserved for future use, the Controller shall return the 	*/
/* error code Unsupported Feature or Parameter Value (0x11).	*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Receiver_Test_v1( uint8_t RX_Channel, DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status = FALSE;

	if( RX_Channel <= 0x27 )
	{
		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 1;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_LE_RECEIVER_TEST_V1;
		PcktPtr->CmdPacket.Parameter_Total_Length = 1;

		PcktPtr->CmdPacket.Parameter[0] = RX_Channel;

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_LE_Transmitter_Test_v1()               					*/
/* Location: Page 2534 Core_v5.2								*/
/* Purpose: This command is used to start a test where the DUT	*/
/* generates test reference packets at a fixed interval. The 	*/
/* Controller shall transmit at the power level indicated by the*/
/* Transmit_Power_Level parameter. The TX_Channel and PHY 		*/
/* parameters specify the RF channel and PHY to be used by the 	*/
/* transmitter. If the Host sets the PHY parameter to a PHY that*/
/* the Controller does not support, including a value that is 	*/
/* reserved for future use, the Controller shall return the 	*/
/* error code Unsupported Feature or Parameter Value (0x11). 	*/
/* The Test_Data_Length and Packet_Payload parameters specify 	*/
/* the length and contents of the Payload of the test reference */
/* packets. An LE Controller supporting the 					*/
/* HCI_LE_Transmitter_Test command shall support Packet_Payload */
/* values 0x00, 0x01 and 0x02. An LE Controller supporting the	*/
/* LE Coded PHY shall also support Packet_Payload value 0x04.	*/
/* An LE Controller may support other values of Packet_Payload.	*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Transmitter_Test_v1( uint8_t TX_Channel, uint8_t Test_Data_Length, uint8_t Packet_Payload,
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status = FALSE;

	if( TX_Channel <= 0x27 )
	{
		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 3;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = HCI_LE_TRANSMITTER_TEST_V1;
		PcktPtr->CmdPacket.Parameter_Total_Length = 3;

		PcktPtr->CmdPacket.Parameter[0] = TX_Channel;
		PcktPtr->CmdPacket.Parameter[1] = Test_Data_Length;
		PcktPtr->CmdPacket.Parameter[2] = Packet_Payload;

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_LE_Test_End()               								*/
/* Location: Page 2539 Core_v5.2								*/
/* Purpose: This command is used to stop any test which is in 	*/
/* progress. The Num_Packets for a transmitter test shall be 	*/
/* reported as 0x0000. The Num_Packets is an unsigned number 	*/
/* and contains the number of received packets.					*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Test_End( DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_TEST_END;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Add_Device_To_Resolving_List()               			*/
/* Location: Page 2554 Core_v5.2								*/
/* Purpose: This command is used to add one device to the 		*/
/* resolving list used to generate and resolve Resolvable 		*/
/* Private Addresses in the Controller. This command shall not 	*/
/* be used when address resolution is enabled in the Controller */
/* and:															*/
/* • Advertising (other than periodic advertising) is enabled,	*/
/* • Scanning is enabled, or									*/
/* • an HCI_LE_Create_Connection, 								*/
/* HCI_LE_Extended_Create_Connection, or						*/
/* HCI_LE_Periodic_Advertising_Create_Sync command is 			*/
/* outstanding. This command may be used at any time when 		*/
/* address resolution is disabled in the Controller. The added 	*/
/* device shall be set to Network Privacy mode. When a 			*/
/* Controller cannot add a device to the list because there is 	*/
/* no space available, it shall return the error code Memory 	*/
/* Capacity Exceeded (0x07).									*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Add_Device_To_Resolving_List( PEER_ADDR_TYPE Peer_Identity_Address_Type, BD_ADDR_TYPE Peer_Identity_Address,
		IRK_TYPE* Peer_IRK, IRK_TYPE* Local_IRK, DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 39;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_ADD_DEVICE_TO_RESOLVING_LIST;
	PcktPtr->CmdPacket.Parameter_Total_Length = 39;

	PcktPtr->CmdPacket.Parameter[0] = Peer_Identity_Address_Type;
	memcpy( &(PcktPtr->CmdPacket.Parameter[1]), &Peer_Identity_Address.Bytes[0], sizeof(BD_ADDR_TYPE) );
	memcpy( &(PcktPtr->CmdPacket.Parameter[sizeof(BD_ADDR_TYPE) + 1]), &(Peer_IRK->Bytes[0]), sizeof(IRK_TYPE) );
	memcpy( &(PcktPtr->CmdPacket.Parameter[sizeof(IRK_TYPE) + sizeof(BD_ADDR_TYPE) + 1]), &(Local_IRK->Bytes[0]), sizeof(IRK_TYPE) );

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, &Hosted_LE_Add_Device_To_Resolving_List, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Remove_Device_From_Resolving_List()          			*/
/* Location: Page 2556 Core_v5.2								*/
/* Purpose: This command is used to remove one device from the  */
/* resolving list used to resolve Resolvable Private Addresses 	*/
/* in the Controller. This command shall not be used when 		*/
/* address resolution is enabled in the Controller and:			*/
/* • Advertising (other than periodic advertising) is enabled,	*/
/* • Scanning is enabled, or									*/
/* • an HCI_LE_Create_Connection, 								*/
/* HCI_LE_Extended_Create_Connection, or						*/
/* HCI_LE_Periodic_Advertising_Create_Sync command is 			*/
/* outstanding. This command may be used at any time when 		*/
/* address resolution is disabled in the Controller. When a 	*/
/* Controller cannot remove a device from the resolving list 	*/
/* because it is not found, it shall return the error code 		*/
/* Unknown Connection Identifier (0x02). 						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Remove_Device_From_Resolving_List( PEER_ADDR_TYPE Peer_Identity_Address_Type, BD_ADDR_TYPE Peer_Identity_Address,
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 7;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_REMOVE_DEVICE_FROM_RESOLVING_LIST;
	PcktPtr->CmdPacket.Parameter_Total_Length = 7;

	PcktPtr->CmdPacket.Parameter[0] = Peer_Identity_Address_Type;
	memcpy( &(PcktPtr->CmdPacket.Parameter[1]), &Peer_Identity_Address.Bytes[0], sizeof(BD_ADDR_TYPE) );

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, &Hosted_LE_Remove_Device_From_Resolving_List, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Clear_Resolving_List()        						*/
/* Location: Page 2558 Core_v5.2								*/
/* Purpose: This command is used to remove all devices from the */
/* resolving list used to resolve Resolvable Private Addresses 	*/
/* in the Controller. This command shall not be used when 		*/
/* address resolution is enabled in the Controller and:			*/
/* • Advertising (other than periodic advertising) is enabled,	*/
/* • Scanning is enabled, or									*/
/* • an HCI_LE_Create_Connection, 								*/
/* HCI_LE_Extended_Create_Connection, or						*/
/* HCI_LE_Periodic_Advertising_Create_Sync command is 			*/
/* outstanding. This command may be used at any time when 		*/
/* address resolution is disabled in the Controller.			*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Clear_Resolving_List( DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_CLEAR_RESOLVING_LIST;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Resolving_List_Size()        					*/
/* Location: Page 2559 Core_v5.2								*/
/* Purpose: This command is used to read the total number of 	*/
/* entries in the resolving list that can be stored in the 		*/
/* Controller. Note: The number of entries that can be stored 	*/
/* is not fixed and the Controller can change it at any time 	*/
/* (e.g. because the memory used to store the list can also be 	*/
/* used for other purposes).									*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Resolving_List_Size( LEReadResolvingListSizeComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = HCI_LE_READ_RESOLVING_LIST_SIZE;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Peer_Resolvable_Address()        				*/
/* Location: Page 2560 Core_v5.2								*/
/* Purpose: This command is used to get the current peer 		*/
/* Resolvable Private Address being used for the corresponding	*/
/* peer Public and Random (static) Identity Address. The peer’s */
/* resolvable address being used may change after the command 	*/
/* is called. This command may be used at any time. When a 		*/
/* Controller cannot find a Resolvable Private Address 			*/
/* associated with the Peer Identity Address, or if the Peer 	*/
/* Identity Address cannot be found in the resolving list, it 	*/
/* shall return the error code Unknown Connection Identifier	*/
/* (0x02).														*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Peer_Resolvable_Address( PEER_ADDR_TYPE Peer_Identity_Address_Type, BD_ADDR_TYPE Peer_Identity_Address,
		LEReadPeerResolvableAddressComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 7;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_READ_PEER_RESOLVABLE_ADDRESS;
	PcktPtr->CmdPacket.Parameter_Total_Length = 7;

	PcktPtr->CmdPacket.Parameter[0] = Peer_Identity_Address_Type;
	memcpy( &(PcktPtr->CmdPacket.Parameter[1]), &Peer_Identity_Address.Bytes[0], sizeof(BD_ADDR_TYPE) );

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, &Hosted_LE_Read_Peer_Resolvable_Address, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Read_Local_Resolvable_Address()        				*/
/* Location: Page 2562 Core_v5.2								*/
/* Purpose: This command is used to get the current local 		*/
/* Resolvable Private Address being used for the corresponding	*/
/* peer Identity Address. The local resolvable address being 	*/
/* used may change after the command is called. This command 	*/
/* may be used at any time. When a Controller cannot find a 	*/
/* Resolvable Private Address associated with the Peer Identity */
/* Address, or if the Peer Identity Address cannot be found in  */
/* the resolving list, it shall return the error code Unknown 	*/
/* Connection Identifier (0x02).								*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Read_Local_Resolvable_Address( PEER_ADDR_TYPE Peer_Identity_Address_Type, BD_ADDR_TYPE Peer_Identity_Address,
		LEReadLocalResolvableAddressComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 7;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_READ_LOCAL_RESOLVABLE_ADDRESS;
	PcktPtr->CmdPacket.Parameter_Total_Length = 7;

	PcktPtr->CmdPacket.Parameter[0] = Peer_Identity_Address_Type;
	memcpy( &(PcktPtr->CmdPacket.Parameter[1]), &Peer_Identity_Address.Bytes[0], sizeof(BD_ADDR_TYPE) );

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, &Hosted_LE_Read_Local_Resolvable_Address, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Address_Resolution_Enable()        				*/
/* Location: Page 2564 Core_v5.2								*/
/* Purpose: This command is used to enable resolution of 		*/
/* Resolvable Private Addresses in the Controller. This causes 	*/
/* the Controller to use the resolving list whenever the 		*/
/* Controller receives a local or peer Resolvable Private 		*/
/* Address. This command shall not be used when:				*/
/* • Advertising (other than periodic advertising) is enabled,	*/
/* • Scanning is enabled, or									*/
/* • an HCI_LE_Create_Connection, 								*/
/* HCI_LE_Extended_Create_Connection, or						*/
/* HCI_LE_Periodic_Advertising_Create_Sync command is 			*/
/* outstanding.  Enabling address resolution when it is already */
/* enabled, or disabling it when it is already disabled, has no */
/* effect. Note: This command does not affect the generation of */
/* Resolvable Private Addresses.								*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Address_Resolution_Enable( uint8_t Address_Resolution_Enable, DefCmdComplete CompleteCallBack,
		DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 1;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_ADDRESS_RESOLUTION_ENABLE;
	PcktPtr->CmdPacket.Parameter_Total_Length = 1;

	PcktPtr->CmdPacket.Parameter[0] = Address_Resolution_Enable;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, &Hosted_LE_Set_Address_Resolution_Enable, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_LE_Set_Resolvable_Private_Address_Timeout()   			*/
/* Location: Page 2566 Core_v5.2								*/
/* Purpose: This command set the length of time the Controller 	*/
/* uses a Resolvable Private Address before a new resolvable 	*/
/* private address is generated and starts being used. This 	*/
/* timeout applies to all resolvable private addresses 			*/
/* generated by the Controller.									*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_LE_Set_Resolvable_Private_Address_Timeout( uint16_t RPA_Timeout, DefCmdComplete CompleteCallBack,
		DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 2;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = HCI_LE_SET_RESOLVABLE_PRIVATE_ADDRESS_TIMEOUT;
	PcktPtr->CmdPacket.Parameter_Total_Length = 2;

	PcktPtr->CmdPacket.Parameter[0] = RPA_Timeout & 0xFF;
	PcktPtr->CmdPacket.Parameter[1] = ( RPA_Timeout >> 8 ) & 0xFF;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, &Hosted_LE_Set_Resolvable_Private_Address_Timeout, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* HCI_Disconnection_Complete()        							*/
/* Location: Page 2296 Core_v5.2								*/
/* Purpose: The HCI_Disconnection_Complete event occurs when a 	*/
/* connection is terminated. The status parameter indicates if	*/
/* the disconnection was successful or not. The reason parameter*/
/* indicates the reason for the disconnection if the 			*/
/* disconnection was successful. If the disconnection was not 	*/
/* successful, the value of the reason parameter shall be 		*/
/* ignored by the Host. For example, this can be the case if 	*/
/* the Host has issued the HCI_Disconnect command and there was */
/* a parameter error, or the command was not presently allowed, */
/* or a Connection_Handle that didn’t correspond to a 			*/
/* connection was given. Note: When a physical link fails, one 	*/
/* HCI_Disconnection_Complete event will be returned for each 	*/
/* logical channel on the physical link with the corresponding	*/
/* Connection_Handle as a parameter.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Disconnection_Complete( DisconnectionComplete* DisConnCpltData )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Encryption_Change()        								*/
/* Location: Page 2299 Core_v5.2								*/
/* Purpose: The HCI_Encryption_Change event is used to indicate */
/* that the change of the encryption mode has been completed. 	*/
/* The Connection_Handle will be a Connection_Handle for an ACL */
/* connection and is used to identify the remote device. The 	*/
/* Encryption_Enabled event parameter specifies the new			*/
/* Encryption_Enabled parameter for the Connection_Handle 		*/
/* specified by the Connection_Handle event parameter. This		*/
/* event will occur on both devices to notify the Hosts when 	*/
/* Encryption has changed for all connections between the two 	*/
/* devices. This event shall not be generated if encryption is 	*/
/* paused or resumed; during a role switch, for example. The 	*/
/* meaning of the Encryption_Enabled parameter depends on 		*/
/* whether the Host has indicated support for Secure 			*/
/* Connections in the Secure_Connections_Host_Support parameter.*/
/* When Secure_Connections_Host_Support is ‘disabled’ or the 	*/
/* Connection_Handle refers to an LE link, the Controller shall */
/* only use Encryption_Enabled values 0x00 (OFF) and 0x01 (ON). */
/* When Secure_Connections_Host_Support is ‘enabled’ and the 	*/
/* Connection_Handle refers to a BR/EDR link, the Controller 	*/
/* shall set Encryption_Enabled to 0x00 when encryption is off, */
/* to 0x01 when encryption is on and using E0 and to 0x02 when 	*/
/* encryption is on and using AES-CCM.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Encryption_Change( CONTROLLER_ERROR_CODES Status, uint16_t Connection_Handle, uint8_t Encryption_Enabled )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Command_Complete()                						*/
/* Location: 2308 Core_v5.2		 								*/
/* Purpose: Called when no handler is provided for the command	*/
/* opcode.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Command_Complete( uint8_t Num_HCI_Command_Packets, HCI_COMMAND_OPCODE Command_Opcode, uint8_t* Return_Parameters )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Command_Status()                							*/
/* Location: 2310 Core_v5.2		 								*/
/* Purpose: Called when no handler is provided for the command	*/
/* opcode.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Command_Status( CONTROLLER_ERROR_CODES Status, uint8_t Num_HCI_Command_Packets, HCI_COMMAND_OPCODE Command_Opcode )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Hardware_Error()                							*/
/* Location: 2312 Core_v5.2		 								*/
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
/* HCI_Number_Of_Completed_Packets()                			*/
/* Location: 2315 Core_v5.2		 								*/
/* Purpose: The HCI_Number_Of_Completed_Packets event is used 	*/
/* by the Controller to indicate to the Host how many HCI Data 	*/
/* packets have been completed (transmitted or flushed) for each*/
/* Connection_Handle since the previous 						*/
/* HCI_Number_Of_Completed_Packets event was sent to the Host. 	*/
/* This means that the corresponding buffer space has been 		*/
/* freed in the Controller. Based on this information, and the 	*/
/* Total_Num_ACL_Data_Packets and 								*/
/* Total_Num_Synchronous_Data_Packets return parameter of the	*/
/* HCI_Read_Buffer_Size command, the Host can determine for 	*/
/* which Connection_Handles the following HCI Data packets 		*/
/* should be sent to the Controller. The 						*/
/* HCI_Number_Of_Completed_Packets event shall not specify a 	*/
/* given Connection_Handle before the HCI_Connection_Complete 	*/
/* event for the corresponding connection or after an event 	*/
/* indicating disconnection of the corresponding connection. 	*/
/* While the Controller has HCI Data packets in its buffer, it 	*/
/* shall keep sending the HCI_Number_Of_Completed_Packets event */
/* to the Host at least periodically, until it finally reports 	*/
/* that all the pending ACL Data packets have been transmitted 	*/
/* or flushed. The rate with which this event is sent is 		*/
/* manufacturer specific. Note: HCI_Number_Of_Completed_Packets */
/* events will not report on synchronous Connection_Handles if  */
/* synchronous Flow Control is disabled.	(See Section 7.3.36 */
/* and Section 7.3.37.)											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Number_Of_Completed_Packets( uint8_t Num_Handles, uint16_t Connection_Handle[], uint16_t Num_Completed_Packets[] )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Data_Buffer_Overflow()                					*/
/* Location: 2325 Core_v5.2		 								*/
/* Purpose: This event is used to indicate that the 			*/
/* Controller’s data buffers have been overflowed. This can 	*/
/* occur if the Host has sent more packets than allowed. The 	*/
/* Link_Type parameter is used to indicate the type of data 	*/
/* whose buffers overflowed.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Data_Buffer_Overflow( uint8_t Link_Type )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Encryption_Key_Refresh_Complete()                		*/
/* Location: 2349 Core_v5.2		 								*/
/* Purpose: The HCI_Encryption_Key_Refresh_Complete event is 	*/
/* used to indicate to the Host that the encryption key was 	*/
/* refreshed on the given Connection_Handle any time encryption */
/* is paused and then resumed. The Controller shall send this	*/
/* event when the encryption key has been refreshed due to 		*/
/* encryption being started or resumed. If the 					*/
/* HCI_Encryption_Key_Refresh_Complete event was generated due	*/
/* to an encryption pause and resume operation embedded within 	*/
/* a change connection link key procedure, the 					*/
/* HCI_Encryption_Key_Refresh_Complete event shall be sent 		*/
/* prior to the HCI_Change_Connection_Link_Key_Complete event.	*/
/* If the HCI_Encryption_Key_Refresh_Complete event was 		*/
/* generated due to an encryption pause and resume operation 	*/
/* embedded within a role switch procedure, the 				*/
/* HCI_Encryption_Key_Refresh_Complete event shall be sent 		*/
/* prior to the HCI_Role_Change event.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_Encryption_Key_Refresh_Complete( CONTROLLER_ERROR_CODES Status, uint16_t Connection_Handle )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Connection_Complete()                					*/
/* Location: 2379 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Connection_Complete event indicates to 	*/
/* both of the Hosts forming the connection that a new 			*/
/* connection has been created. Upon the creation of the		*/
/* connection a Connection_Handle shall be assigned by the		*/
/* Controller, and passed to the Host in this event. If the 	*/
/* connection creation fails this event shall be provided to 	*/
/* the Host that had issued the HCI_LE_Create_Connection 		*/
/* command. This event indicates to the Host which issued an 	*/
/* HCI_LE_Create_Connection command and received an 			*/
/* HCI_Command_Status event if the connection creation failed 	*/
/* or was successful. The Master_Clock_Accuracy parameter is 	*/
/* only valid for a slave. On a master, this parameter shall be */
/* set to 0x00. Note: This event is not sent if the 			*/
/* HCI_LE_Enhanced_Connection_Complete event 					*/
/* (see Section 7.7.65.10) is unmasked.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Connection_Complete( LEConnectionComplete* ConnCpltData )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Enhanced_Connection_Complete()               			*/
/* Location: 2394 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Enhanced_Connection_Complete event 		*/
/* indicates to both of the Hosts forming the connection that a */
/* new connection has been created. Upon the creation of the 	*/
/* connection a Connection_Handle shall be assigned by the		*/
/* Controller, and passed to the Host in this event. If the 	*/
/* connection creation fails, this event shall be provided to 	*/
/* the Host that had issued the HCI_LE_Create_Connection or 	*/
/* HCI_LE_Extended_Create_Connection command. If this event is 	*/
/* unmasked and the HCI_LE_Connection_Complete event is 		*/
/* unmasked, only the HCI_LE_Enhanced_Connection_Complete event */
/* is sent when a new connection has been created. This event 	*/
/* indicates to the Host that issued an 						*/
/* HCI_LE_Create_Connection or 									*/
/* HCI_LE_Extended_Create_Connection command and received an	*/
/* HCI_Command_Status event if the connection creation failed 	*/
/* or was successful. The Peer_Address, 						*/
/* Peer_Resolvable_Private_Address, and							*/
/* Local_Resolvable_Private_Address shall always reflect the 	*/
/* most recent packet sent and received on air. The 			*/
/* Master_Clock_Accuracy parameter is only valid for a slave. 	*/
/* On a master, this parameter shall be set to 0x00.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Enhanced_Connection_Complete( LEEnhancedConnectionComplete* ConnCpltData )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Advertising_Report()                					*/
/* Location: 2382 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Advertising_Report event indicates that 	*/
/* one or more Bluetooth devices have responded to an active 	*/
/* scan or have broadcast advertisements that were received 	*/
/* during a passive scan. The Controller may queue these 		*/
/* advertising reports and send information from multiple 		*/
/* devices in one HCI_LE_Advertising_Report event. This event 	*/
/* shall only be generated if scanning was enabled using the	*/
/* HCI_LE_Set_Scan_Enable command. It only reports advertising  */
/* events that used legacy advertising PDUs.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Advertising_Report( LEAdvertisingReport* AdvReport )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Connection_Update_Complete()                			*/
/* Location: 2384 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Connection_Update_Complete event is used */
/* to indicate that the Controller process to update the 		*/
/* connection has completed. This event shall be issued if the 	*/
/* HCI_LE_Connection_Update command was issued by the Host or 	*/
/* if the connection parameters are updated following a request */
/* from the peer device. If no parameters are updated following */
/* a request from the peer device then this event shall not be 	*/
/* issued. Note: This event can be issued autonomously by the 	*/
/* Master’s Controller if it decides to change the connection 	*/
/* interval based on the range of allowable connection 			*/
/* intervals for that connection. Note: The parameter values 	*/
/* returned in this event may be different from the parameter 	*/
/* values provided by the Host through the 						*/
/* HCI_LE_Connection_Update command (Section 7.8.18) or the		*/
/* HCI_LE_Remote_Connection_Parameter_Request_Reply command 	*/
/* (Section 7.8.31).											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Connection_Update_Complete( CONTROLLER_ERROR_CODES Status, uint16_t Connection_Handle, uint16_t Connection_Interval,
		uint16_t Connection_Latency, uint16_t Supervision_Timeout )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_LE_Long_Term_Key_Request()                				*/
/* Location: 2387 Core_v5.2		 								*/
/* Purpose: The HCI_LE_Long_Term_Key_Request event indicates 	*/
/* that the peer device, in the Master role, is attempting to 	*/
/* encrypt or re-encrypt the link and is requesting the Long 	*/
/* Term Key from the Host. (See [Vol 6] Part B, Section 5.1.3). */
/* This event shall only be generated when the local device’s 	*/
/* role is Slave.												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void HCI_LE_Long_Term_Key_Request( uint16_t Connection_Handle, uint8_t Random_Number[8], uint16_t Encrypted_Diversifier )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* HCI_Host_ACL_Data()                							*/
/* Location: 1892 Core_v5.2		 								*/
/* Purpose: HCI ACL Data packets are used to exchange data 		*/
/* between the Host and Controller. There are two types of HCI 	*/
/* ACL Data packets:											*/
/* • Automatically-Flushable									*/
/* • Non-Automatically-Flushable								*/
/* Automatically-Flushable HCI Data packets are flushed based 	*/
/* on the setting of an automatic flush timer (see Read 		*/
/* Automatic Flush Timeout command). Non-Automatically-Flushable*/
/* HCI Data packets are not controlled by the automatic flush 	*/
/* timeout and shall not be automatically flushed. The format 	*/
/* of the HCI ACL Data packet is shown in Figure 5.2. The 		*/
/* definition for each of the fields in the data packets is 	*/
/* explained below. Hosts and Controllers shall be able to 		*/
/* accept HCI ACL Data packets with up to 27 bytes of data 		*/
/* excluding the HCI ACL Data packet header on 					*/
/* Connection_Handles associated with an LE-U logical link.The 	*/
/* HCI ACL Data packet header is the first 4 octets of the 		*/
/* packet. Note: HCI ACL Data packets with a Connection_Handle  */
/* associated with an LEU logical link will not be affected by  */
/* the automatic flush timer because only nonflushable packet 	*/
/* boundary flags are allowed. All packets on an AMP logical 	*/
/* link are affected by the automatic flush timer.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t HCI_Host_ACL_Data( HCI_ACL_DATA_PCKT_HEADER ACLDataPacketHeader, uint8_t Data[] )
{
	uint8_t Status = FALSE;

	if( ACLDataPacketHeader.Data_Total_Length <= 27 )
	{
		uint16_t ByteArraySize = sizeof(HCI_SERIAL_ACL_DATA_PCKT) + ACLDataPacketHeader.Data_Total_Length;
		HCI_SERIAL_ACL_DATA_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_ACL_DATA_PACKET;
		PcktPtr->ACLDataPacket.Header = ACLDataPacketHeader;

		memcpy( &(PcktPtr->ACLDataPacket.Data[0]), &Data[0], ACLDataPacketHeader.Data_Total_Length );

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = NULL, .CmdStatusCallBack = NULL };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );
	}

	return (Status);
}


/****************************************************************/
/* HCI_Controller_ACL_Data()                					*/
/* Location: 1892 Core_v5.2		 								*/
/* Purpose: HCI ACL Data packets are used to exchange data 		*/
/* between the Host and Controller. There are two types of HCI 	*/
/* ACL Data packets:											*/
/* • Automatically-Flushable									*/
/* • Non-Automatically-Flushable								*/
/* Automatically-Flushable HCI Data packets are flushed based 	*/
/* on the setting of an automatic flush timer (see Read 		*/
/* Automatic Flush Timeout command). Non-Automatically-Flushable*/
/* HCI Data packets are not controlled by the automatic flush 	*/
/* timeout and shall not be automatically flushed. The format 	*/
/* of the HCI ACL Data packet is shown in Figure 5.2. The 		*/
/* definition for each of the fields in the data packets is 	*/
/* explained below. Hosts and Controllers shall be able to 		*/
/* accept HCI ACL Data packets with up to 27 bytes of data 		*/
/* excluding the HCI ACL Data packet header on 					*/
/* Connection_Handles associated with an LE-U logical link.The 	*/
/* HCI ACL Data packet header is the first 4 octets of the 		*/
/* packet. Note: HCI ACL Data packets with a Connection_Handle  */
/* associated with an LEU logical link will not be affected by  */
/* the automatic flush timer because only nonflushable packet 	*/
/* boundary flags are allowed. All packets on an AMP logical 	*/
/* link are affected by the automatic flush timer.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void 	HCI_Controller_ACL_Data( HCI_ACL_DATA_PCKT_HEADER ACLDataPacketHeader, uint8_t Data[] )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
