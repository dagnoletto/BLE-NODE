

/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"
#include "stm32f0xx_hal.h"


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


/****************************************************************/
/* Local variables definition                                   */
/****************************************************************/


/****************************************************************/
/* TimeBase_Init()                                              */
/* Purpose: Used to configure the Timer                  		*/
/* Parameters: none				         						*/
/* Return: none  												*/
/* Description:													*/
/****************************************************************/
void TimeFunctions_Init(void)
{
  SystemCoreClockUpdate();
}


/****************************************************************/
/* TimeBase_DelayMs()                                           */
/* Purpose: Delay in milliseconds don't stop other tasks 		*/
/* executions                            					    */
/*   - ui32Buffer: Pointer of the variable                      */
/*   - ui32Time  : Time in milliseconds                         */
/*   - ui32Reset : Resets the buffer                            */
/* Return: Operation result                                     */
/*   FALSE - Counting the delay                                 */
/*   TRUE - Finished delay                                      */
/* Description: This function initialize the timer Hardware and */
/* variables related to state machine           				*/
/****************************************************************/
uint32_t TimeBase_DelayMs( uint32_t* ui32Buffer, uint32_t ui32Time, uint32_t ui32Reset )
{
  uint32_t ui32Return = FALSE;

  if( *ui32Buffer == 0 ) /* Checks if the ui32Buffer is clean */
  {
    *ui32Buffer = uwTick; /* First iteration, so ui32Buffer is free. Saves the current value of the time counter */
  }
  else
  {
    /* If the ui32Buffer already was loaded with start time */
    if( uwTick >= *ui32Buffer ) /* Check if the time counter of the system is more than was before */
    {
      if( (uwTick - *ui32Buffer) >= ui32Time ) /* If it's larger, checks if passed the time */
      {
	ui32Return = TRUE;
      }
    }
    else
    {
      /* This condition foresees cases wherein the counter exceeded the 4 bytes */
      /* In this case, calculates the offset between the that bursted and that the already counted */
      if( ((0xFFFFFFFF - *ui32Buffer) + uwTick) >= ui32Time )
      {
	ui32Return = TRUE;
      }
    }
  }

  if( (ui32Return == TRUE) && (ui32Reset == TRUE) )
  {
    *ui32Buffer = 0; /* If yes, clears ui32Buffer and indicates that the delay has passed */
  }

  return ui32Return;
}


/****************************************************************/
/* TimeBase_DelayMsV2()                                         */
/****************************************************************/
/*                                                              */
/* Purpose: Similar as TimeBase_DelayMs but with the run time	*/
/* since the first call               							*/
/* Date  : 13/10/2020                                           */
/* Author: Daniel Agnoletto                                     */
/****************************************************************/
uint32_t TimeBase_DelayMsV2( uint32_t* ui32Buffer, uint32_t ui32Time, uint32_t* ui32RunTime, uint32_t ui32Reset)
{
	uint32_t ui32Return = FALSE;

	if( *ui32Buffer == 0 ) /* Checks if the ui32Buffer is clean */
	{
		*ui32Buffer = uwTick; /* First iteration, so ui32Buffer is free. Saves the current value of the time counter */
		*ui32RunTime = 0;
	}
	else
	{
		/* If the ui32Buffer already was loaded with start time */
		if( uwTick >= *ui32Buffer ) /* Check if the time counter of the system is more than was before */
		{
			*ui32RunTime = uwTick - *ui32Buffer;
			if( *ui32RunTime >= ui32Time ) /* If it's larger, checks if passed the time */
			{
				ui32Return = TRUE;
			}
		}
		else
		{
			/* This condition foresees cases wherein the counter exceeded the 4 bytes */
			/* In this case, calculates the offset between the that bursted and that the already counted */
			*ui32RunTime = (0xFFFFFFFF - *ui32Buffer) + uwTick;
			if( *ui32RunTime >= ui32Time )
			{
				ui32Return = TRUE;
			}
		}
	}

	if( (ui32Return == TRUE) && (ui32Reset == TRUE) )
	{
		*ui32Buffer = 0; /* If yes, clears ui32Buffer and indicates that the delay has passed */
	}

	return ui32Return;
}


/****************************************************************/
/* TimerON()                                                    */
/****************************************************************/
/*                                                              */
/* Purpose: Timer ON delay (delay to become true)               */
/* Date  : 01/06/2015                                           */
/* Author: Daniel Agnoletto                                     */
/****************************************************************/
uint8_t TimerON( uint32_t* Counter, uint32_t Time, uint8_t Input )
{
  uint8_t Output = FALSE;

  if( Input )
  {
    if( TimeBase_DelayMs( Counter, Time, FALSE ) )
    {
      Output = TRUE;
    }
  }
  else
  {
    *Counter = 0;
  }
  return (Output);
}


/****************************************************************/
/* TimerOFF()                                                   */
/****************************************************************/
/*                                                              */
/* Purpose: Timer OFF delay (delay to become false)             */
/* Date  : 14/06/2015                                           */
/* Author: Daniel Agnoletto                                     */
/****************************************************************/
uint8_t TimerOFF( uint32_t* Counter, uint32_t Time, uint8_t Input)
{
  uint8_t Output = TRUE;

  if( Input )
  {
	  *Counter = uwTick;
  }
  else
  {
	if(*Counter == 0)
	{
		Output = FALSE;
	}else
	{
		if( TimeBase_DelayMs( Counter, Time, FALSE ) )
		{
			Output = FALSE;
			*Counter = 0;
		}
	}
  }
  return (Output);
}


/****************************************************************/
/* Wait_DelayUs( uint32_t us )                                  */
/****************************************************************/
/*                                                              */
/* Purpose: Delay in microseconds                               */
/* Parameters:                                                  */
/*   - us: us to wait                                           */
/* Date  : 25/08/2018                                           */
/****************************************************************/
void Wait_DelayUs(uint32_t us)
{
	uint32_t Counter = us * (HAL_RCC_GetHCLKFreq()/1000000 /* us */);
	while(Counter)
	{
		Counter--;
	}
}


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
