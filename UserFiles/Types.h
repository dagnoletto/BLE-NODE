

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


/* The SR_reg is used in inline assembler. */
extern volatile uint8_t SR_reg;        /* Current FAULTMASK register */
/* The SR_reg is used in inline assembler. */
extern volatile uint8_t SR_lock;


/* Save status register and disable interrupts */
#define EnterCritical() \
 do {\
   if (++SR_lock == 1u) {\
  /*lint -save  -e586 -e950 Disable MISRA rule (2.1,1.1) checking. */\
     asm ( \
     "MRS R0, PRIMASK\n\t" \
     "CPSID i\n\t"            \
     "STRB R0, %[output]"  \
     : [output] "=m" (SR_reg)\
     :: "r0");\
  /*lint -restore Enable MISRA rule (2.1,1.1) checking. */\
   }\
 } while(0)


/* Restore status register  */
#define ExitCritical() \
 do {\
   if (--SR_lock == 0u) { \
  /*lint -save  -e586 -e950 Disable MISRA rule (2.1,1.1) checking. */\
     asm (                 \
       "ldrb r0, %[input]\n\t"\
       "msr PRIMASK,r0;\n\t" \
       ::[input] "m" (SR_reg)  \
       : "r0");                \
  /*lint -restore Enable MISRA rule (2.1,1.1) checking. */\
   }\
 } while(0)


#endif /* TYPES_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
