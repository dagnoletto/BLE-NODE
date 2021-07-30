

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
	RESOLVING_RECORD Entry[];
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
/* Get_Number_Of_Resolving_Records()		      				*/
/* Location: 					 								*/
/* Purpose: Get the number of records actually saved			*/
/* on Non-volatile memory.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint16_t Get_Number_Of_Resolving_Records( void )
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
/* Add_Record_To_Resolving_List()		  						*/
/* Location: 					 								*/
/* Purpose: Add item to resolving list.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint16_t Add_Record_To_Resolving_List( RESOLVING_RECORD* Record )
{
	uint32_t ListAddress = GET_LE_RESOLVING_LIST_BASE_ADDRESS();

	RESOLVING_LIST* ListPtr = (RESOLVING_LIST*)( ListAddress );
	uint16_t NumberOfEntries = Get_Number_Of_Resolving_Records();

	if( NumberOfEntries < MAX_NUMBER_OF_RESOLVING_LIST_ENTRIES )
	{
		/* Check if there is no similar device identity already allocated. */
		for ( uint16_t i = 0; i < NumberOfEntries; i++ )
		{
			RESOLVING_RECORD* LocalRecord = Get_Record_From_Index( i );
			if ( memcmp( &( LocalRecord->Peer.Peer_Identity_Address ), &(Record->Peer.Peer_Identity_Address), sizeof(IDENTITY_ADDRESS) ) == 0 )
			{
				return (FALSE);
			}
		}

		/* Program device identity */
		FLASH_Program( ListAddress + offsetof(RESOLVING_LIST,Entry[NumberOfEntries]), (uint8_t*)( Record ), sizeof(RESOLVING_RECORD) );

		/* Update number of entries */
		NumberOfEntries++;
		FLASH_Program( ListAddress + offsetof(RESOLVING_LIST,Flags.NumberOfEntries), (uint8_t*)( &NumberOfEntries ), sizeof(ListPtr->Flags.NumberOfEntries) );

		if( ListPtr->Flags.NumberOfEntries == NumberOfEntries )
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Remove_Record_From_Resolving_List()		 					*/
/* Location: 					 								*/
/* Purpose: Remove item from resolving list.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint16_t Remove_Record_From_Resolving_List( IDENTITY_ADDRESS* Peer_Identity_Address )
{
	uint32_t ListAddress = GET_LE_RESOLVING_LIST_BASE_ADDRESS();

	RESOLVING_LIST* ListPtr = (RESOLVING_LIST*)( ListAddress );
	RESOLVING_RECORD Record;
	uint16_t NumberOfEntries = Get_Number_Of_Resolving_Records();

	/* TODO: this code is not optimized for performance. A better implementation
	 * is encouraged. */
	for ( uint16_t i = 0; i < NumberOfEntries; i++ )
	{
		RESOLVING_RECORD* LocalRecord = Get_Record_From_Index( i );
		if ( memcmp( &( LocalRecord->Peer.Peer_Identity_Address ), Peer_Identity_Address, sizeof(IDENTITY_ADDRESS) ) == 0 )
		{
			for ( uint16_t a = i; a < (NumberOfEntries - 1); a++ )
			{
				LocalRecord = Get_Record_From_Index( a + 1 );
				Record = *LocalRecord;
				FLASH_Program( ListAddress + offsetof(RESOLVING_LIST,Entry[a]), (uint8_t*)( &Record ), sizeof(RESOLVING_RECORD) );
			}

			/* Update number of entries */
			NumberOfEntries--;
			FLASH_Program( ListAddress + offsetof(RESOLVING_LIST,Flags.NumberOfEntries), (uint8_t*)( &NumberOfEntries ), sizeof(ListPtr->Flags.NumberOfEntries) );

			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Clear_Resolving_List()		 								*/
/* Location: 					 								*/
/* Purpose: clear resolving list.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint16_t Clear_Resolving_List( void )
{
	uint32_t ListAddress = GET_LE_RESOLVING_LIST_BASE_ADDRESS();

	RESOLVING_LIST* ListPtr = (RESOLVING_LIST*)( ListAddress );
	uint16_t NumberOfEntries = Get_Number_Of_Resolving_Records();

	/* Update number of entries */
	NumberOfEntries = 0;
	FLASH_Program( ListAddress + offsetof(RESOLVING_LIST,Flags.NumberOfEntries), (uint8_t*)( &NumberOfEntries ), sizeof(ListPtr->Flags.NumberOfEntries) );

	if( ListPtr->Flags.NumberOfEntries == NumberOfEntries )
	{
		return (TRUE);
	}

	return (FALSE);
}


/****************************************************************/
/* Get_Record_From_Peer_Identity()		 						*/
/* Location: 					 								*/
/* Purpose: Return the record from peer identity.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
RESOLVING_RECORD* Get_Record_From_Peer_Identity( IDENTITY_ADDRESS* Peer_Identity_Address )
{
	RESOLVING_LIST* ListPtr = (RESOLVING_LIST*)( GET_LE_RESOLVING_LIST_BASE_ADDRESS() );
	uint16_t NumberOfEntries = Get_Number_Of_Resolving_Records();

	for ( uint16_t i = 0; i < NumberOfEntries; i++ )
	{
		RESOLVING_RECORD* LocalRecord = &( ListPtr->Entry[i] );

		if ( memcmp( &( LocalRecord->Peer.Peer_Identity_Address ), Peer_Identity_Address, sizeof(IDENTITY_ADDRESS) ) == 0 )
		{
			return ( LocalRecord );
		}
	}

	return (NULL);
}


/****************************************************************/
/* Get_Record_From_Index()	  									*/
/* Location: 					 								*/
/* Purpose: Get item from resolving list.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
RESOLVING_RECORD* Get_Record_From_Index( uint16_t Index )
{
	RESOLVING_LIST* ListPtr = (RESOLVING_LIST*)( GET_LE_RESOLVING_LIST_BASE_ADDRESS() );
	uint16_t NumberOfEntries = Get_Number_Of_Resolving_Records();

	if( Index < NumberOfEntries )
	{
		return ( &ListPtr->Entry[Index] );
	}

	return (NULL);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
