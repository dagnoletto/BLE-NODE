

#ifndef HCI_TRANSPORT_LAYER_H_
#define HCI_TRANSPORT_LAYER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
/* Those are the packet types defined in the Bluetooth Core Specification */
/* The standard defines the UART Transport Layer in v5.2 Volume 4: Part A */
typedef enum
{
	HCI_COMMAND_PACKET 			= 0x01,
	HCI_ACL_DATA_PACKET 		= 0x02,
	HCI_SYNCHRONOUS_DATA_PACKET = 0x03,
	HCI_EVENT_PACKET			= 0x04,
	HCI_ISO_DATA_PACKET 		= 0x05
}HCI_PACKET_TYPE;


typedef struct
{
	uint8_t PacketType;
	HCI_COMMAND_PCKT CmdPacket;
}__attribute__((packed)) HCI_SERIAL_COMMAND_PCKT;


typedef struct
{
	uint8_t PacketType;
	HCI_ACL_DATA_PCKT ACLDataPacket;
}__attribute__((packed)) HCI_SERIAL_ACL_DATA_PCKT;


typedef struct
{
	uint8_t PacketType;
	HCI_SYNCHRONOUS_DATA_PCKT SyncDataPacket;
}__attribute__((packed)) HCI_SERIAL_SYNCHRONOUS_DATA_PCKT;


typedef struct
{
	uint8_t PacketType;
	HCI_EVENT_PCKT EventPacket;
}__attribute__((packed)) HCI_SERIAL_EVENT_PCKT;


typedef struct
{
	uint8_t PacketType;
	HCI_ISO_DATA_PCKT ISODataPacket;
}__attribute__((packed)) HCI_SERIAL_ISO_DATA_PCKT;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* HCI_TRANSPORT_LAYER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
