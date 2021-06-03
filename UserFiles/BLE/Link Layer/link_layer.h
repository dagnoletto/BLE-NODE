

#ifndef LINK_LAYER_H_
#define LINK_LAYER_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"


/****************************************************************/
/* Type Defines (PDU structure)                		            */
/****************************************************************/
/* Those are the PDU types defined in the Bluetooth Core Specification */
/* The standard defines the types in v5.2 Volume 6: Part B - Link Layer Specification */


/*------------------------------ ADVERTISING PDUs ------------------------------------*/
typedef struct
{
	BD_ADDR_TYPE AdvA; /* Advertiser’s address: either public (TxAdd = 0) or random (TxAdd = 1) depending on TxAdd field of advertising PDU header */
	uint8_t AdvData[]; /* Advertising Data from the advertiser’s Host (0-31 octets). */
}__attribute__((packed)) ADV_IND_PDU; /* Legacy advertising PDU. Used in connectable and scannable undirected advertising events. */


typedef struct
{
	BD_ADDR_TYPE AdvA; /* Advertiser’s address: either public (TxAdd = 0) or random (TxAdd = 1) depending on TxAdd field of advertising PDU header */
	BD_ADDR_TYPE TargetA; /* Target’s address: either public (RxAdd = 0) or random (RxAdd = 1) depending on RxAdd field of advertising PDU header */
}__attribute__((packed)) ADV_DIRECT_IND_PDU; /* Legacy advertising PDU. Used in connectable directed advertising events. */


typedef ADV_IND_PDU ADV_NONCONN_IND_PDU; /* Legacy advertising PDU. Used in non-connectable and non-scannable undirected advertising events. */


typedef ADV_IND_PDU ADV_SCAN_IND_PDU; /* Legacy advertising PDU. Used in scannable undirected advertising events. */


/* TODO: The extended advertising is not finished and needs much more efforts. Maybe functions must be called
 * to handle the variable data fields. */


/*-------------------------------- SCANNING PDUs -------------------------------------*/
typedef struct
{
	BD_ADDR_TYPE ScanA; /* Scanner’s address: either public (TxAdd = 0) or random (TxAdd = 1) depending on TxAdd field of advertising PDU header */
	BD_ADDR_TYPE AdvA; /* Advertiser’s address: either public (RxAdd = 0) or random (RxAdd = 1) depending on RxAdd field of advertising PDU header */
}__attribute__((packed)) SCAN_REQ_PDU; /* Used to request scan. */


typedef SCAN_REQ_PDU AUX_SCAN_REQ_PDU; /* Used to request auxiliary scan. */


typedef struct
{
	BD_ADDR_TYPE AdvA; /* Advertiser’s address: either public (TxAdd = 0) or random (TxAdd = 1) depending on TxAdd field of advertising PDU header */
	uint8_t ScanRspData[]; /* The ScanRspData field may contain any data from the advertiser’s Host. (0-31 octets). */
}__attribute__((packed)) SCAN_RSP_PDU; /* Used to respond to a scan request. */


/* TODO: The AUX_SCAN_RSP is an extended version based on Common Extended Advertising Payload Format. Must be implemented. */


/*-------------------------------- INITIATING PDUs -------------------------------------*/
typedef struct
{
	BD_ADDR_TYPE InitA; /* Initiator’s address: either public (TxAdd = 0) or random (TxAdd = 1) depending on TxAdd field of advertising PDU header */
	BD_ADDR_TYPE AdvA;  /* Advertiser’s address: either public (RxAdd = 0) or random (RxAdd = 1) depending on RxAdd field of advertising PDU header */
	union
	{
		struct LLData_Fields
		{
			uint32_t 	AA;		    /* The AA field shall contain the ACL connection’s Access Address */
			uint8_t  	CRCInit[3]; /* The CRCInit field shall contain the initialization value for the CRC calculation for the ACL connection */
			uint8_t  	WinSize;	/* The WinSize field shall be set to indicate the transmitWindowSize value as transmitWindowSize = WinSize * 1.25 ms */
			uint16_t 	WinOffset;	/* The WinOffset field shall be set to indicate the transmitWindowOffset as transmitWindowOffset = WinOffset * 1.25 ms */
			uint16_t 	Interval;	/* The Interval field shall be set to indicate the connInterval as connInterval = Interval * 1.25 ms */
			uint16_t 	Latency;	/* The Latency field shall be set to indicate the connSlaveLatency as connSlaveLatency = Latency */
			uint16_t 	Timeout;	/* The Timeout field shall be set to indicate the connSupervisionTimeout as connSupervisionTimeout = Timeout * 10 ms */
			CHANNEL_MAP ChM;    	/* The ChM field shall contain the channel map indicating Used and Unused data channels. */
			uint8_t  	Hop :5;	    /* The Hop field shall be set to indicate the hopIncrement used in the data channel selection algorithm */
			uint8_t  	SCA :3;	    /* The SCA field shall be set to indicate the masterSCA used to determine the worst case Master’s sleep clock accuracy */
		}__attribute__((packed)) Fields;
		uint8_t Bytes[sizeof(struct LLData_Fields)];	/* LLData field (22 octets) */
	}LLData;
}__attribute__((packed)) CONNECT_IND_PDU; /* Sent by the Link Layer in the Initiating state. Received by the Link Layer in the Advertising state. */


typedef CONNECT_IND_PDU AUX_CONNECT_REQ_PDU; /* Sent by the Link Layer in the Initiating state. Received by the Link Layer in the Advertising state. */


/* TODO: The AUX_CONNECT_RSP is an extended version based on Common Extended Advertising Payload Format. Must be implemented. */


/*-------------------------------- LL DATA PDU -------------------------------------*/


/****************************************************************/
/* Type Defines (For driver interface)                          */
/****************************************************************/


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* LINK_LAYER_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
