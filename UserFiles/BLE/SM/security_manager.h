

#ifndef SECURITY_MANAGER_H_
#define SECURITY_MANAGER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define MAX_NUMBER_OF_RESOLVING_LIST_ENTRIES	4


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


typedef struct
{
	DEVICE_IDENTITY Peer;
	IDENTITY_ADDRESS Local_Identity_Address;
}__attribute__((packed)) RESOLVING_RECORD;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
uint16_t Get_Number_Of_Resolving_Records( void );
uint8_t Add_Record_To_Resolving_List( RESOLVING_RECORD* Record );
uint8_t Remove_Record_From_Resolving_List( IDENTITY_ADDRESS* Peer_Identity_Address );
RESOLVING_RECORD* Get_Record_From_Peer_Identity( IDENTITY_ADDRESS* Peer_Identity_Address );
RESOLVING_RECORD* Get_Record_From_Index( uint16_t Index );
uint8_t Clear_Resolving_List( void );


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* SECURITY_MANAGER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
