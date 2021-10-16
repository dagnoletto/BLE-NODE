

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"
#include "hosted_functions.h"
#include "security_manager.h"
#include "ble_advertising.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	DISABLE_ADVERTISING,
	DISABLE_ADDRESS_RESOLUTION,
	CLEAR_RESOLVING_LIST,
	ADD_TO_RESOLVING_LIST,
	VERIFY_ADDRESS,
	WAIT_FOR_NEW_LOCAL_READ,
	GENERATE_NON_RESOLVABLE_ADDRESS,
	SET_RANDOM_ADDRESS,
	SET_PEER_ADDRESS,
	WAIT_FOR_NEW_PEER_READ,
	ENABLE_ADDRESS_RESOLUTION,
	SET_ADV_PARAMETERS,
	LOAD_ADV_DATA,
	READ_ADV_POWER,
	SET_ADV_POWER,
	SET_ADV_DATA,
	SET_SCAN_RSP_DATA,
	ENABLE_ADVERTISING,
	END_ADV_CONFIG,
	FAILED_ADV_CONFIG,
	WAIT_OPERATION,
}ADV_CONFIG_STEPS;


typedef struct
{
	ADV_CONFIG_STEPS Actual;
	ADV_CONFIG_STEPS Next;
	ADV_CONFIG_STEPS Prev;
}ADV_CONFIG;


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
int8_t Advertising_Config( void );
void Advertising( void );
static void Free_Advertising_Parameters( void );
static ADV_CONFIG Update_Random_Address( void );
static ADV_CONFIG Check_Local_IRK( RESOLVING_RECORD* ResolvingRecord, uint8_t RPAInController );
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address );
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void Read_Peer_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Peer_Resolvable_Address );
static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Advertising_Parameters_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( CONTROLLER_ERROR_CODES Status, int8_t TX_Power_Level );
static void LE_Set_Data_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static uint8_t Check_Broadcaster_Parameters( ADVERTISING_PARAMETERS* AdvPar );
static uint8_t Check_Peripheral_Parameters( ADVERTISING_PARAMETERS* AdvPar );


/****************************************************************/
/* extern functions declaration                                 */
/****************************************************************/
extern void Set_BLE_State( BLE_STATES NewBLEState );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* Global variables definition                                  */
/****************************************************************/


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/
static ADVERTISING_PARAMETERS* AdvertisingParameters = NULL;
static ADV_CONFIG AdvConfig = { DISABLE_ADVERTISING, DISABLE_ADVERTISING, DISABLE_ADVERTISING };
static BD_ADDR_TYPE RandomAddress;
static uint16_t SM_Resolving_List_Index;


/****************************************************************/
/* Enter_Advertising_Mode()        	 							*/
/* Location: 					 								*/
/* Purpose: Put the controller in advertising mode. 			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Enter_Advertising_Mode( ADVERTISING_PARAMETERS* AdvPar )
{
	if( Get_BLE_State( ) == STANDBY_STATE )
	{
		if( Check_Advertising_Parameters( AdvPar )  )
		{
			Free_Advertising_Parameters( );

			AdvertisingParameters = malloc( sizeof(ADVERTISING_PARAMETERS) );

			if( AdvertisingParameters != NULL )
			{
				*AdvertisingParameters = *AdvPar;
				AdvertisingParameters->Original_Own_Address_Type = AdvertisingParameters->Own_Address_Type;
				AdvertisingParameters->Original_Own_Random_Address_Type = AdvertisingParameters->Own_Random_Address_Type;
				AdvertisingParameters->Original_Peer_Address = AdvertisingParameters->Peer_Address;

				AdvConfig.Actual = DISABLE_ADVERTISING;

				Set_BLE_State( CONFIG_ADVERTISING );
				return (TRUE);
			}
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Exit_Advertising_Mode()        	 							*/
/* Location: 					 								*/
/* Purpose: Put the controller in standby exiting from 			*/
/* advertising mode.		 									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Exit_Advertising_Mode( BLE_STATES CurrentState )
{
	if( CurrentState == ADVERTISING_STATE )
	{
		Free_Advertising_Parameters( );

		AdvConfig.Actual = DISABLE_ADVERTISING;

		return (TRUE);
	}else if( CurrentState == CONFIG_ADVERTISING )
	{
		//TODO: Here we have to test if we are in a safe step to disable the state. By the contrary, we may be
		//blocked in a function or step
		//Acredito que seja melhor forçar o AdvConfig.Actual para um ponto da configuração em que a máquina de estados
		//cancele a configuração e depois disso ela mesmo chame a entrada para stand-by, retornando (-1)

		//		if( AdvConfig.Actual != REQUEST_ADV_CANCEL )
		//		{
		//			AdvConfig.Actual = REQUEST_ADV_CANCEL;
		//		}
		//TODO: Não seve entrar em stand-by por aqui porque a máquina de estados deve continuar em config. Ela entrará em
		//stand-by sozinha.

		if( AdvConfig.Actual == FAILED_ADV_CONFIG )
		{
			AdvConfig.Actual = DISABLE_ADVERTISING;
			Free_Advertising_Parameters( );
			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Free_Advertising_Parameters()        	   					*/
/* Location: 					 								*/
/* Purpose: Free allocated advertising parameters memory		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Free_Advertising_Parameters( void )
{
	if( AdvertisingParameters != NULL )
	{
		free(AdvertisingParameters);
		AdvertisingParameters = NULL;
	}
}


/****************************************************************/
/* Advertising_Config()        	   								*/
/* Location: 					 								*/
/* Purpose: Configure advertising type							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
int8_t Advertising_Config( void )
{
	static uint32_t AdvConfigTimeout = 0;
	static RESOLVING_RECORD* Ptr;

	switch( AdvConfig.Actual )
	{
	case DISABLE_ADVERTISING:
		Ptr = NULL;
		AdvertisingParameters->Own_Address_Type = AdvertisingParameters->Original_Own_Address_Type;
		AdvertisingParameters->Own_Random_Address_Type = AdvertisingParameters->Original_Own_Random_Address_Type;
		AdvertisingParameters->Peer_Address = AdvertisingParameters->Original_Peer_Address;
		AdvConfigTimeout = 0;
		AdvertisingParameters->Counter = 0;
		AdvConfig.Next = DISABLE_ADDRESS_RESOLUTION;
		AdvConfig.Prev = DISABLE_ADVERTISING;
		AdvConfig.Actual = HCI_LE_Set_Advertising_Enable( FALSE, &LE_Set_Advertising_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADVERTISING;
		break;

	case DISABLE_ADDRESS_RESOLUTION:
		AdvConfig.Next = CLEAR_RESOLVING_LIST;
		AdvConfig.Prev = DISABLE_ADDRESS_RESOLUTION;
		AdvConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( FALSE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADDRESS_RESOLUTION;
		break;

	case CLEAR_RESOLVING_LIST:
		if( ( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) || ( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) )
		{
			IDENTITY_ADDRESS PeerId;
			/* Check if this peer device is in the resolving list */
			PeerId.Type = AdvertisingParameters->Peer_Address_Type;
			PeerId.Address = AdvertisingParameters->Peer_Address;
			Ptr = Get_Record_From_Peer_Identity( &PeerId );
			if( Ptr != NULL )
			{
				/* This device is in the Host's list. Clear the controller list to add all list again afterwards */
				SM_Resolving_List_Index = 0;
				AdvConfig.Actual = HCI_LE_Clear_Resolving_List( &LE_Clear_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : CLEAR_RESOLVING_LIST;
			}else
			{
				/* This peer device identity is not in the Host list */
				AdvConfig.Actual = VERIFY_ADDRESS;
			}
		}else
		{
			AdvConfig.Actual = VERIFY_ADDRESS;
		}
		break;

	case ADD_TO_RESOLVING_LIST:
		/* Check if we have bonded devices to add to the resolving list */
		if ( SM_Resolving_List_Index < Get_Number_Of_Resolving_Records() )
		{
			DEVICE_IDENTITY* DevId = &( Get_Record_From_Index( SM_Resolving_List_Index )->Peer );
			/* Here we add a device to the controller's resolving list if it exists in the Host's list */
			AdvConfig.Actual = HCI_LE_Add_Device_To_Resolving_List( DevId->Peer_Identity_Address.Type, DevId->Peer_Identity_Address.Address,
					&DevId->Peer_IRK, &DevId->Local_IRK, &LE_Add_Device_To_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : ADD_TO_RESOLVING_LIST;
		}else
		{
			AdvConfig.Actual = VERIFY_ADDRESS;
		}
		break;

	case VERIFY_ADDRESS:
	{
		AdvConfigTimeout = 0;
		switch ( AdvertisingParameters->Own_Address_Type )
		{
		case OWN_RANDOM_DEV_ADDR:
			AdvConfig.Actual = Update_Random_Address( ).Actual;
			break;

		case OWN_RESOL_OR_PUBLIC_ADDR:
		case OWN_RESOL_OR_RANDOM_ADDR:
			if ( Get_Local_Version_Information()->HCI_Version > CORE_SPEC_4_1 )
			{
				/* Above version 4.1, the controller generates the addresses.
				 * However, we still have to check if local IRK is not null because controller does'nt have
				 * the local identity to do it. */
				if( Ptr != NULL )
				{
					AdvConfig.Actual = Check_Local_IRK( Ptr, TRUE ).Actual;
				}else if( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR )
				{
					AdvConfig.Actual = SET_PEER_ADDRESS;
				}else
				{
					AdvConfig.Actual = Update_Random_Address( ).Actual;
				}
				/* TODO: here we assume the controller automatically loads the address based on the list and
				 * we don't need to retrieve the address using HCI_LE_Read_Local_Resolvable_Address command. */
			}else
			{
				/* For lower versions, the host must generate the resolvable address */
				/* Check if this device identity is in the Host resolving list */
				if ( Ptr != NULL )
				{
					AdvConfig.Actual = Check_Local_IRK( Ptr, FALSE ).Actual;
				}else
				{
					if( AdvertisingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR )
					{
						/* It is NOT on the list, we shall use the public address. */
						AdvertisingParameters->Own_Address_Type = OWN_PUBLIC_DEV_ADDR;
						AdvConfig.Actual = SET_PEER_ADDRESS;
					}else
					{
						/* It is NOT on the list, we shall use the random address from LE_Set_Random_Address. */
						AdvertisingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
						AdvConfig.Actual = Update_Random_Address( ).Actual;
					}
				}
			}
			break;

			/* The default is public device address */
		case OWN_PUBLIC_DEV_ADDR:
		default:
			AdvConfig.Actual = SET_PEER_ADDRESS;
			break;
		}
	}
	break;

	case WAIT_FOR_NEW_LOCAL_READ:
		AdvertisingParameters->Own_Address_Type = AdvertisingParameters->Original_Own_Address_Type;
		AdvertisingParameters->Peer_Address = AdvertisingParameters->Original_Peer_Address;
		if( TimeBase_DelayMs( &AdvConfigTimeout, 500, TRUE ) )
		{
			AdvConfig.Actual = VERIFY_ADDRESS;
		}
		break;

	case GENERATE_NON_RESOLVABLE_ADDRESS:
	{
		BD_ADDR_TYPE* Ptr = Generate_Device_Address( Get_Supported_Commands(), NON_RESOLVABLE_PRIVATE, NULL, 2 );
		if ( Ptr != NULL )
		{
			RandomAddress = *Ptr;
			AdvConfig.Actual = SET_RANDOM_ADDRESS;
		}
	}
	break;

	case SET_RANDOM_ADDRESS:
		AdvConfigTimeout = 0;
		AdvConfig.Next = SET_PEER_ADDRESS;
		AdvConfig.Actual = HCI_LE_Set_Random_Address( RandomAddress, &LE_Set_Random_Address_Complete, NULL ) ? WAIT_OPERATION : SET_RANDOM_ADDRESS;
		break;

	case SET_PEER_ADDRESS:
		AdvConfigTimeout = 0;
		if( ( Get_Local_Version_Information()->HCI_Version > CORE_SPEC_4_1 ) && ( AdvertisingParameters->Own_Address_Type == AdvertisingParameters->Original_Own_Address_Type ) )
		{
			/* The Peer_Address_Type and Peer_Address already contains the peer's device identity
			 * that is going to be used by the controller to generate and use the peer's Resolvable Private
			 * Address. So, everything should be fine. No modification of the original own address was done */
			/* TODO: Here we assume the controller will generate a peer address equals to peer's device identity in case the peerIRK is
			 * set to all zeros. According to Page 3023 Core_v5.2:
			 * If the Host, when populating the resolving list, sets a peer IRK to all zeros, then the peer address used within an
			 * advertising physical channel PDU shall use the peer’s Identity Address, which is provided by the Host. */
			AdvConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
		}else if( ( AdvertisingParameters->Advertising_Type == ADV_DIRECT_IND_HIGH_DUTY || AdvertisingParameters->Advertising_Type == ADV_DIRECT_IND_LOW_DUTY ) &&
				( AdvertisingParameters->Original_Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR || AdvertisingParameters->Original_Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) )
		{
			/* Direct advertising is used, which means the peer address may be resolvable if the peer's identity is in the resolving list */
			if( Ptr != NULL )
			{
				/* Our hosted simulated function already deals with the condition stated in Page 3023 Core_v5.2:
				 * If the Host, when populating the resolving list, sets a peer IRK to all zeros, then the peer address used within an
				 * advertising physical channel PDU shall use the peer’s Identity Address, which is provided by the Host. TODO: However, we are
				 * not sure if versions of the controller above 4.1 do the same. We assume yes. */
				AdvConfig.Actual = HCI_LE_Read_Peer_Resolvable_Address( Ptr->Peer.Peer_Identity_Address.Type,
						Ptr->Peer.Peer_Identity_Address.Address, &Read_Peer_Resolvable_Address_Complete, NULL ) ? WAIT_OPERATION : WAIT_FOR_NEW_PEER_READ;
			}else
			{
				/* It is not on the list, so proceed with whatever peer address loaded */
				AdvConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
			}
		}else
		{
			AdvConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
		}
		break;

	case WAIT_FOR_NEW_PEER_READ:
		if( TimeBase_DelayMs( &AdvConfigTimeout, 500, TRUE ) )
		{
			AdvConfig.Actual = SET_PEER_ADDRESS;
		}
		break;

	case ENABLE_ADDRESS_RESOLUTION:
		if( ( Ptr != NULL ) && ( AdvertisingParameters->Privacy ) )
		{
			AdvConfig.Next = SET_ADV_PARAMETERS;
			AdvConfig.Prev = ENABLE_ADDRESS_RESOLUTION;
			AdvConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( TRUE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADDRESS_RESOLUTION;
		}else
		{
			AdvConfig.Actual = SET_ADV_PARAMETERS;
		}
		break;

	case SET_ADV_PARAMETERS:
		AdvConfigTimeout = 0;
		AdvConfig.Next = LOAD_ADV_DATA;
		AdvConfig.Actual = HCI_LE_Set_Advertising_Parameters( AdvertisingParameters->Advertising_Interval_Min, AdvertisingParameters->Advertising_Interval_Max, AdvertisingParameters->Advertising_Type,
				AdvertisingParameters->Own_Address_Type, AdvertisingParameters->Peer_Address_Type, AdvertisingParameters->Peer_Address,
				AdvertisingParameters->Advertising_Channel_Map, AdvertisingParameters->Advertising_Filter_Policy, &LE_Set_Advertising_Parameters_Complete, NULL ) ? WAIT_OPERATION : SET_ADV_PARAMETERS;
		break;

	case LOAD_ADV_DATA:
		Set_Advertising_HostData( AdvertisingParameters );
		if( ( AdvertisingParameters->HostData.Adv_Data_Length <= Get_Max_Advertising_Data_Length() )
				&& ( AdvertisingParameters->HostData.ScanRsp_Data_Length <= Get_Max_Scan_Response_Data_Length() ) )
		{
			AdvConfig.Actual = READ_ADV_POWER;
		}else
		{
			AdvConfig.Actual = FAILED_ADV_CONFIG;
		}
		break;

	case READ_ADV_POWER:
		AdvConfigTimeout = 0;
		if( Get_Supported_Commands()->Bits.HCI_LE_Read_Advertising_Physical_Channel_Tx_Power )
		{
			AdvConfig.Next = SET_ADV_POWER;
			AdvConfig.Actual = HCI_LE_Read_Advertising_Physical_Channel_Tx_Power( &LE_Read_Advertising_Physical_Channel_Tx_Power_Complete, NULL ) ? WAIT_OPERATION : READ_ADV_POWER;
		}else
		{
			AdvConfig.Actual = SET_ADV_DATA;
		}
		break;

	case SET_ADV_POWER:
		/* So far there is no HCI standard command to adjust specifically the advertising power, this step is for the future */
		/* We can used the ACI_Hal_Set_Tx_Power_Level from vendor_specific_hci, but is for all channels, not only advertising */
		AdvConfig.Actual = SET_ADV_DATA;
		break;

	case SET_ADV_DATA:
		AdvConfigTimeout = 0;
		AdvConfig.Next = SET_SCAN_RSP_DATA;
		AdvConfig.Prev = SET_ADV_DATA;
		AdvConfig.Actual = HCI_LE_Set_Advertising_Data( AdvertisingParameters->HostData.Adv_Data_Length, AdvertisingParameters->HostData.Adv_Data_Ptr, &LE_Set_Data_Complete, NULL ) ? WAIT_OPERATION : SET_ADV_DATA;
		break;

	case SET_SCAN_RSP_DATA:
		AdvConfigTimeout = 0;
		AdvConfig.Next = ENABLE_ADVERTISING;
		AdvConfig.Prev = SET_SCAN_RSP_DATA;
		AdvConfig.Actual = HCI_LE_Set_Scan_Response_Data( AdvertisingParameters->HostData.ScanRsp_Data_Length, AdvertisingParameters->HostData.Scan_Data_Ptr, &LE_Set_Data_Complete, NULL ) ? WAIT_OPERATION : SET_SCAN_RSP_DATA;
		break;

	case ENABLE_ADVERTISING:
		AdvConfigTimeout = 0;
		AdvConfig.Next = END_ADV_CONFIG;
		AdvConfig.Prev = ENABLE_ADVERTISING;
		AdvConfig.Actual = HCI_LE_Set_Advertising_Enable( TRUE, &LE_Set_Advertising_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADVERTISING;
		break;

	case END_ADV_CONFIG:
		AdvConfig.Actual = DISABLE_ADVERTISING;
		return (TRUE);
		break;

	case FAILED_ADV_CONFIG:
		Free_Advertising_Parameters( );
		return (-1); /* Failed condition */
		break;

	case WAIT_OPERATION:
		if( TimeBase_DelayMs( &AdvConfigTimeout, 500, TRUE ) )
		{
			AdvConfig.Actual = DISABLE_ADVERTISING;
			AdvConfig.Next = DISABLE_ADVERTISING;
		}
		break;

	default:
		break;
	}

	return (FALSE);
}


/****************************************************************/
/* Update_Random_Address()        	   							*/
/* Location: 					 								*/
/* Purpose: Update random address based on advertising 			*/
/* parameters.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static ADV_CONFIG Update_Random_Address( void )
{
	ADV_CONFIG AdvStep;

	if( AdvertisingParameters->Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE )
	{
		AdvStep.Actual = GENERATE_NON_RESOLVABLE_ADDRESS;
	}else if( AdvertisingParameters->Own_Random_Address_Type == STATIC_DEVICE_ADDRESS )
	{
		RandomAddress = *( Get_Static_Random_Device_Address( ).AddrPtr );
		AdvStep.Actual = SET_RANDOM_ADDRESS;
	}else
	{
		AdvStep.Actual = FAILED_ADV_CONFIG;
	}

	return ( AdvStep );
}


/****************************************************************/
/* Check_Local_IRK()        	   								*/
/* Location: Page 3023 Core_v5.2								*/
/* Purpose: Test if the local IRK is null.			 			*/
/* parameters.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:	If the Host, when populating the resolving 		*/
/* list, sets a local IRK to all zeros, then any local address 	*/
/* used within an advertising physical channel PDU shall use 	*/
/* the local Identity Address, which is provided by the Host.	*/
/****************************************************************/
static ADV_CONFIG Check_Local_IRK( RESOLVING_RECORD* ResolvingRecord, uint8_t RPAInController )
{
	ADV_CONFIG AdvStep;

	uint8_t LocalIRKNull = Check_NULL_IRK( &ResolvingRecord->Peer.Local_IRK );

	if ( LocalIRKNull )
	{
		/* The local IRK is null, we shall use the local identity instead. */
		if ( ResolvingRecord->Local_Identity_Address.Type == PEER_RANDOM_DEV_ADDR )
		{
			/* Random static local identity */
			AdvertisingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
			AdvertisingParameters->Own_Random_Address_Type = STATIC_DEVICE_ADDRESS;
			AdvStep.Actual = Update_Random_Address( ).Actual;
		}else
		{
			/* Public identity address */
			AdvertisingParameters->Own_Address_Type = OWN_PUBLIC_DEV_ADDR;
			AdvStep.Actual = SET_PEER_ADDRESS;
		}
	}else if( !RPAInController )
	{
		/* Make it random for controller 4.1 compatibility */
		/* RPAInController - Resolvable Private Address in controller is not available, call this function to load
		 * the RPA. */
		AdvertisingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
		/* We should use RPA generated by the controller. */
		AdvStep.Actual = HCI_LE_Read_Local_Resolvable_Address( ResolvingRecord->Peer.Peer_Identity_Address.Type,
				ResolvingRecord->Peer.Peer_Identity_Address.Address, &Read_Local_Resolvable_Address_Complete, NULL ) ? WAIT_OPERATION : WAIT_FOR_NEW_LOCAL_READ;
	}else
	{
		AdvStep.Actual = SET_PEER_ADDRESS;
	}

	return (AdvStep);
}


/****************************************************************/
/* LE_Clear_Resolving_List_Complete()      						*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ADD_TO_RESOLVING_LIST;
}


/****************************************************************/
/* LE_Set_Address_Resolution_Enable_Complete()    				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? AdvConfig.Next : AdvConfig.Prev;
}


/****************************************************************/
/* LE_Add_Device_To_Resolving_List_Complete()	      			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	if ( Status == COMMAND_SUCCESS )
	{
		AdvConfig.Actual = ADD_TO_RESOLVING_LIST;
		SM_Resolving_List_Index++;
	}else if( Status == MEM_CAPACITY_EXCEEDED )
	{
		AdvConfig.Actual = VERIFY_ADDRESS;
	}else
	{
		AdvConfig.Actual = ADD_TO_RESOLVING_LIST;
	}
}


/****************************************************************/
/* Read_Local_Resolvable_Address_Complete()        	   			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address )
{
	if( Status == COMMAND_SUCCESS )
	{
		AdvConfig.Actual = SET_RANDOM_ADDRESS;
		RandomAddress = *Local_Resolvable_Address;
	}else
	{
		AdvConfig.Actual = WAIT_FOR_NEW_LOCAL_READ;
	}
}


/****************************************************************/
/* Read_Peer_Resolvable_Address_Complete()        	   			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void Read_Peer_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Peer_Resolvable_Address )
{
	if( Status == COMMAND_SUCCESS )
	{
		AdvertisingParameters->Peer_Address = *Peer_Resolvable_Address;
		AdvConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
	}else
	{
		AdvConfig.Actual = WAIT_FOR_NEW_PEER_READ;
	}
}


/****************************************************************/
/* Advertising()        	   									*/
/* Location: 					 								*/
/* Purpose: Advertising											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Advertising( void )
{
	/* In privacy-enabled Peripheral, the Host shall set a timer equal to
	 * TGAP(private_addr_int). The Host shall generate a new resolvable
	 * private address or non-resolvable private address when the timer
	 * TGAP(private_addr_int) expires. */

	/* If the Controller has address resolution and that is enabled, the RPA is
	 * automatically generated given the timeout for the controller. However, in versions
	 * lower that 4.1, this functionality does not exist so we need to generate again.
	 * For non-resolvable private address (Broadcaster/non-connectable condition) we shall
	 * generate since the controller will not do it. */

	/* TODO: Here we consider the address resolution is always enabled in advertising.
	 * Even for versions higher than 4.1, this function shall be called if address resolution
	 * is disabled. We lay on the fact address resolution is always enabled in the controller
	 * for versions above 4.1. */

	/* TODO: the AdvertisingParameters can be deallocated at any time. A protection scheme should be employed
	 * as this function may be executing while AdvertisingParameters is being deallocated. */
	if( AdvertisingParameters->Privacy && ( ( Get_Local_Version_Information()->HCI_Version <= CORE_SPEC_4_1 ) ||
			( ( AdvertisingParameters->Original_Own_Address_Type == OWN_RANDOM_DEV_ADDR ) &&
					( AdvertisingParameters->Original_Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE ) ) ) )
	{
		if( TimeBase_DelayMs( &AdvertisingParameters->Counter, TGAP_PRIVATE_ADDR_INT, TRUE ) )
		{
			Set_BLE_State( CONFIG_ADVERTISING );
		}
	}
}


/****************************************************************/
/* LE_Set_Advertising_Enable_Complete()        	   				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Advertising_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? AdvConfig.Next : AdvConfig.Prev;
}


/****************************************************************/
/* LE_Set_Advertising_Parameters_Complete()        	 			*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Advertising_Parameters_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? AdvConfig.Next : SET_ADV_PARAMETERS;
}


/****************************************************************/
/* LE_Set_Random_Address_Complete()        	 					*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? AdvConfig.Next : SET_RANDOM_ADDRESS;
}


/****************************************************************/
/* LE_Read_Advertising_Physical_Channel_Tx_Power_Complete()    	*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Read_Advertising_Physical_Channel_Tx_Power_Complete( CONTROLLER_ERROR_CODES Status, int8_t TX_Power_Level )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? AdvConfig.Next : READ_ADV_POWER;

	if( Status == COMMAND_SUCCESS )
	{
		Tx_Power_Level_Type* Ptr;
		uint8_t* DataPtr = AdvertisingParameters->HostData.Adv_Data_Ptr;
		int16_t DataSize = AdvertisingParameters->HostData.Adv_Data_Length;


		do
		{
			Ptr = Get_AD_Type_Ptr( TX_POWER_LEVEL_TYPE, DataPtr, DataSize );
			if( Ptr != NULL )
			{
				Ptr->Tx_Power_Level = TX_Power_Level;
				DataPtr = (uint8_t*)( (uint32_t)( Ptr ) + (Ptr->length) + 1 ); /* Points to the next structure */
				DataSize = AdvertisingParameters->HostData.Adv_Data_Length - ( (uint32_t)( DataPtr ) - (uint32_t)( AdvertisingParameters->HostData.Adv_Data_Ptr ) );
				if( DataSize < sizeof( Tx_Power_Level_Type ) )
				{
					Ptr = NULL;
				}
			}
		}while( Ptr != NULL );


		DataPtr = AdvertisingParameters->HostData.Scan_Data_Ptr;
		DataSize = AdvertisingParameters->HostData.ScanRsp_Data_Length;


		do
		{
			Ptr = Get_AD_Type_Ptr( TX_POWER_LEVEL_TYPE, DataPtr, DataSize );
			if( Ptr != NULL )
			{
				Ptr->Tx_Power_Level = TX_Power_Level;
				DataPtr = (uint8_t*)( (uint32_t)( Ptr ) + (Ptr->length) + 1 ); /* Points to the next structure */
				DataSize = AdvertisingParameters->HostData.ScanRsp_Data_Length - ( (uint32_t)( DataPtr ) - (uint32_t)( AdvertisingParameters->HostData.Scan_Data_Ptr ) );
				if( DataSize < sizeof( Tx_Power_Level_Type ) )
				{
					Ptr = NULL;
				}
			}
		}while( Ptr != NULL );
	}
}


/****************************************************************/
/* LE_Set_Data_Complete()   					 				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Data_Complete( CONTROLLER_ERROR_CODES Status )
{
	AdvConfig.Actual = ( Status == COMMAND_SUCCESS ) ? AdvConfig.Next : AdvConfig.Prev;
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
			if( ( AdvPar->Own_Address_Type == OWN_RANDOM_DEV_ADDR ) && ( AdvPar->Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE) )
			{
				return (TRUE);
			}else if( ( AdvPar->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) || ( AdvPar->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) )
			{
				IDENTITY_ADDRESS PeerId;
				/* Check if this peer device is in the resolving list */
				PeerId.Type = AdvPar->Peer_Address_Type;
				PeerId.Address = AdvPar->Peer_Address;
				RESOLVING_RECORD* RecordPtr = Get_Record_From_Peer_Identity( &PeerId );
				if( RecordPtr != NULL )
				{
					/* The local IRK must be valid since local identity would be used instead of
					 * Resolvable private address. */
					if( !Check_NULL_IRK( &RecordPtr->Peer.Local_IRK ) )
					{
						return (TRUE);
					}
				}
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
				IDENTITY_ADDRESS PeerId;
				/* Check if this peer device is in the resolving list */
				PeerId.Type = AdvPar->Peer_Address_Type;
				PeerId.Address = AdvPar->Peer_Address;
				RESOLVING_RECORD* RecordPtr = Get_Record_From_Peer_Identity( &PeerId );
				if( RecordPtr != NULL )
				{
					/* The local IRK must be valid since local identity would be used instead of
					 * Resolvable private address. */
					if( !Check_NULL_IRK( &RecordPtr->Peer.Local_IRK ) )
					{
						return (TRUE);
					}
				}
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
/* End of file	                                                */
/****************************************************************/
