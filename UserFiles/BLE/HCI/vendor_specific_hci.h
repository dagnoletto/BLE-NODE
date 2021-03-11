

#ifndef VENDOR_SPECIFIC_HCI_H_
#define VENDOR_SPECIFIC_HCI_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"


/****************************************************************/
/* Type Defines 					                            */
/****************************************************************/
/* Vendor Specific (VS) commands */
typedef enum
{
	VS_ACI_HAL_GET_FW_BUILD_NUMBER		= (PARSE_OPCODE( 0x0000, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_WRITE_CONFIG_DATA		= (PARSE_OPCODE( 0x000C, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_READ_CONFIG_DATA			= (PARSE_OPCODE( 0x000D, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_SET_TX_POWER_LEVEL		= (PARSE_OPCODE( 0x000F, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_DEVICE_STANDBY			= (PARSE_OPCODE( 0x0013, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_LE_TX_TEST_PACKET_NUMBER	= (PARSE_OPCODE( 0x0014, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_TONE_START				= (PARSE_OPCODE( 0x0015, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_TONE_STOP				= (PARSE_OPCODE( 0x0016, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_GET_LINK_STATUS			= (PARSE_OPCODE( 0x0017, VENDOR_SPECIFIC_CMD )),
	VS_ACI_HAL_GET_ANCHOR_PERIOD		= (PARSE_OPCODE( 0x00F8, VENDOR_SPECIFIC_CMD ))
}VS_COMMAND_OPCODE;


typedef union
{
	struct
	{
		uint16_t EID: 10;
		uint16_t EGID: 6;
	};
	uint16_t ECODE;
}ECODE_Struct;


typedef enum
{
	REASON_RESERVED			  = 0x00, /* Reserved */
	FIRMWARE_STARTED_PROPERLY = 0x01, /* Firmware started properly */
	UPDATER_MODE_ACI_START	  = 0x02, /* Updater mode entered because of Aci_Updater_Start command */
	UPDATER_MODE_BLUE_FLAG	  = 0x03, /* Updater mode entered because of a bad BLUE flag */
	UPDATER_MODE_IRQ_PIN	  = 0x04, /* Updater mode entered with IRQ pin */
	RESET_BY_WATCHDOG		  = 0x05, /* Reset caused by watch dog */
	RESET_BY_LOCKUP			  = 0x06, /* Reset due to lockup */
	BROWNOUT_RESET			  = 0x07, /* Brownout reset */
	RESET_BY_CRASH			  = 0x08, /* Reset caused by a crash (NMI or Hard Fault) */
	RESET_ECC_ERROR			  = 0x09  /* Reset caused by an ECC error */
}REASON_CODE;


typedef enum
{
	EVT_BLUE_INITIALIZED_EVENT_CODE	= 0x0001,
	EVT_BLUE_LOST_EVENTS_CODE		= 0x0002,
	FAULT_DATA_EVENT_CODE			= 0x0003
}VS_EVENT_CODE;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint8_t ACI_Hal_Get_Fw_Build_Number( void );
void 	ACI_Hal_Get_Fw_Build_Number_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode, uint16_t RevisionNumber );

uint8_t ACI_Hal_Write_Config_Data( uint8_t Offset, uint8_t Length, uint8_t Value[] );
void    ACI_Hal_Write_Config_Data_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );

uint8_t ACI_Hal_Read_Config_Data( uint8_t Offset );
void 	ACI_Hal_Read_Config_Data_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode, uint8_t* DataPtr, uint8_t DataSize );

uint8_t ACI_Hal_Set_Tx_Power_Level( uint8_t EN_HIGH_POWER, uint8_t PA_LEVEL );
void	ACI_Hal_Set_Tx_Power_Level_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );

uint8_t ACI_Hal_Device_Standby( void );
void 	ACI_Hal_Device_Standby_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );

uint8_t ACI_Hal_LE_Tx_Test_Packet_Number( void );
void	ACI_Hal_LE_Tx_Test_Packet_Number_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode, uint32_t PacketCounter );

uint8_t ACI_Hal_Tone_Start( uint8_t ChannelID );
void 	ACI_Hal_Tone_Start_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );

uint8_t ACI_Hal_Tone_Stop( void );
void 	ACI_Hal_Tone_Stop_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode );

uint8_t ACI_Hal_Get_Link_Status( void );
void 	ACI_Hal_Get_Link_Status_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode, uint8_t* LinkStatus, uint8_t* Connection_Handle );

uint8_t ACI_Hal_Get_Anchor_Period( void );
void 	ACI_Hal_Get_Anchor_Period_Event( EVENT_CODE Event, CONTROLLER_ERROR_CODES ErrorCode, uint32_t AnchorInterval, uint32_t Maxslot );

void 	ACI_Blue_Initialized_Event( REASON_CODE Code );
void	ACI_Blue_Lost_Event( uint8_t* LostEventsMap );
void	ACI_Fault_Data_Event( uint8_t FaultReason, uint32_t* RegistersPtr, uint8_t FaultDataLength, uint8_t* FaultData );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* VENDOR_SPECIFIC_HCI_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
