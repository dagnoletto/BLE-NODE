

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_utils.h"
#include "TimeFunctions.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	GENERATE_RANDOM_NUMBER_PART_A,
	GENERATE_RANDOM_NUMBER_PART_B,
	GENERATE_RANDOM_MANUALLY,
	ENCRYPT_RANDOM_NUMBER,
	CHECK_RANDOM_NUMBERS,
	WAIT_OPERATION_A,
	WAIT_OPERATION_B,
	CONFIG_STATIC_ADDR,
	CONFIG_PRIVATE_ADDR
}CONFIG_BD_ADDR;


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define DEFAULT_AES_128_KEY { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x10, 0x90, \
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x10, 0x90 }


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Verify_Static_Address( uint8_t Address[6] );
static uint8_t Verify_Private_Address( uint8_t Address[6] );
static uint8_t Check_Bit_Presence( uint8_t Address[6] );


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static BD_ADDR_TYPE PUBLIC_ADDRESS  = { DEFAULT_PUBLIC_ADDRESS };
static BD_ADDR_TYPE PRIVATE_STATIC_ADDRESS;
static BD_ADDR_TYPE PRIVATE_NON_RESOLVABLE_ADDRESS;
static BD_ADDR_TYPE PRIVATE_RESOLVABLE_ADDRESS;
static uint8_t Static_Address_Flag = FALSE;
static uint8_t Random_Bytes[8 * 2];
static CONFIG_BD_ADDR BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
static uint8_t AES_128_KEY[16] = DEFAULT_AES_128_KEY;
static uint8_t* DataPtr = NULL;


/****************************************************************/
/* Generate_Device_Addresses()        							*/
/* Location: 					 								*/
/* Purpose: Generate the static and private addresses used.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Generate_Device_Addresses( SUPPORTED_COMMANDS* HCI_Sup_Cmd, uint8_t Update_Static_ADDR )
{
	static uint32_t TimeoutCounter = 0;

	switch ( BD_Config )
	{
	case GENERATE_RANDOM_NUMBER_PART_A:
		if( DataPtr != NULL )
		{
			free(DataPtr);
			DataPtr = NULL;
		}
		TimeoutCounter = 0;
		if( HCI_Sup_Cmd->Bits.HCI_LE_Rand )
		{
			BD_Config = HCI_LE_Rand( ) ? WAIT_OPERATION_A : GENERATE_RANDOM_NUMBER_PART_A;
		}else
		{
			BD_Config = GENERATE_RANDOM_MANUALLY;
		}
		break;

	case GENERATE_RANDOM_NUMBER_PART_B:
		TimeoutCounter = 0;
		/* Move the bytes previously read to the last 8 bytes. */
		memcpy( &Random_Bytes[8], &Random_Bytes[0], ( sizeof(Random_Bytes) / 2 ) );

		if( HCI_Sup_Cmd->Bits.HCI_LE_Rand )
		{
			BD_Config = HCI_LE_Rand( ) ? WAIT_OPERATION_B : GENERATE_RANDOM_NUMBER_PART_B;
		}else
		{
			BD_Config = GENERATE_RANDOM_MANUALLY;
		}
		break;

	case GENERATE_RANDOM_MANUALLY:
		srand( (unsigned)HAL_GetTick() ); /* Initialize the seed function */

		for( int8_t i = 0; i < sizeof(Random_Bytes); i++ )
		{
			Random_Bytes[i] = ( rand( ) % UINT8_MAX ); /* Generate random number between 0 and UINT8_MAX */
		}

		BD_Config = ENCRYPT_RANDOM_NUMBER;
		break;

	case ENCRYPT_RANDOM_NUMBER:
		TimeoutCounter = 0;
		if( HCI_Sup_Cmd->Bits.HCI_LE_Encrypt )
		{
			/* There is no special purpose to encrypt the random bytes to generate the addresses, but the Message Sequence Chart in
			 * Specification v5.2 page 3101 do that. */
			BD_Config = HCI_LE_Encrypt( &AES_128_KEY[0], &Random_Bytes[0] ) ? WAIT_OPERATION_A : ENCRYPT_RANDOM_NUMBER;
		}else
		{
			if( DataPtr != NULL )
			{
				free(DataPtr);
				DataPtr = NULL;
			}

			DataPtr = malloc( 16 );
			memcpy( DataPtr, &Random_Bytes[0], 16 );
			BD_Config = CHECK_RANDOM_NUMBERS;
		}
		break;

	case CHECK_RANDOM_NUMBERS:
	{
		uint8_t status = TRUE;

		TimeoutCounter = 0;

		/* The static address is not subject to modification all the time, only after device power-up. */
		/* So look to the parameters to see if it should update or not */
		if( Update_Static_ADDR )
		{
			if( Verify_Static_Address( DataPtr ) )
			{
				memcpy( &PRIVATE_STATIC_ADDRESS.Bytes[0], DataPtr, sizeof(BD_ADDR_TYPE) );

				/* Make sure the address sub-type is set to 0b11 */
				PRIVATE_STATIC_ADDRESS.Bytes[5] |= 0xC0;
			}else
			{
				status = FALSE;
			}
		}

		/* Check non-resolvable private address */
		if( Verify_Private_Address( DataPtr + sizeof(BD_ADDR_TYPE) ) )
		{
			memcpy( &PRIVATE_NON_RESOLVABLE_ADDRESS.Bytes[0], DataPtr + sizeof(BD_ADDR_TYPE), sizeof(BD_ADDR_TYPE) );

			/* Make sure the address sub-type is set to 0b00 */
			PRIVATE_NON_RESOLVABLE_ADDRESS.Bytes[5] &= 0x3F;
		}else
		{
			status = FALSE;
		}

		if( DataPtr != NULL )
		{
			free(DataPtr);
			DataPtr = NULL;
		}
	}
	break;

	case CONFIG_STATIC_ADDR:
		break;

	case CONFIG_PRIVATE_ADDR:
		break;

	case WAIT_OPERATION_A:
	case WAIT_OPERATION_B:
		if( TimeBase_DelayMs( &TimeoutCounter, 500, TRUE ) )
		{
			if( DataPtr != NULL )
			{
				free(DataPtr);
				DataPtr = NULL;
			}
			BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
		}
		break;
	}

	return (FALSE);

}


/****************************************************************/
/* Verify_Static_Address()        								*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Verify_Static_Address( uint8_t Address[6] )
{
	return ( Check_Bit_Presence( &Address[0] ) );
}


/****************************************************************/
/* Verify_Private_Address()        								*/
/* Location: 					 								*/
/* Purpose: check non-resolvable private address.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Verify_Private_Address( uint8_t Address[6] )
{
	if( Check_Bit_Presence( &Address[0] ) )
	{
		Address[5] &= 0x3F; /* Clears the non-resolvable private address. The sub-type must be 0b00 */
		/* Public and private non-resolvable MUST NOT be equal. */
		if( memcmp( &Address[0], Get_Public_Device_Address(), sizeof(BD_ADDR_TYPE) ) != 0 )
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Check_Bit_Presence()        									*/
/* Location: 					 								*/
/* Purpose: Verify if the random part of an random address has	*/
/* at least one bit one and one bit zero.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Bit_Presence( uint8_t Address[6] )
{
	uint8_t BitOneFound = FALSE;
	uint8_t BitZeroFound = FALSE;

	uint8_t BitMask = 0xFF;
	uint8_t Byte;

	for( int8_t i = 0; i < 6; i++ )
	{
		Byte = Address[i];

		/* The most two significant bits are excluded */
		if( i == 5 )
		{
			BitMask = 0x3F;
		}

		/* Check the presence of bit one */
		if( Byte & BitMask )
		{
			BitOneFound = TRUE;
		}

		/* Check the presence of bit zero */
		if( (~Byte) & BitMask )
		{
			BitZeroFound = TRUE;
		}

		if( BitOneFound && BitZeroFound )
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Get_Public_Device_Address()        							*/
/* Location: 					 								*/
/* Purpose: Return the public address assigned by the user.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BD_ADDR_TYPE* Get_Public_Device_Address( void )
{
	return ( &PUBLIC_ADDRESS );
}


/****************************************************************/
/* HCI_LE_Rand_Complete()        								*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_LE_Rand_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Random_Number[8] )
{
	if( Status == COMMAND_SUCCESS )
	{
		memcpy( &Random_Bytes[0], &Random_Number[0], ( sizeof(Random_Bytes) / 2 ) );
		switch ( BD_Config )
		{
		case WAIT_OPERATION_A:
			BD_Config = GENERATE_RANDOM_NUMBER_PART_B;
			break;

		case WAIT_OPERATION_B:
			BD_Config = ENCRYPT_RANDOM_NUMBER;
			break;

		default: break;
		}
	}
}


/****************************************************************/
/* HCI_LE_Encrypt_Complete()        							*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void HCI_LE_Encrypt_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Encrypted_Data[16] )
{
	if( DataPtr != NULL )
	{
		free(DataPtr);
		DataPtr = NULL;
	}

	if( Status == COMMAND_SUCCESS )
	{
		DataPtr = malloc( 16 );
		memcpy( DataPtr, &Encrypted_Data[0], 16 );
		BD_Config = CHECK_RANDOM_NUMBERS;
	}else
	{
		BD_Config = ENCRYPT_RANDOM_NUMBER;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
