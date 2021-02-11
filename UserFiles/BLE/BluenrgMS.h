

#ifndef BLUENRG_MS_DRIVER_H_
#define BLUENRG_MS_DRIVER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "BLE_HAL.h"
#include "TimeFunctions.h"
#include "hci_transport_layer.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef struct
{
	HCI_SERIAL_EVENT_PCKT EventPacket;
}__attribute__((packed)) BLUENRG_EVENT_PCKT;


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
