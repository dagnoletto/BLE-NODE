

#ifndef TYPES_H_
#define TYPES_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
#ifndef FALSE
#define FALSE  0  /* Boolean value FALSE. FALSE is defined always as a zero value */
#endif

#ifndef TRUE
#define TRUE  1  /* Boolean value TRUE. TRUE is defined always as a non zero value */
#endif


#define EnterCritical() asm ( "CPSID i\n\t" ) /* Disable exceptions */
#define ExitCritical() asm ( "CPSIE i\n\t" )  /* Enable exceptions */


#endif /* TYPES_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
