

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
/* The DEFAULT_PUBLIC_ADDRESS is just a "default" address needed for the
 controller configuration. However, this address shall be an IEEE unique address
 as defined in the Page 2859 and Page 416 of Core_v5.2. PLEASE DO NOT USE PUBLIC
 ADDRESS IN ADVERTISING, SCANNING OR ANY OTHER OPERATION UNLESS THE PUBLIC ADDRESS
 IS REALLY AN IEEE ASSIGNED NUMBER. PREFER TO USE THE RANDOM TYPE INSTEAD. */
#define DEFAULT_PUBLIC_ADDRESS { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 }


/****************************************************************/
/* Static functions declaration                                 */
/****************************************************************/
static uint8_t Create_Static_Address( uint8_t Random_Bytes[6] );
static uint8_t Create_Private_Address( uint8_t Random_Bytes[6] );
static uint8_t Create_Private_Resolvable_Address( uint8_t prand[3] );
static uint8_t Check_Bit_Presence( uint8_t Random_Part[], uint8_t Size_In_Bytes );
static void Hash_CallBack_Function(uint8_t EncryptedData[16], uint8_t status);
static void Resolve_Private_Address_CallBack(uint8_t EncryptedData[16], uint8_t status);
static void Confirm_Private_Addr(uint8_t status);
static void LE_Encrypt_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Encrypted_Data[16] );
static void LE_Rand_Complete( CONTROLLER_ERROR_CODES Status, uint8_t Random_Number[8] );
static uint8_t Set_Static_Random_Device_Address( BD_ADDR_TYPE* StaticAddress );
static uint8_t Check_Broadcaster_Parameters( ADVERTISING_PARAMETERS* AdvPar );
static uint8_t Check_Peripheral_Parameters( ADVERTISING_PARAMETERS* AdvPar );
extern uint8_t LE_Write_Address( LE_BD_ADDR_TYPE* Address );
extern LE_BD_ADDR_TYPE* LE_Read_Address( PEER_ADDR_TYPE AddressType );


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static LE_BD_ADDR_TYPE LE_BLUETOOTH_DEVICE_ADDRESS; /* Address used by the link layer in advertising/scanning and connected mode */
static uint8_t Rand_Bytes[16];
static uint8_t plaintextData[16];
static uint8_t hash[3];
static CONFIG_BD_ADDR BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
static EncryptCallBack Encrypt_CallBack = NULL;
static RESOLVE_ADDR_STRUCT ResolveStruct = { .CallBack = NULL };


/****************************************************************/
/* Generate_Device_Address()        							*/
/* Location: 					 								*/
/* Purpose: Generate the static and private addresses used.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
BD_ADDR_TYPE* Generate_Device_Address( SUPPORTED_COMMANDS* HCI_Sup_Cmd, RANDOM_ADDRESS_TYPE AddrType, IRK_TYPE* IRK )
{
	static uint32_t TimeoutCounter = 0;
	static BD_ADDR_TYPE RANDOM_ADDRESS;
	uint8_t status = TRUE;
	GET_BD_ADDR StaticAddr;

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
		switch ( AddrType )
		{
		case STATIC_DEVICE_ADDRESS:
			/* The static address is not subject to modification all the time, only after device power-up. */
			/* So look to the parameters to see if it should update or not */
			StaticAddr = Get_Static_Random_Device_Address( );
			if( !StaticAddr.Status )
			{
				if( Create_Static_Address( &Rand_Bytes[0] ) )
				{
					if( !Set_Static_Random_Device_Address( (BD_ADDR_TYPE*)( &Rand_Bytes ) ) )
					{
						status = FALSE;
					}else
					{
						RANDOM_ADDRESS = *StaticAddr.Ptr;
					}
				}else
				{
					status = FALSE;
				}
			}else
			{
				RANDOM_ADDRESS = *StaticAddr.Ptr;
			}
			BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
			return ( ( status == TRUE ) ? &RANDOM_ADDRESS : NULL );
			break;

		case RESOLVABLE_PRIVATE:
			/* The last part of the byte arrya will be the prand part of private address. */
			/* Check if prand part cpmplies with requeriment and the jump to hash generation. */
			if( !Create_Private_Resolvable_Address( &Rand_Bytes[2 * sizeof(BD_ADDR_TYPE)] ) )
			{
				status = FALSE;
			}
			BD_Config = ( status == TRUE ) ? REQUEST_HASH_CALC : GENERATE_RANDOM_NUMBER_PART_A;
			break;

		case NON_RESOLVABLE_PRIVATE:
		default:
			/* Check non-resolvable private address */
			if( Create_Private_Address( &Rand_Bytes[sizeof(BD_ADDR_TYPE)] ) )
			{
				memcpy( &RANDOM_ADDRESS.Bytes[0], &Rand_Bytes[sizeof(BD_ADDR_TYPE)], sizeof(BD_ADDR_TYPE) );
			}else
			{
				status = FALSE;
			}
			BD_Config = ( status == TRUE ) ? END_ADDRESSES_CONFIG : GENERATE_RANDOM_NUMBER_PART_A;
			break;
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
			memcpy( &RANDOM_ADDRESS.Bytes[0], &hash[0], sizeof(BD_ADDR_TYPE)/2 );
			memcpy( &RANDOM_ADDRESS.Bytes[sizeof(BD_ADDR_TYPE)/2], &Rand_Bytes[ 2 * sizeof(BD_ADDR_TYPE) ], sizeof(BD_ADDR_TYPE)/2 );
			BD_Config = VERIFY_RESOLVABLE_ADDRESS;
			break;

		case VERIFY_RESOLVABLE_ADDRESS:
			TimeoutCounter = 0;
			BD_Config = Resolve_Private_Address( HCI_Sup_Cmd, &RANDOM_ADDRESS, IRK, &Confirm_Private_Addr ) ? WAIT_OPERATION_A : VERIFY_RESOLVABLE_ADDRESS;
			break;

		case END_ADDRESSES_CONFIG:
			BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
			return (&RANDOM_ADDRESS);
			break;

		case WAIT_OPERATION_A:
		case WAIT_OPERATION_B:
			if( TimeBase_DelayMs( &TimeoutCounter, 500, TRUE ) )
			{
				BD_Config = GENERATE_RANDOM_NUMBER_PART_A;
			}
			break;
	}

	return (NULL);

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
GET_BD_ADDR Get_Public_Device_Address( void )
{
	GET_BD_ADDR ReturnVal = { .Status = FALSE };

	LE_BD_ADDR_TYPE* Ptr = LE_Read_Address( PEER_PUBLIC_DEV_ADDR );
	uint8_t Type = ( Ptr->Reserved << 1 ) | ( Ptr->Type );

	if( Type != PEER_PUBLIC_DEV_ADDR ) /* Address is not initialized */
	{
		LE_BD_ADDR_TYPE PublicAddrRecord = { .Address = { DEFAULT_PUBLIC_ADDRESS } };
		PublicAddrRecord.Type = PEER_PUBLIC_DEV_ADDR;
		PublicAddrRecord.Reserved = 0;
		if( !LE_Write_Address( &PublicAddrRecord ) )
		{
			return (ReturnVal);
		}
	}

	ReturnVal.Status = TRUE;
	ReturnVal.Ptr = (BD_ADDR_TYPE*)( &Ptr->Address );

	return (ReturnVal);
}


/****************************************************************/
/* Get_Static_Random_Device_Address()        					*/
/* Location: 					 								*/
/* Purpose: Return the static random address.					*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
GET_BD_ADDR Get_Static_Random_Device_Address( void )
{
	GET_BD_ADDR ReturnVal = { .Status = FALSE };

	LE_BD_ADDR_TYPE* Ptr = LE_Read_Address( PEER_RANDOM_DEV_ADDR );
	uint8_t Type = ( Ptr->Reserved << 1 ) | ( Ptr->Type );

	if( Type == PEER_RANDOM_DEV_ADDR ) /* Address is initialized? */
	{
		ReturnVal.Status = TRUE;
		ReturnVal.Ptr = (BD_ADDR_TYPE*)( &Ptr->Address );
	}

	return (ReturnVal);
}


/****************************************************************/
/* Set_Static_Random_Device_Address()        					*/
/* Location: 					 								*/
/* Purpose: Save the static random address.						*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static uint8_t Set_Static_Random_Device_Address( BD_ADDR_TYPE* StaticAddress )
{
	LE_BD_ADDR_TYPE StaticAddrRecord;

	StaticAddrRecord.Type = PEER_RANDOM_DEV_ADDR;
	StaticAddrRecord.Reserved = 0;
	StaticAddrRecord.Address = *StaticAddress;

	if( !LE_Write_Address( &StaticAddrRecord ) )
	{
		return (FALSE);
	}else
	{
		return (TRUE);
	}
}


/****************************************************************/
/* Get_Identity_Address()        								*/
/* Location: 					 								*/
/* Purpose: Return the identity address of the LE device.		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
IDENTITY_ADDRESS Get_Identity_Address( PEER_ADDR_TYPE Type )
{
	GET_BD_ADDR Addr;
	IDENTITY_ADDRESS Identity;

	memset( &Identity, 0, sizeof(Identity) );

	if ( Type == PEER_PUBLIC_DEV_ADDR )
	{
		Addr = Get_Public_Device_Address();
	}else
	{
		Addr = Get_Static_Random_Device_Address();
	}

	if ( Addr.Status )
	{
		Identity.Address = *( Addr.Ptr );
	}

	return ( Identity );
}


/****************************************************************/
/* Get_LE_Bluetooth_Device_Address()        					*/
/* Location: 					 								*/
/* Purpose: Return the address being used by the LE device. 	*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
LE_BD_ADDR_TYPE* Get_LE_Bluetooth_Device_Address( void )
{
	//TODO: verificar se não deve ser o identity address porque
	//não há como retornar o resolvable address
	return (&LE_BLUETOOTH_DEVICE_ADDRESS);
}


/****************************************************************/
/* Get_Max_Advertising_Data_Length()        					*/
/* Location: 					 								*/
/* Purpose: Get the maximum data bytes allowed for advertising	*/
/* data for advertising.										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Get_Max_Advertising_Data_Length( void )
{
	return ( MAX_ADVERTISING_DATA_LENGTH );
}


/****************************************************************/
/* Get_Max_Scan_Response_Data_Length()        					*/
/* Location: 					 								*/
/* Purpose: Get the maximum data bytes allowed for scan 		*/
/* response data.												*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Get_Max_Scan_Response_Data_Length( void )
{
	return ( MAX_SCAN_RESPONSE_DATA_LENGTH );
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
__attribute__((weak)) uint8_t LE_Write_Address( LE_BD_ADDR_TYPE* Address )
{
	/* Considers address was not saved. This function should be
	 * implemented at application side. */
	return (FALSE);
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
__attribute__((weak)) LE_BD_ADDR_TYPE* LE_Read_Address( PEER_ADDR_TYPE AddressType )
{
	/* Considers address was not saved. This function should be
	 * implemented at application side. */
	return (NULL);
}


/****************************************************************/
/* Check_Advertising_Parameters()      							*/
/* Location: 													*/
/* Purpose: Verify advertising parameters.					 	*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
uint8_t Check_Advertising_Parameters( ADVERTISING_PARAMETERS* AdvPar )
{
	if ( ( AdvPar->Own_Address_Type == OWN_RANDOM_DEV_ADDR ) && ( AdvPar->Own_Random_Address_Type == RESOLVABLE_PRIVATE ) )
	{
		/* We should only configure random as non-resolvable or static random address */
		return (FALSE);
	}

	switch( AdvPar->Role )
	{

	case BROADCASTER:
		return( Check_Broadcaster_Parameters( AdvPar ) );
		break;

	case PERIPHERAL:
		return( Check_Peripheral_Parameters( AdvPar ) );
		break;

		/* Other roles are not allowed in advertising */
	default: break;

	}

	return (FALSE);
}


/****************************************************************/
/* Check_Broadcaster_Parameters()      							*/
/* Location: 													*/
/* Purpose: Verify advertising parameters for broadcaster role.	*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Broadcaster_Parameters( ADVERTISING_PARAMETERS* AdvPar )
{
	/* A broadcaster cannot send connectable advertisements */
	switch( AdvPar->Advertising_Type )
	{
	case ADV_SCAN_IND: /* Scannable */
	case ADV_NONCONN_IND: /* Non connectable */
		if( AdvPar->Privacy )
		{
			/* A Peripheral shall use non-resolvable or resolvable private addresses when in
			   non-connectable mode as defined in Section 9.3.2. (Page 1387 Core_v5.2). */
			if( ( AdvPar->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) || ( AdvPar->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) ||
					( ( AdvPar->Own_Address_Type == OWN_RANDOM_DEV_ADDR ) && ( AdvPar->Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE) ) )
			{
				return (TRUE);
			}
		}else
		{
			return (TRUE);
		}
		break;

	default: break;
	}

	return (FALSE);
}


/****************************************************************/
/* Check_Peripheral_Parameters()      							*/
/* Location: 													*/
/* Purpose: Verify advertising parameters for Peripheral role.	*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Peripheral_Parameters( ADVERTISING_PARAMETERS* AdvPar )
{
	switch( AdvPar->Advertising_Type )
	{
	case ADV_IND: /* Connectable and scannable */
	case ADV_DIRECT_IND_HIGH_DUTY: /* Connectable */
	case ADV_DIRECT_IND_LOW_DUTY: /* Connectable */
		if( AdvPar->Privacy )
		{
			/* The privacy-enabled Peripheral shall use a resolvable private address as the
			advertiser's device address when in connectable mode (Page 1387 Core_v5.2). */
			if( ( AdvPar->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) || ( AdvPar->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) )
			{
				return (TRUE);
			}
		}else
		{
			return (TRUE);
		}
		break;

	default: break;
	}

	return (FALSE);
}


/****************************************************************/
/* Set_Advertising_HostData()      								*/
/* Location: 													*/
/* Purpose: Load advertising/scanning host data.			 	*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
void Set_Advertising_HostData( ADVERTISING_PARAMETERS* AdvPar )
{
	static uint8_t AdvData[MAX_ADVERTISING_DATA_LENGTH];
	static uint8_t ScanRspData[MAX_SCAN_RESPONSE_DATA_LENGTH];

	uint8_t offset;
	uint8_t Length = 0;

	/*------------------------------------ LOAD ADVERTISING DATA -----------------------------------------*/
	offset = Load_Flags( (Flags_Type*)&AdvData[Length], sizeof(AdvData) - Length, AdvPar->Role, AdvPar->DiscoveryMode );

	Length += offset;

	/* In privacy-enabled Peripheral, the device should not send the device name or unique data in the advertising
	data that can be used to recognize the device (Page 1387 Core_v5.2). */
	if( !AdvPar->Privacy ) /* Privacy disabled */
	{
		/* The power level is set as 0, but will be loaded with the right value before advertising */
		offset = Load_Tx_Power_Level( (Tx_Power_Level_Type*)&AdvData[Length], sizeof(AdvData) - Length, 0 );

		Length += offset;

		/* Only add Slave_Conn_Interval_Range_Type to those advertising events that are connectable */
		switch( AdvPar->Advertising_Type )
		{
		case ADV_IND:
		case ADV_DIRECT_IND_HIGH_DUTY:
		case ADV_DIRECT_IND_LOW_DUTY:
			offset = Load_Slave_Conn_Interval_Range( (Slave_Conn_Interval_Range_Type*)&AdvData[Length], sizeof(AdvData) - Length,
					AdvPar->connIntervalmin, AdvPar->connIntervalmax );

			Length += offset;
			break;

		default: break;
		}

		offset = Load_Appearance( (Appearance_Type*)&AdvData[Length], sizeof(AdvData) - Length, TEMPERATURE_SENSOR );

		Length += offset;

		offset = Load_Local_Name( (Local_Name_Type*)&AdvData[Length], sizeof(AdvData) - Length );

		Length += offset;
	}

	AdvPar->HostData.Adv_Data_Length = Length;
	AdvPar->HostData.Adv_Data_Ptr = &AdvData[0];

	/*------------------------------------ LOAD SCAN RESPONSE DATA ---------------------------------------*/
	Length = 0;

	/* In privacy-enabled Peripheral, the device should not send the device name or unique data in the advertising
	data that can be used to recognize the device (Page 1387 Core_v5.2). */
	/* If the advertising data or the scan response data change regularly then those changes should be synchronized with any changes in
	 * private addresses (both local and remote). For this purpose, the Host should not offload the private address generation to the Controller but,
	 * instead, generate private addresses as described in Section 10.7.1.2. (Page 1388 Core_v5.2). */
	if( !AdvPar->Privacy ) /* Privacy disabled */
	{
		offset = Load_Local_Name( (Local_Name_Type*)&ScanRspData[Length], sizeof(ScanRspData) - Length );

		Length += offset;

		offset = Load_Appearance( (Appearance_Type*)&ScanRspData[Length], sizeof(ScanRspData) - Length, TEMPERATURE_SENSOR );

		Length += offset;

		offset = Load_Manufacturer_Specific_Data( (Manufacturer_Specific_Data_Type*)&ScanRspData[Length], sizeof(ScanRspData) - Length, NULL, 0 );

		Length += offset;
	}

	AdvPar->HostData.ScanRsp_Data_Length = Length;
	AdvPar->HostData.Scan_Data_Ptr = &ScanRspData[0];
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
		if( memcmp( &Random_Bytes[0], Get_Public_Device_Address( ).Ptr, sizeof(BD_ADDR_TYPE) ) != 0 )
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
/* Confirm_Private_Addr()      									*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Confirm_Private_Addr(uint8_t status)
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
