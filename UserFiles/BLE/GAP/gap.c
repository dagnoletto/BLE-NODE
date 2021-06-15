

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "gap.h"
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
static char LOCAL_NAME[] = "User";


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static uint8_t Load_Local_Name( Local_Name_Type* Ptr, int16_t ArraySize );
static uint8_t Load_Public_Target_Address( Public_Target_Address_Type* Ptr, int16_t ArraySize,
		BD_ADDR_TYPE BDAddress[], uint8_t NumberOfAddresses );
static uint8_t Load_Random_Target_Address( Random_Target_Address_Type* Ptr, int16_t ArraySize,
		BD_ADDR_TYPE BDAddress[], uint8_t NumberOfAddresses );
static uint8_t Load_LE_Bluetooth_Device_Address( LE_BD_Address_Type* Ptr, int16_t ArraySize );


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
static uint8_t Load_Local_Name( Local_Name_Type* Ptr, int16_t ArraySize )
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
static uint8_t Load_Public_Target_Address( Public_Target_Address_Type* Ptr, int16_t ArraySize,
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
static uint8_t Load_Random_Target_Address( Random_Target_Address_Type* Ptr, int16_t ArraySize,
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
static uint8_t Load_LE_Bluetooth_Device_Address( LE_BD_Address_Type* Ptr, int16_t ArraySize )
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
/* Get_Advertising_Data()        								*/
/* Location: 													*/
/* Purpose: Load advertising data and return the data pointer 	*/
/* and size to the caller.										*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
uint8_t* Get_Advertising_Data( uint8_t* DataSizePtr )
{
	static uint8_t Data[MAX_ADVERTISING_DATA_LENGTH];
	uint8_t offset = 0;
	uint8_t Length = 0;

	offset = Load_Local_Name( (Local_Name_Type*)&Data[Length], sizeof(Data) - Length );

	Length += offset;

	offset = Load_LE_Bluetooth_Device_Address( (LE_BD_Address_Type*)&Data[Length], sizeof(Data) - Length );

	Length += offset;

	*DataSizePtr = Length;

	return ( Data );
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
