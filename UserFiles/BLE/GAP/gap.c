

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "gap.h"
#include "ble_states.h"
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
static char LOCAL_NAME[] = "Pitoca!";


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/


/****************************************************************/
/* Load_Local_Name()        									*/
/* Location: Page 11 Supplement CSS_v9							*/
/* Purpose: The Local Name data type shall be the same as, or a */
/* shortened version of, the local name assigned to the device. */
/* The Local Name data type value indicates if the name is 		*/
/* complete or shortened. If the name is shortened, the complete*/
/* name can be read using the remote name request procedure over*/
/* BR/EDR or by reading the device name characteristic after the*/
/* connection has been established using GATT. A shortened name */
/* shall only contain contiguous characters from the beginning  */
/* of the full name. For example, if the device name is 		*/
/* ‘BT_Device_Name’ then the shortened name could be ‘BT_Device’*/
/* or ‘BT_Dev’.													*/
/* Parameters: none				         						*/
/* Return: offset from the loading pointer						*/
/* Description:													*/
/****************************************************************/
uint8_t Load_Local_Name( Local_Name_Type* Ptr, int16_t ArraySize )
{
	uint8_t ArraySizeOK = ( ArraySize >= ( sizeof(Local_Name_Type) + strlen(LOCAL_NAME) ) ) ? TRUE : FALSE;
	uint8_t CopySize;

	if( ( strlen(LOCAL_NAME) <= 248 ) && ( ArraySizeOK ) )
	{

		/* Copy all LOCAL_NAME */
		Ptr->type = COMPLETE_LOCAL_NAME_TYPE;
		CopySize = strlen(LOCAL_NAME);

	}else if( ArraySizeOK )
	{

		/* The maximum local name is 248. If the name is higher than that, consider shortened name */
		/* Copy first 248 bytes of LOCAL_NAME */
		Ptr->type = SHORTENED_LOCAL_NAME_TYPE;
		CopySize = 248;

	}else if( ArraySize > sizeof(Local_Name_Type) )
	{

		/* Copy ( ArraySize - sizeof(Local_Name_Type) ) from LOCAL_NAME */
		Ptr->type = SHORTENED_LOCAL_NAME_TYPE;
		CopySize = ArraySize - sizeof(Local_Name_Type);

	}else
	{
		return (0);
	}

	Ptr->length = sizeof( Ptr->type ) + CopySize;
	memcpy( &Ptr->local_Name, &LOCAL_NAME, CopySize );

	return ( sizeof(Local_Name_Type) + CopySize );

}


/****************************************************************/
/* Load_Public_Target_Address()        							*/
/* Location: Page 19 Supplement CSS_v9							*/
/* Purpose: The Public Target Address data type defines the 	*/
/* address of one or more intended recipients of an				*/
/* advertisement when one or more devices were bonded using a 	*/
/* public address. This data type is intended to be used to 	*/
/* avoid a situation where a bonded device unnecessarily 		*/
/* responds to an advertisement intended for another bonded 	*/
/* device.														*/
/* Parameters: none				         						*/
/* Return: offset from the loading pointer						*/
/* Description:													*/
/****************************************************************/
uint8_t Load_Public_Target_Address( Public_Target_Address_Type* Ptr, int16_t ArraySize,
		BD_ADDR_TYPE BDAddress[], uint8_t NumberOfAddresses )
{
	uint8_t length_total = sizeof(Public_Target_Address_Type) + sizeof(BD_ADDR_TYPE) * NumberOfAddresses;

	if( ( ArraySize >= length_total ) && ( NumberOfAddresses < 43 ) )
	{

		Ptr->type = PUBLIC_TARGET_ADDRESS_TYPE;
		Ptr->length = length_total - sizeof( Ptr->length );

		for( uint8_t i = 0; i < NumberOfAddresses; i++)
		{
			memcpy( &Ptr->Public_Target_Address[i], &BDAddress[i], sizeof(BD_ADDR_TYPE) );
		}

		return ( length_total );
	}

	return (0);
}


/****************************************************************/
/* Load_Random_Target_Address()        							*/
/* Location: Page 19 Supplement CSS_v9							*/
/* Purpose: The Random Target Address data type defines the 	*/
/* address of one or more intended recipients of an 			*/
/* advertisement when one or more devices were bonded using a 	*/
/* random address. This data type is intended to be used to 	*/
/* avoid a situation where a bonded device unnecessarily 		*/
/* responds to an advertisement intended for another bonded 	*/
/* device.														*/
/* Parameters: none				         						*/
/* Return: offset from the loading pointer						*/
/* Description:													*/
/****************************************************************/
uint8_t Load_Random_Target_Address( Random_Target_Address_Type* Ptr, int16_t ArraySize,
		BD_ADDR_TYPE BDAddress[], uint8_t NumberOfAddresses )
{
	uint8_t length_total = sizeof(Random_Target_Address_Type) + sizeof(BD_ADDR_TYPE) * NumberOfAddresses;

	if( ( ArraySize >= length_total ) && ( NumberOfAddresses < 43 ) )
	{

		Ptr->type = RANDOM_TARGET_ADDRESS_TYPE;
		Ptr->length = length_total - sizeof( Ptr->length );

		for( uint8_t i = 0; i < NumberOfAddresses; i++)
		{
			memcpy( &Ptr->Random_Target_Address[i], &BDAddress[i], sizeof(BD_ADDR_TYPE) );
		}

		return ( length_total );
	}

	return (0);
}


/****************************************************************/
/* Load_LE_Bluetooth_Device_Address()        					*/
/* Location: Page 20 Supplement CSS_v9							*/
/* Purpose: The LE Bluetooth Device Address data type defines 	*/
/* the device address of the local device and the address type 	*/
/* on the LE transport.											*/
/* Parameters: none				         						*/
/* Return: offset from the loading pointer						*/
/* Description:													*/
/****************************************************************/
uint8_t Load_LE_Bluetooth_Device_Address( LE_BD_Address_Type* Ptr, int16_t ArraySize )
{
	if( ArraySize >= sizeof(LE_BD_Address_Type) )
	{

		Ptr->type = LE_BLUETOOTH_DEVICE_ADDRESS_TYPE;
		Ptr->length = sizeof(LE_BD_Address_Type) - sizeof( Ptr->length );

		memcpy( &Ptr->LE_BD_Address, Get_LE_Bluetooth_Device_Address(), sizeof(Ptr->LE_BD_Address) );

		return ( sizeof(LE_BD_Address_Type) );
	}

	return (0);
}


/****************************************************************/
/* Load_Flags()        											*/
/* Location: Page 12 Supplement CSS_v9							*/
/* Purpose: The Flags data type contains one bit Boolean flags. */
/* The Flags data type shall be included when any of the Flag 	*/
/* bits are non-zero and the advertising packet is connectable, */
/* otherwise the Flags data type may be omitted. All 0x00 		*/
/* octets after the last non-zero octet shall be omitted from 	*/
/* the value transmitted. Note: If the Flags AD type is not 	*/
/* present in a non-connectable advertisement, the Flags should */
/* be considered as unknown and no assumptions should be made 	*/
/* by the scanner. Flags used over the LE physical channel are: */
/* - Limited Discoverable Mode									*/
/* - General Discoverable Mode									*/
/* - BR/EDR Not Supported										*/
/* - Simultaneous LE and BR/EDR to Same Device Capable			*/
/* (Controller)													*/
/* - Simultaneous LE and BR/EDR to Same Device Capable (Host)	*/
/* The LE Limited Discoverable Mode and LE General Discoverable */
/* Mode flags shall be ignored when received over the BR/EDR 	*/
/* physical channel. The ‘BR/EDR Not Supported’ flag shall be 	*/
/* set to 0 when sent over the BR/EDR physical channel.			*/
/* Parameters: none				         						*/
/* Return: offset from the loading pointer						*/
/* Description:													*/
/****************************************************************/
uint8_t Load_Flags( Flags_Type* Ptr, int16_t ArraySize,
		GAP_LE_ROLE Role, GAP_DISCOVERY_MODE DiscoveryMode )
{
	/* The Flags AD type shall not be included in the scan response data. The advertising
	data shall not contain more than one instance of the Flags AD type. The
	Flags AD type shall be included in the advertising data if any of the bits are
	non-zero. The Flags AD type may be omitted from the advertising data if all of
	the bits are zero. */

	if( ArraySize >= sizeof(Flags_Type) )
	{
		Ptr->type = FLAGS_TYPE;
		Ptr->length = sizeof(Flags_Type) - sizeof( Ptr->length );

		SUPPORTED_FEATURES* FeaturesPtr = Get_Supported_Features( );

		if( Role == PERIPHERAL)
		{
			switch ( DiscoveryMode )
			{
			case LIMITED_DISCOVERABLE_MODE:
				Ptr->Flags.Bits.LE_General_Discoverable_Mode = 0;
				Ptr->Flags.Bits.LE_Limited_Discoverable_Mode = 1;
				break;

			case GENERAL_DISCOVERABLE_MODE:
				Ptr->Flags.Bits.LE_General_Discoverable_Mode = 1;
				Ptr->Flags.Bits.LE_Limited_Discoverable_Mode = 0;
				break;

			case NON_DISCOVERABLE_MODE:
			default:
				Ptr->Flags.Bits.LE_General_Discoverable_Mode = 0;
				Ptr->Flags.Bits.LE_Limited_Discoverable_Mode = 0;
				break;
			}
		}else
		{
			Ptr->Flags.Bits.LE_General_Discoverable_Mode = 0;
			Ptr->Flags.Bits.LE_Limited_Discoverable_Mode = 0;
		}

		Ptr->Flags.Bits.BR_EDR_Not_Supported = FeaturesPtr->Bits.BR_EDR_Not_Supported;
		Ptr->Flags.Bits.Simul_LE_BR_EDR_Same_Dev_Capable_Controller = FeaturesPtr->Bits.Simultaneous_LE_and_BR_EDR_to_Same_Device_Capable;
		Ptr->Flags.Bits.Simul_LE_BR_EDR_Same_Dev_Capable_Host = 0; /* TODO: this values should be loaded from HCI_Read_Local_Extended_Features() function when implemented. */

		/* Only include flags if at least one bit is set */
		if( Ptr->Flags.Val )
		{
			return ( sizeof(Flags_Type) );
		}
	}

	return (0);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
