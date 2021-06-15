

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


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
__attribute__( (aligned(FLASH_PAGE_SIZE)) ) const uint8_t FLASH_DATA_VECTOR[FLASH_PAGE_SIZE] = { [0 ... FLASH_PAGE_SIZE - 1] = 0xFF };


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
	HAL_StatusTypeDef Status;

	MemData.DataBytes[0] = AddressType & 0x3;

	memcpy( &MemData.DataBytes[1], &Data[0], 7 );

	uint32_t MemAddress = ( MemData.DataBytes[0] * sizeof(MemData.DataBytes) ) + (uint32_t)( &FLASH_DATA_VECTOR[0] );

	HAL_FLASH_Unlock();

	Status = HAL_FLASH_Program( FLASH_TYPEPROGRAM_DOUBLEWORD, MemAddress, MemData.Data );

	HAL_FLASH_Lock();

	if( ( Status == HAL_OK ) &&
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
	uint8_t* MemAddress = ( ( AddressType & 0x3 ) * 8 ) +  (uint8_t*)( &FLASH_DATA_VECTOR[0] );

	return ( MemAddress );
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
