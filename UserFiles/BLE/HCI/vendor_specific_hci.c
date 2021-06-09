

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "vendor_specific_hci.h"
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
static const CMD_CALLBACK NullCmdCallBack = { .Status = FALSE, .CmdCompleteCallBack = NULL, .CmdStatusCallBack = NULL };


/****************************************************************/
/* ACI_Hal_Get_Fw_Build_Number()                   			    */
/* Purpose: This command is intended to retrieve the firmware 	*/
/* revision number.												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Get_Fw_Build_Number( HalGetFwBuildNumberComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = VS_ACI_HAL_GET_FW_BUILD_NUMBER;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* ACI_Hal_Write_Config_Data()                    			    */
/* Purpose: This command writes a value to a low level 			*/
/* configure data structure. It is useful to set up directly 	*/
/* some low level parameters for the system in the runtime.		*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Write_Config_Data( uint8_t Offset, uint8_t Length, uint8_t Value[],
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + Length + 2;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = VS_ACI_HAL_WRITE_CONFIG_DATA;
	PcktPtr->CmdPacket.Parameter_Total_Length = Length + 2;
	PcktPtr->CmdPacket.Parameter[0] = Offset;
	PcktPtr->CmdPacket.Parameter[1] = Length;

	for( uint8_t i = 0; i < Length; i++ )
	{
		PcktPtr->CmdPacket.Parameter[i + 2] = Value[i];
	}

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* ACI_Hal_Read_Config_Data()                    			    */
/* Purpose: This command requests the value in the low level 	*/
/* configure data structure. For more information see the		*/
/* command ACI_Hal_Write_Config_Data. The number of bytes of 	*/
/* returned value changes for different offset.					*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Read_Config_Data( uint8_t Offset,
		HalReadConfigDataComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 1;
	HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

	PcktPtr->PacketType = HCI_COMMAND_PACKET;
	PcktPtr->CmdPacket.OpCode.Val = VS_ACI_HAL_READ_CONFIG_DATA;
	PcktPtr->CmdPacket.Parameter_Total_Length = 1;
	PcktPtr->CmdPacket.Parameter[0] = Offset;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	free( PcktPtr );

	return (Status);
}


/****************************************************************/
/* ACI_Hal_Set_Tx_Power_Level()                    			    */
/* Purpose: This command sets the TX power level of the 		*/
/* BlueNRG-MS. By controlling the EN_HIGH_POWER and the			*/
/* PA_LEVEL, the combination of the 2 determines the output 	*/
/* power level (dBm).											*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Set_Tx_Power_Level( uint8_t EN_HIGH_POWER, uint8_t PA_LEVEL,
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status = FALSE;

	if( ( EN_HIGH_POWER <= 1 ) && ( PA_LEVEL <= 7 ) )
	{

		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 2;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = VS_ACI_HAL_SET_TX_POWER_LEVEL;
		PcktPtr->CmdPacket.Parameter_Total_Length = 2;
		PcktPtr->CmdPacket.Parameter[0] = EN_HIGH_POWER;
		PcktPtr->CmdPacket.Parameter[1] = PA_LEVEL;

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );

	}

	return (Status);
}


/****************************************************************/
/* ACI_Hal_Device_Standby()                    				    */
/* Purpose: Normally the BlueNRG-MS will automatically enter 	*/
/* sleep mode to save power. This Aci_Hal_Device_Standby command*/
/* further put the device into the Standby mode instead of the 	*/
/* sleep mode. The difference is that, in sleep mode, the device*/
/* can still wake up itself with the internal timer. But in 	*/
/* standby mode, this timer is also disabled. So the only 		*/
/* possibility to  wake up the device is by the external signals*/
/* e.g. a HCI command sent via SPI bus.							*/
/* command.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Device_Standby( DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = VS_ACI_HAL_DEVICE_STANDBY;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* ACI_Hal_LE_Tx_Test_Packet_Number()                    		*/
/* Purpose: During the Direct Test mode, in the TX tests, the 	*/
/* number of packets sent in the test is not returned when		*/
/* executing the Direct Test End command. This command 			*/
/* implements this feature.If the Direct TX test is started, a 	*/
/* 32-bit counter will be used to count how many packets have 	*/
/* been transmitted. After the Direct Test End, this command 	*/
/* can be used to check how many packets were sent during the 	*/
/* Direct TX test. The counter starts from 0 and counts upwards.*/
/* As would be the case if 32-bits are all used, the counter 	*/
/* wraps back and starts from 0 again. The counter is not 		*/
/* cleared until the next Direct TX test starts.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_LE_Tx_Test_Packet_Number( HalLETxTestPacketNumberComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* ACI_Hal_Tone_Start()                   				 		*/
/* Purpose: This command starts a carrier frequency, i.e. a 	*/
/* tone, on a specific channel. The frequency sine wave at the	*/
/* specific channel may be used for debugging purpose only. The */
/* channel ID is a parameter from 0x00 to 0x27 for				*/
/* the 40 BLE channels, e.g. 0x00 for 2.402 GHz, 0x01 for 		*/
/* 2.404 GHz etc. This command should not be used when normal 	*/
/* Bluetooth activities are ongoing. The tone should be 		*/
/* stopped by Aci_Hal_Tone_Stop command.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Tone_Start( uint8_t ChannelID,
		DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status = FALSE;

	if( ChannelID <= 0x27 ) /* 40 channels: channel 0 to channel 39 */
	{

		uint16_t ByteArraySize = sizeof(HCI_SERIAL_COMMAND_PCKT) + 1;
		HCI_SERIAL_COMMAND_PCKT* PcktPtr = malloc( ByteArraySize );

		PcktPtr->PacketType = HCI_COMMAND_PACKET;
		PcktPtr->CmdPacket.OpCode.Val = VS_ACI_HAL_TONE_START;
		PcktPtr->CmdPacket.Parameter_Total_Length = 1;
		PcktPtr->CmdPacket.Parameter[0] = ChannelID;

		CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

		Status = HCI_Transmit( PcktPtr, ByteArraySize, CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

		free( PcktPtr );

	}

	return (Status);
}


/****************************************************************/
/* ACI_Hal_Tone_Stop()                   				 		*/
/* Purpose: This command is used to stop the previously 		*/
/* started Aci_Hal_Tone_Start command.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Tone_Stop( DefCmdComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = VS_ACI_HAL_TONE_STOP;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* ACI_Hal_Get_Link_Status()                   			 		*/
/* Purpose: This command is intended to return the Link Layer 	*/
/* Status and Connection Handles.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Get_Link_Status( HalGetLinkStatusComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = VS_ACI_HAL_GET_LINK_STATUS;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* ACI_Hal_Get_Anchor_Period()                   		 		*/
/* Purpose: This command is intended to retrieve information 	*/
/* about the current Anchor Interval and allocable timing 		*/
/* slots.														*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t ACI_Hal_Get_Anchor_Period( HalGetAnchorPeriodComplete CompleteCallBack, DefCmdStatus StatusCallBack )
{
	uint8_t Status;

	HCI_SERIAL_COMMAND_PCKT Pckt;

	Pckt.PacketType = HCI_COMMAND_PACKET;
	Pckt.CmdPacket.OpCode.Val = VS_ACI_HAL_GET_ANCHOR_PERIOD;
	Pckt.CmdPacket.Parameter_Total_Length = 0;

	CMD_CALLBACK CmdCallBack = { .CmdCompleteCallBack = CompleteCallBack, .CmdStatusCallBack = StatusCallBack };

	Status = HCI_Transmit( &Pckt, sizeof(Pckt), CALL_BACK_AFTER_TRANSFER, NULL, &CmdCallBack );

	return (Status);
}


/****************************************************************/
/* ACI_Blue_Initialized_Event()                    		      	*/
/* Purpose: Vendor Specific Event 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void ACI_Blue_Initialized_Event( REASON_CODE Code )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* ACI_Blue_Lost_Event()                   		 		      	*/
/* Purpose: Vendor Specific Event 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void ACI_Blue_Lost_Event( uint8_t* LostEventsMap )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* ACI_Fault_Data_Event()                   	 		      	*/
/* Purpose: Vendor Specific Event 								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
__attribute__((weak)) void ACI_Fault_Data_Event( uint8_t FaultReason, uint32_t* RegistersPtr, uint8_t FaultDataLength, uint8_t* FaultData )
{
	/* The user should implement at higher layers since it is weak. */
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
