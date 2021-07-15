

#ifndef SECURITY_MANAGER_H_
#define SECURITY_MANAGER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define MAX_NUMBER_OF_RESOLVING_LIST_ENTRIES	8


/****************************************************************/
/* Type Defines 				               		            */
/****************************************************************/
typedef struct
{
	PEER_ADDR_TYPE Type;
	BD_ADDR_TYPE Address;
}__attribute__((packed)) IDENTITY_ADDRESS;


typedef struct
{
	IDENTITY_ADDRESS Peer_Identity_Address;
	IRK_TYPE Local_IRK;
	IRK_TYPE Peer_IRK;
}__attribute__((packed)) DEVICE_IDENTITY;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint16_t Get_Size_Of_Resolving_List( void );
uint16_t Add_Device_Identity_To_Resolving_List( DEVICE_IDENTITY* Device );
DEVICE_IDENTITY* Get_Device_Identity_From_Resolving_List( uint16_t Index );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* SECURITY_MANAGER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
