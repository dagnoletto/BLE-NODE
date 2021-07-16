

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "flash.h"
#include "ble_utils.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define BT_BASIC_INFO_BASE_ADDRESS  		( (uint32_t)( &FLASH_DATA_VECTOR[0x000] ) )
#define LE_RESOLVING_LIST_BASE_ADDRESS		( (uint32_t)( &FLASH_DATA_VECTOR[0x100] ) )


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
__attribute__( (aligned(FLASH_PAGE_SIZE)) ) const uint8_t FLASH_DATA_VECTOR[FLASH_PAGE_SIZE] = { [0 ... FLASH_PAGE_SIZE - 1] = 0xFF };


/****************************************************************/
/* GET_BT_BASIC_INFO_BASE_ADDRESS()		      					*/
/* Location: 					 								*/
/* Purpose: Return the flash address for basic Bluetooth Info.	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint32_t GET_BT_BASIC_INFO_BASE_ADDRESS( void )
{
	return ( BT_BASIC_INFO_BASE_ADDRESS );
}


/****************************************************************/
/* GET_LE_RESOLVING_LIST_BASE_ADDRESS()		   					*/
/* Location: 					 								*/
/* Purpose: Return the flash address for Low Energy (LE) Device */
/* Identity resolving list.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint32_t GET_LE_RESOLVING_LIST_BASE_ADDRESS( void )
{
	return ( LE_RESOLVING_LIST_BASE_ADDRESS );
}


/****************************************************************/
/* LE_Write_Address()		        							*/
/* Location: 					 								*/
/* Purpose: Save address to NVM memory. It should be 			*/
/* implemented on application side.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t LE_Write_Address( ADDRESS_TYPE AddressType, uint8_t Data[7] )
{
	union MemDataStruct
	{
		uint8_t DataBytes[8];
		uint64_t Data;
	};

	union MemDataStruct MemData;
	uint8_t Status;

	MemData.DataBytes[0] = AddressType & 0x3;

	memcpy( &MemData.DataBytes[1], &Data[0], 7 );

	uint32_t MemAddress = ( MemData.DataBytes[0] * sizeof(MemData.DataBytes) ) + GET_BT_BASIC_INFO_BASE_ADDRESS();

	Status = FLASH_Program( MemAddress, &MemData.DataBytes[0], sizeof(MemData.DataBytes) );

	if( ( Status ) &&
			( memcmp( &MemData.DataBytes[0], (uint8_t*)MemAddress, sizeof(MemData.DataBytes) ) == 0 ) )
	{
		return (TRUE);
	}else
	{
		return (FALSE);
	}
}


/****************************************************************/
/* LE_Read_Address()		        							*/
/* Location: 					 								*/
/* Purpose: Read address from NVM memory. It should be 			*/
/* implemented on application side.								*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t* LE_Read_Address( ADDRESS_TYPE AddressType )
{
	uint8_t* MemAddress = ( ( AddressType & 0x3 ) * 8 ) +  (uint8_t*)( GET_BT_BASIC_INFO_BASE_ADDRESS() );

	return ( MemAddress );
}


/****************************************************************/
/* FLASH_Program()		 		       							*/
/* Location: 					 								*/
/* Purpose: Write bytes to NVM flash.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t FLASH_Program( uint32_t Address, uint8_t DataPtr[], uint16_t DataSize )
{
	/* TODO: This is not the best approach for saving information in flash memory
	 * since we erase and reprogram all the pages at every write operation.
	 * This is being let for future improvement. */
	if( ( DataSize ) && ( Address >= (uint32_t)( &FLASH_DATA_VECTOR[0] ) ) &&
			( ( Address + DataSize ) <= (uint32_t)( &FLASH_DATA_VECTOR[ sizeof(FLASH_DATA_VECTOR) ] ) ) )
	{
		EnterCritical();

		uint32_t MirrorPointer = (uint32_t)malloc( sizeof(FLASH_DATA_VECTOR) );
		if ( (uint8_t*)MirrorPointer == NULL )
		{
			ExitCritical();
			return (FALSE);
		}

		memcpy( (uint8_t*)MirrorPointer, &FLASH_DATA_VECTOR[0], sizeof(FLASH_DATA_VECTOR) );
		uint32_t MirrorOffset = Address - (uint32_t)( &FLASH_DATA_VECTOR[0] );
		memcpy( (uint8_t*)( MirrorPointer + MirrorOffset ), &DataPtr[0], DataSize );

		FLASH_EraseInitTypeDef Erase;
		uint32_t PageError;
		Erase.NbPages = sizeof(FLASH_DATA_VECTOR) / FLASH_PAGE_SIZE;
		Erase.PageAddress = (uint32_t)( &FLASH_DATA_VECTOR[0] );
		Erase.TypeErase = FLASH_TYPEERASE_PAGES;

		HAL_FLASH_Unlock();

		HAL_FLASHEx_Erase( &Erase, &PageError );

		uint16_t* Mirror = (uint16_t*)MirrorPointer;
		uint16_t* Flash = (uint16_t*)( &FLASH_DATA_VECTOR[0] );

		for ( uint32_t i = 0; i < ( sizeof(FLASH_DATA_VECTOR) / sizeof( uint16_t ) ); i++ )
		{
			if( Mirror[i] != Flash[i] )
			{
				HAL_FLASH_Program( FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)( &Flash[i] ), Mirror[i] );
			}
		}

		HAL_FLASH_Lock();
		free( (uint8_t*)MirrorPointer );
		ExitCritical();

		return (TRUE);
	}

	return (FALSE);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
