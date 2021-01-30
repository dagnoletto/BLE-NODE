

#ifndef TIME_FUNCTIONS_H_
#define TIME_FUNCTIONS_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/


/****************************************************************/
/* Type Defines                                                 */
/****************************************************************/


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/
extern void TimeFunctions_Init(void);
extern uint32_t TimeBase_DelayMs( uint32_t* ui32Buffer, uint32_t ui32Time,
		uint32_t ui32Reset);
extern uint32_t TimeBase_DelayMsV2( uint32_t* ui32Buffer, uint32_t ui32Time,
		uint32_t* ui32RunTime, uint32_t ui32Reset);
extern uint8_t TimerON( uint32_t* Counter, uint32_t Time, uint8_t Input );
extern uint8_t TimerOFF( uint32_t* Counter, uint32_t Time, uint8_t Input );
extern void Wait_DelayUs(uint32_t us);


/****************************************************************/
/* Defines                                                      */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* TIME_FUNCTIONS_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
