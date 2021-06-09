

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
	CHECK_RANDOM_NUMBERS,
	REQUEST_HASH_CALC,
	LOAD_RESOLVABLE_ADDRESS,
	VERIFY_RESOLVABLE_ADDRESS,
	END_ADDRESSES_CONFIG,
	WAIT_OPERATION_A,
	WAIT_OPERATION_B
}CONFIG_BD_ADDR;


typedef struct
{
	uint8_t hash[3];
	StatusCallBack CallBack;
}RESOLVE_ADDR_STRUCT;


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#define DEFAULT_PUBLIC_ADDRESS { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 }
#define DEFAULT_IRK { 0x02, 0x56, 0xFF, 0x90, 0x71, 0xAB, 0xC5, 0x4A, \
		0x8E, 0x91, 0x00, 0x3C, 0xC3, 0x99, 0x69, 0x24 }


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Create_Static_Address( uint8_t Random_Bytes[6] );
static uint8_t Create_Private_Address( uint8_t Random_Bytes[6] );
static uint8_t Create_Private_Resolvable_Address( uint8_t prand[3] );
static uint8_t Check_Bit_Presence( uint8_t Random_Part[], uint8_t Size_In_Bytes );
static void Hash_CallBack_Function(uint8_t EncryptedData[16], uint8_t status);
static void Resolve_Private_Address_CallBack(uint8_t EncryptedData[16], uint8_t status);
static void Confirm_Local_Private_Addr(uint8_t status);
static void LE_Encrypt_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Encrypted_Data[16] );
static void LE_Rand_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Random_Number[8] );


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static BD_ADDR_TYPE PUBLIC_ADDRESS = { DEFAULT_PUBLIC_ADDRESS };
static BD_ADDR_TYPE RANDOM_STATIC_ADDRESS;
static BD_ADDR_TYPE PRIVATE_NON_RESOLVABLE_ADDRESS;
static BD_ADDR_TYPE PRIVATE_RESOLVABLE_ADDRESS;
static IRK_TYPE DEFAULT_IDENTITY_RESOLVING_KEY = { DEFAULT_IRK };
static uint8_t Rand_Bytes[16];
static uint8_t plaintextData[16];
static uint8_t hash[3];
static CONFIG_BD_ADDR BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
static EncryptCallBack Encrypt_CallBack = NULL;
static RESOLVE_ADDR_STRUCT ResolveStruct = { .CallBack = NULL };


/****************************************************************/
/* Generate_Device_Addresses()        							*/
/* Location: 					 								*/
/* Purpose: Generate the static and private addresses used.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Generate_Device_Addresses( SUPPORTED_COMMANDS* HCI_Sup_Cmd, IRK_TYPE* IRK, uint8_t Update_Static_ADDR )
{
	static uint32_t TimeoutCounter = 0;

	switch ( BD_Config )
	{
	case GENERATE_RANDOM_NUMBER_PART_A:
		TimeoutCounter = 0;
		if( HCI_Sup_Cmd->Bits.HCI_LE_Rand )
		{
			BD_Config = HCI_LE_Rand( &LE_Rand_Complete, NULL ) ? WAIT_OPERATION_A : GENERATE_RANDOM_NUMBER_PART_A;
		}else
		{
			BD_Config = GENERATE_RANDOM_MANUALLY;
		}
		break;

	case GENERATE_RANDOM_NUMBER_PART_B:
		TimeoutCounter = 0;
		/* Move the bytes previously read to the last 8 bytes. */
		memcpy( &Rand_Bytes[8], &Rand_Bytes[0], ( sizeof(Rand_Bytes) / 2 ) );

		if( HCI_Sup_Cmd->Bits.HCI_LE_Rand )
		{
			BD_Config = HCI_LE_Rand( &LE_Rand_Complete, NULL ) ? WAIT_OPERATION_B : GENERATE_RANDOM_NUMBER_PART_B;
		}else
		{
			BD_Config = GENERATE_RANDOM_MANUALLY;
		}
		break;

	case GENERATE_RANDOM_MANUALLY:
		srand( (unsigned)HAL_GetTick() ); /* Initialize the seed function */

		for( int8_t i = 0; i < sizeof(Rand_Bytes); i++ )
		{
			Rand_Bytes[i] = ( rand( ) % UINT8_MAX ); /* Generate random number between 0 and UINT8_MAX */
		}

		BD_Config = CHECK_RANDOM_NUMBERS;
		break;

	case CHECK_RANDOM_NUMBERS:
	{
		uint8_t status = TRUE;

		/* The static address is not subject to modification all the time, only after device power-up. */
		/* So look to the parameters to see if it should update or not */
		if( Update_Static_ADDR )
		{
			if( Create_Static_Address( &Rand_Bytes[0] ) )
			{
				memcpy( &RANDOM_STATIC_ADDRESS.Bytes[0], &Rand_Bytes[0], sizeof(BD_ADDR_TYPE) );
			}else
			{
				status = FALSE;
			}
		}

		/* Check non-resolvable private address */
		if( Create_Private_Address( &Rand_Bytes[sizeof(BD_ADDR_TYPE)] ) )
		{
			memcpy( &PRIVATE_NON_RESOLVABLE_ADDRESS.Bytes[0], &Rand_Bytes[sizeof(BD_ADDR_TYPE)], sizeof(BD_ADDR_TYPE) );
		}else
		{
			status = FALSE;
		}

		/* The last part of the byte arrya will be the prand part of private address. */
		/* Check if prand part cpmplies with requeriment and the jump to hash generation. */
		if( !Create_Private_Resolvable_Address( &Rand_Bytes[2 * sizeof(BD_ADDR_TYPE)] ) )
		{
			status = FALSE;
		}

		BD_Config = ( status == TRUE ) ? REQUEST_HASH_CALC : GENERATE_RANDOM_NUMBER_PART_A;
	}
	break;

	case REQUEST_HASH_CALC:
		TimeoutCounter = 0;
		memset( plaintextData, 0, sizeof(plaintextData) ); /* Clear the structure */

		/* Concatenate the values of prand and r do form r': */
		/* The most significant octet of the PlainText_Data corresponds to plaintextData[0] using the notation specified in FIPS 197. */
		plaintextData[ sizeof(plaintextData) - 1 ] = Rand_Bytes[ 2 * sizeof(BD_ADDR_TYPE) ];
		plaintextData[ sizeof(plaintextData) - 2 ] = Rand_Bytes[ 2 * sizeof(BD_ADDR_TYPE) + 1 ];
		plaintextData[ sizeof(plaintextData) - 3 ] = Rand_Bytes[ 2 * sizeof(BD_ADDR_TYPE) + 2 ];

		BD_Config = AES_128_Encrypt( HCI_Sup_Cmd, &IRK->Bytes[0], &plaintextData[0], &Hash_CallBack_Function ) ? WAIT_OPERATION_A : REQUEST_HASH_CALC;
		break;

	case LOAD_RESOLVABLE_ADDRESS:
		memcpy( &PRIVATE_RESOLVABLE_ADDRESS.Bytes[0], &hash[0], sizeof(BD_ADDR_TYPE)/2 );
		memcpy( &PRIVATE_RESOLVABLE_ADDRESS.Bytes[sizeof(BD_ADDR_TYPE)/2], &Rand_Bytes[ 2 * sizeof(BD_ADDR_TYPE) ], sizeof(BD_ADDR_TYPE)/2 );
		BD_Config = VERIFY_RESOLVABLE_ADDRESS;
		break;

	case VERIFY_RESOLVABLE_ADDRESS:
		TimeoutCounter = 0;
		BD_Config = Resolve_Private_Address( HCI_Sup_Cmd, &PRIVATE_RESOLVABLE_ADDRESS, IRK, &Confirm_Local_Private_Addr ) ? WAIT_OPERATION_A : VERIFY_RESOLVABLE_ADDRESS;
		break;

	case END_ADDRESSES_CONFIG:
		BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
		return (TRUE);
		break;

	case WAIT_OPERATION_A:
	case WAIT_OPERATION_B:
		if( TimeBase_DelayMs( &TimeoutCounter, 500, TRUE ) )
		{
			BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
		}
		break;
	}

	return (FALSE);

}


/****************************************************************/
/* AES_128_Encrypt()        									*/
/* Location: 					 								*/
/* Purpose: AES-128 encrypt machine.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t AES_128_Encrypt( SUPPORTED_COMMANDS* HCI_Sup_Cmd, uint8_t Key[16], uint8_t Plaintext_Data[16], EncryptCallBack CallBack )
{
	if( Encrypt_CallBack == NULL ) /* The encrypt operation is free */
	{
		if( HCI_Sup_Cmd->Bits.HCI_LE_Encrypt ) /* The module supports encryption. */
		{
			Encrypt_CallBack = HCI_LE_Encrypt( &Key[0], &Plaintext_Data[0], &LE_Encrypt_Complete, NULL ) ? CallBack : NULL;
			return ( ( Encrypt_CallBack == NULL ) ? FALSE : TRUE );
		}else
		{
			/* TODO: implement the AES-128 algorithm in software and call the callback function here! */
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Resolve_Private_Address()        							*/
/* Location: 					 								*/
/* Purpose: Resolve private resolvable addresses.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Resolve_Private_Address( SUPPORTED_COMMANDS* HCI_Sup_Cmd, BD_ADDR_TYPE* PrivateAddress, IRK_TYPE* IRK, StatusCallBack CallBack )
{
	uint8_t* DataPtr;

	if( Encrypt_CallBack == NULL ) /* The encrypt operation is free */
	{
		DataPtr = malloc( 16 );
		if( DataPtr != NULL )
		{
			memset( DataPtr, 0, 16 );  /* Clear data */
			DataPtr[15] = PrivateAddress->Bytes[3];
			DataPtr[14] = PrivateAddress->Bytes[4];
			DataPtr[13] = PrivateAddress->Bytes[5];

			ResolveStruct.CallBack = CallBack;
			ResolveStruct.hash[0] = PrivateAddress->Bytes[0];
			ResolveStruct.hash[1] = PrivateAddress->Bytes[1];
			ResolveStruct.hash[2] = PrivateAddress->Bytes[2];

			if ( AES_128_Encrypt( HCI_Sup_Cmd, &IRK->Bytes[0], DataPtr, &Resolve_Private_Address_CallBack ) )
			{
				free( DataPtr );
				return (TRUE);
			}
			free( DataPtr );
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
/* Get_Static_Device_Address()        							*/
/* Location: 					 								*/
/* Purpose: Return the static random address.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BD_ADDR_TYPE* Get_Static_Device_Address( void )
{
	return ( &RANDOM_STATIC_ADDRESS );
}


/****************************************************************/
/* Get_Private_Device_Address()        							*/
/* Location: 					 								*/
/* Purpose: Return the private random address. Either 			*/
/* resolvable or not.											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BD_ADDR_TYPE* Get_Private_Device_Address( uint8_t resolvable )
{
	return ( resolvable ? &PRIVATE_RESOLVABLE_ADDRESS : &PRIVATE_NON_RESOLVABLE_ADDRESS );
}


/****************************************************************/
/* Get_Default_IRK()        									*/
/* Location: 					 								*/
/* Purpose: Return the default Identity Resolving Key.			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
IRK_TYPE* Get_Default_IRK( void )
{
	return ( &DEFAULT_IDENTITY_RESOLVING_KEY );
}


/****************************************************************/
/* Create_Static_Address()        								*/
/* Location: 					 								*/
/* Purpose: create static random address.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Create_Static_Address( uint8_t Random_Bytes[6] )
{
	if( Check_Bit_Presence( &Random_Bytes[0], sizeof(BD_ADDR_TYPE) ) )
	{
		Random_Bytes[sizeof(BD_ADDR_TYPE) - 1] |= 0xC0; /* Sets the static random address MSbs. Make sure the address sub-type is set to 0b11 */
		return (TRUE);
	}

	return (FALSE);
}


/****************************************************************/
/* Create_Private_Address()        								*/
/* Location: 					 								*/
/* Purpose: create non-resolvable private address.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Create_Private_Address( uint8_t Random_Bytes[6] )
{
	if( Check_Bit_Presence( &Random_Bytes[0], sizeof(BD_ADDR_TYPE) ) )
	{
		Random_Bytes[sizeof(BD_ADDR_TYPE) - 1] &= 0x3F; /* Clears the non-resolvable private address. The sub-type must be 0b00 */
		/* Public and private non-resolvable MUST NOT be equal. */
		if( memcmp( &Random_Bytes[0], Get_Public_Device_Address(), sizeof(BD_ADDR_TYPE) ) != 0 )
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Create_Private_Resolvable_Address()      					*/
/* Location: 					 								*/
/* Purpose: create resolvable private address.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Create_Private_Resolvable_Address( uint8_t prand[3] )
{
	if( Check_Bit_Presence( &prand[0], 3 ) ) /* Check prand part */
	{
		prand[2] &= 0x3F; /* Clears the prand most significant bits */
		prand[2] |= 0x40; /* The sub-type must be 0b10 */

		return (TRUE);
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
static uint8_t Check_Bit_Presence( uint8_t Random_Part[], uint8_t Size_In_Bytes )
{
	uint8_t BitOneFound = FALSE;
	uint8_t BitZeroFound = FALSE;

	uint8_t BitMask = 0xFF;
	uint8_t Byte;

	for( int8_t i = 0; i < Size_In_Bytes; i++ )
	{
		Byte = Random_Part[i];

		/* The most two significant bits are excluded */
		if( i == ( Size_In_Bytes - 1 ) )
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
/* Hash_CallBack_Function()        								*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Hash_CallBack_Function(uint8_t EncryptedData[16], uint8_t status)
{
	/* The most significant octet of the Encrypted_Data corresponds to
	Encrypted_Data[0] using the notation specified in FIPS 197. */
	if( status )
	{
		for( int8_t i = 0; i < sizeof(hash); i++ )
		{
			hash[i] = EncryptedData[ 15 - i ];
		}
		BD_Config = LOAD_RESOLVABLE_ADDRESS;
	}else
	{
		BD_Config = REQUEST_HASH_CALC;
	}
}


/****************************************************************/
/* Resolve_Private_Address_CallBack()      						*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Resolve_Private_Address_CallBack(uint8_t EncryptedData[16], uint8_t status)
{
	uint8_t comparisonStatus = TRUE;

	/* The most significant octet of the Encrypted_Data corresponds to
	Encrypted_Data[0] using the notation specified in FIPS 197. */
	if( status )
	{
		for( int8_t i = 0; i < sizeof(ResolveStruct.hash); i++ )
		{
			if( ResolveStruct.hash[i] != EncryptedData[ 15 - i ] )
			{
				comparisonStatus = FALSE;
				break;
			}
		}
	}else
	{
		comparisonStatus = FALSE;
	}


	if( ResolveStruct.CallBack != NULL )
	{
		ResolveStruct.CallBack(comparisonStatus);
	}
}


/****************************************************************/
/* Confirm_Local_Private_Addr()      							*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Confirm_Local_Private_Addr(uint8_t status)
{
	BD_Config = status ? END_ADDRESSES_CONFIG : VERIFY_RESOLVABLE_ADDRESS;
}


/****************************************************************/
/* LE_Rand_Complete()        									*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Rand_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Random_Number[8] )
{
	if( Status == COMMAND_SUCCESS )
	{
		memcpy( &Rand_Bytes[0], &Random_Number[0], ( sizeof(Rand_Bytes) / 2 ) );
		switch ( BD_Config )
		{
		case WAIT_OPERATION_A:
			BD_Config = GENERATE_RANDOM_NUMBER_PART_B;
			break;

		case WAIT_OPERATION_B:
			BD_Config = CHECK_RANDOM_NUMBERS;
			break;

		default: break;
		}
	}
}


/****************************************************************/
/* LE_Encrypt_Complete()        								*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Encrypt_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Encrypted_Data[16] )
{
	/* The most significant octet of the Encrypted_Data corresponds to
	Encrypted_Data[0] using the notation specified in FIPS 197. */

	if( Encrypt_CallBack != NULL )
	{
		if( Status == COMMAND_SUCCESS )
		{
			Encrypt_CallBack( &Encrypted_Data[0], TRUE );
		}else
		{
			Encrypt_CallBack( NULL, FALSE );
		}

		Encrypt_CallBack = NULL;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
