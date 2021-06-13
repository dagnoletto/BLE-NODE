

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "hci.h"
#include "gap.h"


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
static char LOCAL_NAME[] = "User friendly name";


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/


/****************************************************************/
/* Load_Local_Name()        									*/
/* Location: 					 								*/
/* Purpose: Load the local name from the pointer and returns	*/
/* the offset from the pointer.									*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
uint8_t Load_Local_Name( Local_Name_Type* Ptr, uint16_t ArraySize )
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
/* End of file	                                                */
/****************************************************************/
