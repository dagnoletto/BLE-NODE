

#ifndef SECURITY_MANAGER_H_
#define SECURITY_MANAGER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


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


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* SECURITY_MANAGER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
