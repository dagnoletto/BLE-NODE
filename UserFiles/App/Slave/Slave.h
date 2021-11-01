

#ifndef SLAVE_H_
#define SLAVE_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "hci.h"


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
typedef struct
{
	uint16_t Connection_Handle;
	LE_SUPPORTED_FEATURES SupFeatures;
	REMOTE_VERSION_INFORMATION Version;
}MASTER_INFO;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
void SlaveNode( void );
void Server( void );
void Reset_Server( void );


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/
MASTER_INFO MasterInfo;


#endif /* SLAVE_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
