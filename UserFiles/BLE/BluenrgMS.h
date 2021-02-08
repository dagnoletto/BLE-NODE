

#ifndef BLUENRG_MS_DRIVER_H_
#define BLUENRG_MS_DRIVER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "BLE_HAL.h"
#include "TimeFunctions.h"


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


typedef enum
{
	SPI_WRITE,
	SPI_READ,
}BLUENRG_COMMAND;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void Reset_BluenrgMS(void);
void BluenrgMS_IRQ(void);


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* BLUENRG_MS_DRIVER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
