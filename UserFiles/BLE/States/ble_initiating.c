

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
	VERIFY_OWN_ADDRESS,
	GENERATE_RANDOM_ADDRESS,
	WAIT_FOR_NEW_LOCAL_READ,
	SET_RANDOM_ADDRESS,
	ENABLE_ADDRESS_RESOLUTION,
	SET_INIT_PARAMETERS,
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
static void HCI_LE_Create_Connection_Cancel_Complete( CONTROLLER_ERROR_CODES Status );
static void Read_Local_Resolvable_Address_Complete( CONTROLLER_ERROR_CODES Status, BD_ADDR_TYPE* Local_Resolvable_Address );
static void LE_Clear_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Add_Device_To_Resolving_List_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Random_Address_Complete( CONTROLLER_ERROR_CODES Status );
static void LE_Set_Address_Resolution_Enable_Complete( CONTROLLER_ERROR_CODES Status );
static uint8_t Check_Initiator_Address( INITIATING_PARAMETERS* InitPar );
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
	static RESOLVING_RECORD* RecordPtr;

	//TODO: Quando privacidade estiver habilitado, nunca deixar o InitA ser ajustado para identity address

	switch( InitConfig.Actual )
	{
	case CANCEL_INITIATING:
		RecordPtr = NULL;
		InitConfigTimeout = 0;
		InitiatingParameters->Counter = 0;
		InitConfig.Next = DISABLE_ADDRESS_RESOLUTION;
		InitConfig.Prev = CANCEL_INITIATING;
		InitConfig.Actual = HCI_LE_Create_Connection_Cancel( &HCI_LE_Create_Connection_Cancel_Complete, NULL ) ? WAIT_OPERATION : CANCEL_INITIATING;
		break;

	case DISABLE_ADDRESS_RESOLUTION:
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
			DEVICE_IDENTITY* DevId = &( Get_Record_From_Index( SM_Resolving_List_Index )->Peer );
			/* Here we add a device to the controller's resolving list if it exists in the Host's list */
			InitConfig.Actual = HCI_LE_Add_Device_To_Resolving_List( DevId->Peer_Identity_Address.Type, DevId->Peer_Identity_Address.Address,
					&DevId->Peer_IRK, &DevId->Local_IRK, &LE_Add_Device_To_Resolving_List_Complete, NULL ) ? WAIT_OPERATION : ADD_TO_RESOLVING_LIST;
		}else
		{
			InitConfig.Actual = Get_Number_Of_Resolving_Records() ? ENABLE_ADDRESS_RESOLUTION : VERIFY_OWN_ADDRESS;
		}
		break;

		//	case VERIFY_OWN_ADDRESS:
		/* Check if this peer device is in the resolving list */
		//RecordPtr = Get_Record_From_Peer_Identity( &InitiatingParameters->->PeerId );
		//		switch( ScanningParameters->Own_Address_Type )
		//		{
		//		case OWN_PUBLIC_DEV_ADDR:
		//		case OWN_RESOL_OR_PUBLIC_ADDR:
		//			ScanConfig.Actual = ENABLE_ADDRESS_RESOLUTION;
		//			break;
		//
		//			/* For random address options, we shall populate the random address before setting scanning parameters */
		//		case OWN_RANDOM_DEV_ADDR:
		//		case OWN_RESOL_OR_RANDOM_ADDR:
		//		default:
		//			ScanConfig.Actual = GENERATE_RANDOM_ADDRESS;
		//			break;
		//		}
		//		break;
		//
		//		case GENERATE_RANDOM_ADDRESS:
		//		{
		//			switch( ScanningParameters->Own_Random_Address_Type )
		//			{
		//			case NON_RESOLVABLE_PRIVATE:
		//			{
		//				BD_ADDR_TYPE* Ptr = Generate_Device_Address( Get_Supported_Commands(), NON_RESOLVABLE_PRIVATE, NULL, 7 );
		//				if ( Ptr != NULL )
		//				{
		//					RandomAddress = *Ptr;
		//					ScanConfig.Actual = SET_RANDOM_ADDRESS;
		//				}
		//			}
		//			break;
		//
		//			case RESOLVABLE_PRIVATE:
		//				if( RecordPtr != NULL )
		//				{
		//					ScanConfigTimeout = 0;
		//					/* We should use RPA generated by the controller. */
		//					ScanConfig.Actual = HCI_LE_Read_Local_Resolvable_Address( RecordPtr->Peer.Peer_Identity_Address.Type,
		//							RecordPtr->Peer.Peer_Identity_Address.Address, &Read_Local_Resolvable_Address_Complete, NULL ) ? WAIT_OPERATION : WAIT_FOR_NEW_LOCAL_READ;
		//				}else
		//				{
		//					ScanConfig.Actual = FAILED_SCAN_CONFIG;
		//				}
		//				break;
		//
		//			case STATIC_DEVICE_ADDRESS:
		//			default:
		//				RandomAddress = *( Get_Static_Random_Device_Address( ).Ptr );
		//				ScanConfig.Actual = SET_RANDOM_ADDRESS;
		//				break;
		//			}
		//		}
		//		break;
		//
		//		case WAIT_FOR_NEW_LOCAL_READ:
		//			if( TimeBase_DelayMs( &ScanConfigTimeout, 500, TRUE ) )
		//			{
		//				ScanConfig.Actual = GENERATE_RANDOM_ADDRESS;
		//			}
		//			break;
		//
		//		case SET_RANDOM_ADDRESS:
		//			ScanConfigTimeout = 0;
		//			ScanConfig.Next = ENABLE_ADDRESS_RESOLUTION;
		//			ScanConfig.Actual = HCI_LE_Set_Random_Address( RandomAddress, &LE_Set_Random_Address_Complete, NULL ) ? WAIT_OPERATION : SET_RANDOM_ADDRESS;
		//			break;
		//
		//		case ENABLE_ADDRESS_RESOLUTION:
		//			if( ScanningParameters->Privacy )
		//			{
		//				ScanConfig.Next = SET_SCAN_PARAMETERS;
		//				ScanConfig.Prev = ENABLE_ADDRESS_RESOLUTION;
		//				ScanConfig.Actual = HCI_LE_Set_Address_Resolution_Enable( TRUE, &LE_Set_Address_Resolution_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_ADDRESS_RESOLUTION;
		//			}else
		//			{
		//				ScanConfig.Actual = SET_SCAN_PARAMETERS;
		//			}
		//			break;
		//
		//		case SET_SCAN_PARAMETERS:
		//			ScanConfigTimeout = 0;
		//			ScanConfig.Next = ENABLE_SCANNING;
		//			ScanConfig.Actual = HCI_LE_Set_Scan_Parameters( ScanningParameters->LE_Scan_Type, ScanningParameters->LE_Scan_Interval, ScanningParameters->LE_Scan_Window,
		//					ScanningParameters->Own_Address_Type, ScanningParameters->Scanning_Filter_Policy, &LE_Set_Scan_Parameters_Complete, NULL ) ? WAIT_OPERATION : SET_SCAN_PARAMETERS;
		//			break;
		//
		//		case ENABLE_SCANNING:
		//			ScanConfigTimeout = 0;
		//			ScanConfig.Next = END_INIT_CONFIG;
		//			ScanConfig.Prev = ENABLE_SCANNING;
		//			ScanConfig.Actual = HCI_LE_Set_Scan_Enable( TRUE, ScanningParameters->Filter_Duplicates, &LE_Set_Scan_Enable_Complete, NULL ) ? WAIT_OPERATION : ENABLE_SCANNING;
		//			break;

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
/* HCI_LE_Create_Connection_Cancel_Complete()    				*/
/* Location: 					 								*/
/* Purpose: 													*/
/* Description:													*/
/****************************************************************/
static void HCI_LE_Create_Connection_Cancel_Complete( CONTROLLER_ERROR_CODES Status )
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
		InitConfig.Actual = VERIFY_OWN_ADDRESS;
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
/* Initiating()        	   										*/
/* Location: 					 								*/
/* Purpose: Initiating											*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void Initiating( void )
{
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
		/* The Own_Address_Type can only be resolvable if Peer_Address_Type loads
		 * the peer identity, otherwise we cannot find out the local IRK. */
		if( InitPar->Own_Address_Type >= OWN_RESOL_OR_PUBLIC_ADDR )
		{
			if( InitPar->Peer_Address_Type < PUBLIC_IDENTITY_ADDR )
			{
				return (FALSE);
			}
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
/* Check_Local_Resolvable_Private_Address()   					*/
/* Location: 													*/
/* Purpose: Check if local RPA can be generated.				*/
/* Parameters: none				         						*/
/* Return:														*/
/* Description:													*/
/****************************************************************/
static uint8_t Check_Local_Resolvable_Private_Address( IDENTITY_ADDRESS* Peer_Identity_Address )
{
	RESOLVING_RECORD* RecordPtr = Get_Record_From_Peer_Identity( Peer_Identity_Address );
	if( RecordPtr != NULL )
	{
		/* The local IRK must be valid since local identity would be used instead of
		 * Resolvable private address. */
		if( !Check_NULL_IRK( &RecordPtr->Peer.Local_IRK ) )
		{
			return (TRUE);
		}
	}
	return (FALSE);
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
