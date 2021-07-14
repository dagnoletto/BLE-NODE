

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "gap.h"
#include "ble_states.h"
#include "ble_utils.h"
#include "security_manager.h"
#include "flash.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef struct
{
	uint32_t Initialized;
	uint16_t NumberOfEntries;
}__attribute__((packed)) RESOLVING_LIST_FLAGS;


typedef struct
{
	RESOLVING_LIST_FLAGS Flags;
	DEVICE_IDENTITY Entry[];
}__attribute__((packed)) RESOLVING_LIST;


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define RESOLVING_LIST_FILLED 0x35892541


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/


/****************************************************************/
/* Get_Size_Of_Resolving_List()		      						*/
/* Location: 					 								*/
/* Purpose: Get the number of device identities actually saved	*/
/* on Non-volatile memory.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint16_t Get_Size_Of_Resolving_List( void )
{
	uint16_t ListSize = 0;
	uint32_t ListAddress = GET_LE_RESOLVING_LIST_BASE_ADDRESS();

	RESOLVING_LIST* ListPtr = (RESOLVING_LIST*)( ListAddress );

	if ( ListPtr->Flags.Initialized != RESOLVING_LIST_FILLED )
	{
		/* Initialize the list */
		RESOLVING_LIST_FLAGS ListFlags;
		ListFlags.Initialized = RESOLVING_LIST_FILLED;
		ListFlags.NumberOfEntries = 0;

		FLASH_Program( ListAddress, (uint8_t*)( &ListFlags ), sizeof(ListFlags) );
		if ( ListPtr->Flags.Initialized == RESOLVING_LIST_FILLED )
		{
			ListSize = ListPtr->Flags.NumberOfEntries;
		}
	}else
	{
		ListSize = ListPtr->Flags.NumberOfEntries;
	}

	return ( ListSize );
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
