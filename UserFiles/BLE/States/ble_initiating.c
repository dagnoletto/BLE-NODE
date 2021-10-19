

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "ble_states.h"
#include "TimeFunctions.h"
#include "ble_utils.h"
#include "hosted_functions.h"
#include "ble_initiating.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef enum
{
	CANCEL_INITIATING,
	DISABLE_ADDRESS_RESOLUTION,
	CLEAR_RESOLVING_LIST,
	ADD_TO_RESOLVING_LIST,
	ENABLE_ADDRESS_RESOLUTION,
	VERIFY_OWN_ADDRESS,
	GET_PEER_IDENTITY,
	LOAD_RESOLVING_RECORD,
	RESOLVE_PEER_RPA,
	PEER_RPA_RESOLVED,
	GENERATE_RANDOM_ADDRESS,
	WAIT_FOR_NEW_LOCAL_READ,
	SET_RANDOM_ADDRESS,
	VERIFY_PEER_ADDRESS,
	WAIT_HOST_TO_FINISH,
	CREATE_CONNECTION,
	END_INIT_CONFIG,
	FAILED_INIT_CONFIG,
	WAIT_OPERATION,
}INIT_CONFIG_STEPS;


typedef struct
{
	INIT_CONFIG_STEPS Actual;
	INIT_CONFIG_STEPS Next;
	INIT_CONFIG_STEPS Prev;
}INIT_CONFIG;


/****************************************************************/
/* Local functions declaration                                  */
/****************************************************************/
int8_t Initiating_Config( void );
void Initiating( void );
static void Free_Initiating_Parameters( void );
static void LE_Create_Connection_Cancel_Complete( CONTROLLER_ERROR_CODES Status );
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address );
static void LE_Create_Connection_Status( CONTROLLER_ERROR_CODES Status );
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static uint8_t Check_Initiator_Address( INITIATING_PARAMETERS* InitPar );
static void Check_Private_Addr(uint8_t resolvstatus, CONTROLLER_ERROR_CODES status);
static INIT_CONFIG_STEPS Verify_Local_RPA( IDENTITY_ADDRESS* Peer_Identity_Address );
static INIT_CONFIG_STEPS No_Local_RPA_Is_Possible( void );
static uint8_t Check_Local_Resolvable_Private_Address( IDENTITY_ADDRESS* Peer_Identity_Address );


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
static INITIATING_PARAMETERS* InitiatingParameters = NULL;
static INIT_CONFIG InitConfig = { CANCEL_INITIATING, CANCEL_INITIATING, CANCEL_INITIATING };
static BD_ADDR_TYPE RandomAddress;
static uint16_t SM_Resolving_List_Index;
static RESOLVING_RECORD* RecordPtr;


/****************************************************************/
/* Enter_Initiating_Mode()        	 							*/
/* Location: 					 								*/
/* Purpose: Put the controller in initiating mode.				*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Enter_Initiating_Mode( INITIATING_PARAMETERS* InitPar )
{
	if( Get_BLE_State( ) == STANDBY_STATE )
	{
		if( Check_Initiating_Parameters( InitPar )  )
		{
			Free_Initiating_Parameters( );

			InitiatingParameters = malloc( sizeof(INITIATING_PARAMETERS) );

			if( InitiatingParameters != NULL )
			{
				*InitiatingParameters = *InitPar;
				InitiatingParameters->Original_Own_Address_Type = InitiatingParameters->Own_Address_Type;

				InitConfig.Actual = CANCEL_INITIATING;

				Set_BLE_State( CONFIG_INITIATING );
				return (TRUE);
			}
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Exit_Initiating_Mode()        	 							*/
/* Location: 					 								*/
/* Purpose: Put the controller in standby exiting from 			*/
/* initiating mode.		 										*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Exit_Initiating_Mode( BLE_STATES CurrentState )
{
	if( CurrentState == INITIATING_STATE )
	{
		Free_Initiating_Parameters( );

		InitConfig.Actual = CANCEL_INITIATING;

		return (TRUE);
	}else if( CurrentState == CONFIG_INITIATING )
	{
		//TODO: Here we have to test if we are in a safe step to disable the state. By the contrary, we may be
		//blocked in a function or step
		//Acredito que seja melhor forçar o InitConfig.Actual para um ponto da configuração em que a máquina de estados
		//cancele a configuração e depois disso ela mesmo chame a entrada para stand-by, retornando (-1)

		//		if( InitConfig.Actual != REQUEST_ADV_CANCEL )
		//		{
		//			InitConfig.Actual = REQUEST_ADV_CANCEL;
		//		}
		//TODO: Não seve entrar em stand-by por aqui porque a máquina de estados deve continuar em config. Ela entrará em
		//stand-by sozinha.

		if( InitConfig.Actual == FAILED_INIT_CONFIG )
		{
			InitConfig.Actual = CANCEL_INITIATING;
			Free_Initiating_Parameters( );
			return (TRUE);
		}
	}

	return (FALSE);
}


/****************************************************************/
/* Free_Initiating_Parameters()        	   						*/
/* Location: 					 								*/
/* Purpose: Free allocated initiating parameters memory			*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Free_Initiating_Parameters( void )
{
	if( InitiatingParameters != NULL )
	{
		free(InitiatingParameters);
		InitiatingParameters = NULL;
	}
}


/****************************************************************/
/* Initiating_Config()        	   								*/
/* Location: 					 								*/
/* Purpose: Configure initiating type.							*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
int8_t Initiating_Config( void )
{
	static uint32_t InitConfigTimeout = 0;
	static RESOLVING_RECORD* PeerRecordPtr;

	switch( InitConfig.Actual )
	{
	case CANCEL_INITIATING:
		RecordPtr = NULL;
		InitConfigTimeout = 0;
		InitiatingParameters->Own_Address_Type = InitiatingParameters->Original_Own_Address_Type;
		InitiatingParameters->Counter = 0;
		InitConfig.Next = DISABLE_ADDRESS_RESOLUTION;
		InitConfig.Prev = CANCEL_INITIATING;
		InitConfig.Actual = HCI_LE_Create_Connection_Cancel( &LE_Create_Connection_Cancel_Complete, NULL ) ? WAIT_OPERATION : CANCEL_INITIATING;
		break;

	case DISABLE_ADDRESS_RESOLUTION:
		InitConfigTimeout = 0;
		InitConfig.Next = CLEAR_RESOLVING_LIST;
		InitConfig.Prev = DISABLE_ADDRESS_RESOLUTION;
		InitConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( FALSE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : DISABLE_ADDRESS_RESOLUTION;
		break;

	case CLEAR_RESOLVING_LIST:
		if( ( InitiatingParameters->Privacy ) ||
				( InitiatingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) ||
				( InitiatingParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) ||
				( InitiatingParameters->Peer_Address_Type == PUBLIC_IDENTITY_ADDR ) ||
				( InitiatingParameters->Peer_Address_Type == RANDOM_IDENTITY_ADDR ) )
		{
			InitConfigTimeout = 0;
			SM_Resolving_List_Index = 0;
			InitConfig.Actual = HCI_LE_Clear_Resolving_List( &LE_Clear_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : CLEAR_RESOLVING_LIST;
		}else
		{
			InitConfig.Actual = VERIFY_OWN_ADDRESS;
		}
		break;

	case ADD_TO_RESOLVING_LIST:
		/* Check if we have bonded devices to add to the resolving list */
		if ( SM_Resolving_List_Index < Get_Number_Of_Resolving_Records() )
		{
			InitConfigTimeout = 0;
			DEVICE_IDENTITY* DevId = &( Get_Record_From_Index( SM_Resolving_List_Index )->Peer );
			/* Here we add a device to the controller's resolving list if it exists in the Host's list */
			InitConfig.Actual = HCI_LE_Add_Device_To_Resolving_List( DevId->Peer_Identity_Address.Type, DevId->Peer_Identity_Address.Address,
					&DevId->Peer_IRK, &DevId->Local_IRK, &LE_Add_Device_To_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : ADD_TO_RESOLVING_LIST;
		}else
		{
			InitConfig.Actual = Get_Number_Of_Resolving_Records() ? ENABLE_ADDRESS_RESOLUTION : VERIFY_OWN_ADDRESS;
		}
		break;

	case ENABLE_ADDRESS_RESOLUTION:
		InitConfigTimeout = 0;
		InitConfig.Next = VERIFY_OWN_ADDRESS;
		InitConfig.Prev = ENABLE_ADDRESS_RESOLUTION;
		InitConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( TRUE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADDRESS_RESOLUTION;
		break;

	case VERIFY_OWN_ADDRESS:
	{
		switch( InitiatingParameters->Own_Address_Type )
		{
		case OWN_PUBLIC_DEV_ADDR:
			InitConfig.Actual = VERIFY_PEER_ADDRESS;
			break;

		case OWN_RANDOM_DEV_ADDR:
			InitConfig.Actual = GENERATE_RANDOM_ADDRESS;
			break;

		case OWN_RESOL_OR_PUBLIC_ADDR:
		case OWN_RESOL_OR_RANDOM_ADDR:
			if( Get_Local_Version_Information()->HCI_Version <= CORE_SPEC_4_1 )
			{
				if( InitiatingParameters->Peer_Address_Type >= PUBLIC_IDENTITY_ADDR )
				{
					/* For lower than 4.1 version, address resolution at the controller is not available */
					InitConfig.Actual = FAILED_INIT_CONFIG;
				}else
				{
					InitConfig.Actual = GET_PEER_IDENTITY;
				}
			}else
			{
				/* For higher versions, the controller generates the RPA itself */
				InitConfig.Actual = ( InitiatingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR ) ? VERIFY_PEER_ADDRESS : GENERATE_RANDOM_ADDRESS;
			}
			break;

		default: break;
		}
	}
	break;

	case GET_PEER_IDENTITY:
	{
		InitConfigTimeout = 0;
		IDENTITY_ADDRESS Peer_Id_Addr;

		if( InitiatingParameters->Peer_Address_Type == PUBLIC_DEV_ADDR )
		{
			/* We assume we have a public identity address same as peer public device address loaded
			 * into the resolving list. If found, we can generate InitA as RPA. If not, InitA will be
			 * public or random address */
			Peer_Id_Addr.Type = PEER_PUBLIC_DEV_ADDR;
			Peer_Id_Addr.Address = InitiatingParameters->Peer_Address;

			InitConfig.Actual = Verify_Local_RPA( &Peer_Id_Addr );
		}else if( InitiatingParameters->Peer_Address_Type == RANDOM_DEV_ADDR )
		{
			RESOLVING_RECORD* Record;
			Peer_Id_Addr.Address = InitiatingParameters->Peer_Address;

			for( PEER_ADDR_TYPE i = PEER_PUBLIC_DEV_ADDR; i <= PEER_RANDOM_DEV_ADDR; i++ )
			{
				Peer_Id_Addr.Type = i;
				Record = Get_Record_From_Peer_Identity( &Peer_Id_Addr );
				if( Record != NULL )
				{
					break;
				}
			}

			if( Record != NULL )
			{
				/* If the peer address is equal to the identity address, we don't need to resolve anything. Just check if local RPA
				 * is OK to be generated. */
				InitConfig.Actual = Verify_Local_RPA( &Peer_Id_Addr );
			}else if( ( InitiatingParameters->Peer_Address.Bytes[sizeof(BD_ADDR_TYPE) - 1] & 0xC0 ) == 0x40 )
			{
				/* This is a resolvable private address, we should try to resolve it */
				/* We are going to try resolving for every record the random address provided for the peer device */
				SM_Resolving_List_Index = 0;
				InitConfig.Actual = LOAD_RESOLVING_RECORD;
			}else
			{
				/* It cannot be resolved. Most likely it is random not resolvable
				 * or it is random static identity out of resolving list */
				InitConfig.Actual = No_Local_RPA_Is_Possible( );
			}
		}else
		{
			InitConfig.Actual = FAILED_INIT_CONFIG;
		}
	}
	break;

	case LOAD_RESOLVING_RECORD:
		PeerRecordPtr = Get_Record_From_Index( SM_Resolving_List_Index );
		if( PeerRecordPtr != NULL )
		{
			/* This is a resolvable private address, we should try to resolve it */
			if( !Check_NULL_IRK(&PeerRecordPtr->Peer.Peer_IRK) )
			{
				/* The peer IRK for this record is not null, we can proceed with resolution */
				InitConfig.Actual = RESOLVE_PEER_RPA;
				if( Resolve_Private_Address( Get_Supported_Commands(), &InitiatingParameters->Peer_Address, &PeerRecordPtr->Peer.Peer_IRK, 2, &Check_Private_Addr ) )
				{
					InitConfigTimeout = 0;
				}else
				{
					Cancel_Private_Address_Resolution();
					/* End resolution for this address */
					InitConfig.Actual = GET_PEER_IDENTITY;
				}
			}else
			{
				/* Go to the next record */
				SM_Resolving_List_Index++;
			}
		}else
		{
			/* The list ended and no resolving record matched with the peer device address. */
			InitConfig.Actual = No_Local_RPA_Is_Possible( );
		}
		break;

	case RESOLVE_PEER_RPA:
		if( TimeBase_DelayMs( &InitConfigTimeout, 500, TRUE ) )
		{
			Cancel_Private_Address_Resolution();
			/* End resolution for this address */
			InitConfig.Actual = GET_PEER_IDENTITY;
		}
		break;

	case PEER_RPA_RESOLVED:
		InitConfig.Actual = Verify_Local_RPA( &PeerRecordPtr->Peer.Peer_Identity_Address );
		break;

	case GENERATE_RANDOM_ADDRESS:
	{
		switch( InitiatingParameters->Own_Random_Address_Type )
		{
		case NON_RESOLVABLE_PRIVATE:
		{
			BD_ADDR_TYPE* Ptr = Generate_Device_Address( Get_Supported_Commands(), NON_RESOLVABLE_PRIVATE, NULL, 8 );
			if ( Ptr != NULL )
			{
				RandomAddress = *Ptr;
				InitConfig.Actual = SET_RANDOM_ADDRESS;
			}
		}
		break;

		case STATIC_DEVICE_ADDRESS:
			RandomAddress = *( Get_Static_Random_Device_Address( ).AddrPtr );
			InitConfig.Actual = SET_RANDOM_ADDRESS;
			break;

		default:
			InitConfig.Actual = FAILED_INIT_CONFIG;
			break;
		}
	}
	break;

	case WAIT_FOR_NEW_LOCAL_READ:
		InitiatingParameters->Own_Address_Type = InitiatingParameters->Original_Own_Address_Type;
		if( TimeBase_DelayMs( &InitConfigTimeout, 500, TRUE ) )
		{
			InitConfig.Actual = VERIFY_OWN_ADDRESS;
		}
		break;

	case SET_RANDOM_ADDRESS:
		InitConfigTimeout = 0;
		InitConfig.Next = VERIFY_PEER_ADDRESS;
		InitConfig.Actual = HCI_LE_Set_Random_Address( RandomAddress, &LE_Set_Random_Address_Complete, NULL ) ? WAIT_OPERATION : SET_RANDOM_ADDRESS;
		break;

	case VERIFY_PEER_ADDRESS:
		switch( InitiatingParameters->Peer_Address_Type )
		{
		case PUBLIC_DEV_ADDR:
		case RANDOM_DEV_ADDR:
			InitConfig.Actual = WAIT_HOST_TO_FINISH;
			break;

		case PUBLIC_IDENTITY_ADDR:
		case RANDOM_IDENTITY_ADDR:
			if( Get_Local_Version_Information()->HCI_Version <= CORE_SPEC_4_1 )
			{
				/* Only address resolution in the controller can handle this */
				InitConfig.Actual = FAILED_INIT_CONFIG;
			}else
			{
				InitConfig.Actual = WAIT_HOST_TO_FINISH;
			}
			break;
		}
		break;

		case WAIT_HOST_TO_FINISH:
			if( TimeBase_DelayMs( &InitConfigTimeout, 500, TRUE ) )
			{
				InitConfig.Actual = FAILED_INIT_CONFIG;
			}else if( !Get_Hosted_Function().Val )
			{
				InitConfig.Actual = CREATE_CONNECTION;
			}
			break;

		case CREATE_CONNECTION:
			InitConfigTimeout = 0;
			InitConfig.Next = END_INIT_CONFIG;
			InitConfig.Prev = CREATE_CONNECTION;
			InitConfig.Actual = HCI_LE_Create_Connection( InitiatingParameters->LE_Scan_Interval, InitiatingParameters->LE_Scan_Window, InitiatingParameters->Initiator_Filter_Policy,
					InitiatingParameters->Peer_Address_Type, InitiatingParameters->Peer_Address, InitiatingParameters->Own_Address_Type,
					InitiatingParameters->Connection_Interval_Min, InitiatingParameters->Connection_Interval_Max, InitiatingParameters->Connection_Latency,
					InitiatingParameters->Supervision_Timeout, InitiatingParameters->Min_CE_Length, InitiatingParameters->Max_CE_Length, &LE_Create_Connection_Status ) ? WAIT_OPERATION : CREATE_CONNECTION;
			break;

		case END_INIT_CONFIG:
			InitConfig.Actual = CANCEL_INITIATING;
			return (TRUE);
			break;

		case FAILED_INIT_CONFIG:
			Free_Initiating_Parameters( );
			return (-1); /* Failed condition */
			break;

		case WAIT_OPERATION:
			if( TimeBase_DelayMs( &InitConfigTimeout, 500, TRUE ) )
			{
				InitConfig.Actual = CANCEL_INITIATING;
				InitConfig.Next = CANCEL_INITIATING;
			}
			break;

		default:
			break;
	}

	return (FALSE);
}


/****************************************************************/
/* Get_Initiator_Address()      								*/
/* Location: 					 								*/
/* Purpose: Verify the initiating own address.					*/
/* parameters.													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Get_Initiator_Address( LOCAL_ADDRESS_TYPE* Type, BD_ADDR_TYPE* InitA )
{
	uint8_t ReturnStatus = FALSE;

	if( ( InitiatingParameters != NULL ) && ( Get_BLE_State() == INITIATING_STATE ) &&
			( Get_Local_Version_Information()->HCI_Version <= CORE_SPEC_4_1 ) )
	{

		switch( InitiatingParameters->Original_Own_Address_Type )
		{
		case OWN_PUBLIC_DEV_ADDR:
			*InitA = *( Get_Public_Device_Address().AddrPtr );
			*Type = LOCAL_PUBLIC_DEV_ADDR;
			ReturnStatus = TRUE;
			break;

		case OWN_RANDOM_DEV_ADDR:
			*InitA = RandomAddress;
			*Type = LOCAL_RANDOM_DEV_ADDR;
			ReturnStatus = TRUE;
			break;

		case OWN_RESOL_OR_PUBLIC_ADDR:
		case OWN_RESOL_OR_RANDOM_ADDR:
			if( InitiatingParameters->Own_Address_Type == OWN_PUBLIC_DEV_ADDR )
			{
				/* Resolving record not found, so use the public address */
				*InitA = *( Get_Public_Device_Address().AddrPtr );
				*Type = LOCAL_PUBLIC_DEV_ADDR;
				ReturnStatus = TRUE;
			}else if( InitiatingParameters->Own_Address_Type == OWN_RANDOM_DEV_ADDR )
			{
				/* Is there a valid resolving record? */
				if( RecordPtr != NULL )
				{
					if( !Check_NULL_IRK( &RecordPtr->Peer.Local_IRK ) )
					{
						*InitA = RandomAddress;
						*Type = ( RecordPtr->Local_Identity_Address.Type == PEER_PUBLIC_DEV_ADDR ) ? LOCAL_RPA_PUBLIC_IDENTITY : LOCAL_RPA_RANDOM_IDENTITY;
						ReturnStatus = TRUE;
					}else if( InitiatingParameters->Privacy == FALSE )
					{
						if( RecordPtr->Local_Identity_Address.Type == PEER_PUBLIC_DEV_ADDR )
						{
							*InitA = RandomAddress;
							*Type = LOCAL_PUBLIC_IDENTITY_ADDR;
							ReturnStatus = TRUE;
						}else if( RecordPtr->Local_Identity_Address.Type == PEER_RANDOM_DEV_ADDR )
						{
							*InitA = RandomAddress;
							*Type = LOCAL_RANDOM_IDENTITY_ADDR;
							ReturnStatus = TRUE;
						}
					}else if( InitiatingParameters->Original_Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR )
					{
						*InitA = RandomAddress;
						*Type = LOCAL_RANDOM_DEV_ADDR;
						ReturnStatus = TRUE;
					}
				}else if( InitiatingParameters->Original_Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR )
				{
					*InitA = RandomAddress;
					*Type = LOCAL_RANDOM_DEV_ADDR;
					ReturnStatus = TRUE;
				}
			}
			break;
		}

	}

	return ( ReturnStatus );
}


/****************************************************************/
/* LE_Create_Connection_Cancel_Complete()    					*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Create_Connection_Cancel_Complete( CONTROLLER_ERROR_CODES Status )
{
	InitConfig.Actual = ( ( Status == COMMAND_SUCCESS ) || ( Status == COMMAND_DISALLOWED ) ) ? InitConfig.Next : InitConfig.Prev;
}


/****************************************************************/
/* LE_Set_Address_Resolution_Enable_Complete()    				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status )
{
	InitConfig.Actual = ( Status == COMMAND_SUCCESS || Status == COMMAND_DISALLOWED ) ? InitConfig.Next : InitConfig.Prev;
}


/****************************************************************/
/* LE_Clear_Resolving_List_Complete()      						*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status )
{
	InitConfig.Actual = ADD_TO_RESOLVING_LIST;
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
		InitConfig.Actual = ADD_TO_RESOLVING_LIST;
		SM_Resolving_List_Index++;
	}else if( Status == MEM_CAPACITY_EXCEEDED )
	{
		InitConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
	}else
	{
		InitConfig.Actual = ADD_TO_RESOLVING_LIST;
	}
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
	InitConfig.Actual = ( Status == COMMAND_SUCCESS ) ? InitConfig.Next : SET_RANDOM_ADDRESS;
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
		InitConfig.Actual = SET_RANDOM_ADDRESS;
		RandomAddress = *Local_Resolvable_Address;
	}else
	{
		InitConfig.Actual = WAIT_FOR_NEW_LOCAL_READ;
	}
}


/****************************************************************/
/* LE_Create_Connection_Status()        	   					*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void LE_Create_Connection_Status( CONTROLLER_ERROR_CODES Status )
{
	InitConfig.Actual = ( Status == COMMAND_SUCCESS ) ? InitConfig.Next : InitConfig.Prev;
}


/****************************************************************/
/* Initiating()        	   										*/
/* Location: 					 								*/
/* Purpose: Initiating											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Initiating( void )
{
	//TODO: verificar a privacidade para initiating, se existe necessidade de trocar o endereço de tempos em tempos
	//TODO: verificar se alguma coisa deve ser feita aqui
	//	/* Privacy feature in a Central with Host-based privacy: The Host shall generate a resolvable
	//	 * private address using the ‘resolvable private address generation procedure’ as defined in
	//	 * Section 10.8.2.2 or non-resolvable private address procedure as defined in Section 10.8.2.1.
	//	 * The Host shall set a timer equal to TGAP(private_addr_int). The Host shall generate a
	//	 * new resolvable private address or non-resolvable private address when the
	//	 * timer TGAP(private_addr_int) expires. Note: TGAP(private_addr_int) timer need not be run
	//	 * if a Central is not scanning or connected. (Page 1389 Core_v5.2).
	//	 * During active scanning, a privacy enabled Central shall use a non-resolvable
	//	 * or resolvable private address. */
	//
	//	/* Privacy feature in an Observer: During active scanning, a privacy enabled Observer shall use
	//	 * either a resolvable private address or non-resolvable private address.
	//	 * If Address Resolution is not supported or disabled in the Controller, the
	//	 * following applies to the Host: The Host shall generate a resolvable private
	//	 * address using the ‘resolvable private address generation procedure’ as defined
	//	 * in Section 10.8.2.2 or non-resolvable private address procedure as defined in
	//	 * Section 10.8.2.1. The Host shall set a timer equal to TGAP(private_addr_int).
	//	 * The Host shall generate a new resolvable private address or non-resolvable
	//	 * private address when the timer TGAP(private_addr_int) expires. The value of
	//	 * TGAP(private_addr_int) shall not be greater than 1 hour. Note: TGAP(private_addr_int)
	//	 * timer need not be run if an Observer is not scanning. (Page 1389 Core_v5.2). */
	//
	//	/* If the Controller has address resolution and that is enabled, the RPA is
	//	 * automatically generated given the timeout for the controller. However, in versions
	//	 * lower that 4.1, this functionality does not exist so we need to generate again.
	//	 * For non-resolvable private address we shall generate since the controller will not do it. */
	//
	//	/* TODO: Here we consider the address resolution is always enabled in scanning.
	//	 * Even for versions higher than 4.1, this function should be called if address resolution
	//	 * is disabled. We lay on the fact address resolution is always enabled in the controller
	//	 * for versions above 4.1. */

	//	if( ScanningParameters->Privacy && ( ScanningParameters->LE_Scan_Type == ACTIVE_SCANNING ) &&
	//			( ( ScanningParameters->Own_Address_Type == OWN_RANDOM_DEV_ADDR || ScanningParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR ) &&
	//					( ScanningParameters->Own_Random_Address_Type == NON_RESOLVABLE_PRIVATE || ScanningParameters->Own_Random_Address_Type == RESOLVABLE_PRIVATE ) ) )
	//	{
	//		if( TimeBase_DelayMs( &ScanningParameters->Counter, TGAP_PRIVATE_ADDR_INT, TRUE ) )
	//		{
	//			Set_BLE_State( CONFIG_SCANNING );
	//		}
	//	}
}


/****************************************************************/
/* Check_Initiating_Parameters()      							*/
/* Location: 													*/
/* Purpose: Verify initiating parameters.				 		*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
uint8_t Check_Initiating_Parameters( INITIATING_PARAMETERS* InitPar )
{
	if( ( InitPar->LE_Scan_Window > InitPar->LE_Scan_Interval ) ||
			( InitPar->Initiator_Filter_Policy > 1 ) ||
			( InitPar->Connection_Interval_Min > InitPar->Connection_Interval_Max ) ||
			( InitPar->Min_CE_Length > InitPar->Max_CE_Length ) )
	{
		/* Inconsistent configuration */
		return (FALSE);
	}else if( Get_Supported_Commands()->Bits.HCI_LE_Set_Privacy_Mode /* &&  HCI_LE_Set_Privacy_Mode supported by the host */)
	{
		// TODO: Aqui admite que o host suporta HCI_LE_Set_Privacy_Mode, mas tem que implementar este comando no host ainda.
		/* Me parece que nas versões em que HCI_LE_Set_Privacy_Mode está habilitado, qualquer endereço randômico recebido
		 * pelo controlador, seja ele proveniente do host ou de um dispositivo remoto, será resolvido (se for RPA) e automaticamente
		 * o controlador irá manter a privacidade para o endereço identidade correspondente. POr isso não há necessidade de
		 * usar 0x02 ou 0x03 neste caso, pois o controlador irá resolver o endereço randômico passado através de Peer_Address_Type == 0x01
		 * e já saberá se o RPA em questão corresponde a uma identidade pública ou estática. */
		if( InitPar->Peer_Address_Type >= PUBLIC_IDENTITY_ADDR )
		{
			/* Those values shall only be used by the Host if either the Host or the Controller does not
			 * support the HCI_LE_Set_Privacy_Mode command. */
			/* The Host shall not set Peer_Address_Type to either 0x02 or 0x03 if both the Host and the Controller
			support the HCI_LE_Set_Privacy_Mode command. If a Controller that supports the HCI_LE_Set_Privacy_Mode
			command receives the HCI_LE_Create_Connection command with Peer_Address_Type set to either
			0x02 or 0x03, it may use either device privacy mode or network privacy mode for that peer device. */
			return (FALSE);
		}
	}else if( Get_Local_Version_Information()->HCI_Version <= CORE_SPEC_4_1 )
	{
		/* The Peer_Address_Type can only be identity type if address resolution
		 * is enabled in the controller, since incoming advertising packet should be
		 * resolved prior to be accepted by the initiator */
		if( InitPar->Peer_Address_Type >= PUBLIC_IDENTITY_ADDR )
		{
			return (FALSE);
		}
	}

	return ( Check_Initiator_Address(InitPar) );
}


/****************************************************************/
/* Check_Initiator_Address()      								*/
/* Location: 													*/
/* Purpose: Verify initiator address.							*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Initiator_Address( INITIATING_PARAMETERS* InitPar )
{
	/* The Link Layer shall use resolvable private addresses for the initiator’s device
	address (InitA field) when initiating connection establishment with an
	associated device that exists in the Resolving List. The initiator’s device
	address (InitA field) in the initiating PDU is generated using the Resolving List
	Local IRK and the Resolvable Private Address Generation Procedure (see
	Section 1.3.2.2). The Link Layer should not set the InitA field to the same value
	as the TargetA field in the received advertising PDU.
	The Link Layer shall use the Host-provided address for the initiator’s device
	address (InitA field) when initiating connection establishment with a device that
	is not in the Resolving List. */

	switch( InitPar->Own_Address_Type )
	{
	case OWN_PUBLIC_DEV_ADDR:
		/* During initiating, a privacy enabled Initiator shall use a non-resolvable or resolvable private address. */
		return ( InitPar->Privacy ? FALSE : TRUE );
		break;

	case OWN_RANDOM_DEV_ADDR:
	case OWN_RESOL_OR_RANDOM_ADDR:
	{
		/* If we are initiating with privacy, the privacy depends pretty much of how
		 * random part is configured since if no resolving record is found, privacy must lay on random part. */
		switch( InitPar->Own_Random_Address_Type )
		{
		case STATIC_DEVICE_ADDRESS:
			/* During initiating, a privacy enabled Initiator shall use a non-resolvable or resolvable private address. */
			return ( InitPar->Privacy ? FALSE : TRUE );
			break;

		case NON_RESOLVABLE_PRIVATE:
			/* Permitted in privacy mode */
			return (TRUE);
			break;

		default:
			break;
		}
	}
	break;

	case OWN_RESOL_OR_PUBLIC_ADDR:
		/* Due to the fact the public address will be used for initiating if no resolving record is found, no privacy is
		 * guaranteed under this condition. */
		return ( InitPar->Privacy ? FALSE : TRUE );
		break;
	}

	return (FALSE);
}


/****************************************************************/
/* Check_Private_Addr()      									*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
static void Check_Private_Addr(uint8_t resolvstatus, CONTROLLER_ERROR_CODES status)
{
	if( Get_BLE_State() == CONFIG_INITIATING )
	{
		if( InitConfig.Actual == RESOLVE_PEER_RPA )
		{
			SM_Resolving_List_Index++;
			if( status == COMMAND_SUCCESS )
			{
				InitConfig.Actual = resolvstatus ? PEER_RPA_RESOLVED : LOAD_RESOLVING_RECORD;
			}else
			{
				InitConfig.Actual = LOAD_RESOLVING_RECORD; /* Could not be resolved due to controller failure */
			}
		}
	}
}


/****************************************************************/
/* Verify_Local_RPA()   										*/
/* Location: 													*/
/* Purpose: Check if local RPA can be generated.				*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static INIT_CONFIG_STEPS Verify_Local_RPA( IDENTITY_ADDRESS* Peer_Identity_Address )
{
	INIT_CONFIG_STEPS ActualStep;

	if( Check_Local_Resolvable_Private_Address( Peer_Identity_Address ) )
	{
		/* We assume random since for lower than 4.1 controller, this the only possible option */
		InitiatingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
		ActualStep = HCI_LE_Read_Local_Resolvable_Address( Peer_Identity_Address->Type, Peer_Identity_Address->Address,
				&Read_Local_Resolvable_Address_Complete, NULL ) ? WAIT_OPERATION : WAIT_FOR_NEW_LOCAL_READ;
	}else
	{
		/* No RPA can be employed */
		ActualStep = No_Local_RPA_Is_Possible( );
	}

	return ( ActualStep );
}


/****************************************************************/
/* No_Local_RPA_Is_Possible()   								*/
/* Location: 													*/
/* Purpose: Called when no local RPA can be generated.			*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static INIT_CONFIG_STEPS No_Local_RPA_Is_Possible( void )
{
	INIT_CONFIG_STEPS ActualStep = FAILED_INIT_CONFIG;

	/* No RPA can be employed */
	if ( InitiatingParameters->Own_Address_Type == OWN_RESOL_OR_PUBLIC_ADDR )
	{
		InitiatingParameters->Own_Address_Type = OWN_PUBLIC_DEV_ADDR;
		ActualStep = VERIFY_PEER_ADDRESS;
	}else if( InitiatingParameters->Own_Address_Type == OWN_RESOL_OR_RANDOM_ADDR )
	{
		InitiatingParameters->Own_Address_Type = OWN_RANDOM_DEV_ADDR;
		ActualStep = GENERATE_RANDOM_ADDRESS;
	}

	return ( ActualStep );
}


/****************************************************************/
/* Check_Local_Resolvable_Private_Address()   					*/
/* Location: 													*/
/* Purpose: Check if local RPA can be generated.				*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Local_Resolvable_Private_Address( IDENTITY_ADDRESS* Peer_Identity_Address )
{
	RecordPtr = Get_Record_From_Peer_Identity( Peer_Identity_Address );
	if( RecordPtr != NULL )
	{
		/* The local IRK must be valid since local identity would be used instead of
		 * Resolvable private address. If the Privacy is FALSE, the initiator is allowed to use
		 * zero filled local IRK */
		if( ( InitiatingParameters->Privacy == FALSE ) || ( !Check_NULL_IRK( &RecordPtr->Peer.Local_IRK ) ) )
		{
			return (TRUE);
		}
	}
	return (FALSE);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
