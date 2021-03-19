/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "crc.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Types.h"
#include "TimeFunctions.h"
#include "Bluenrg.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */
	static uint32_t Timer = 0;
	static uint32_t Period = 1000UL;
	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	HAL_RCC_DeInit(); /* DeInit RCC to put RCC to the reset state otherwise SystemClock_Config reports Error */
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_CRC_Init();
	MX_SPI1_Init();
	/* USER CODE BEGIN 2 */
	TimeFunctions_Init();
	Reset_Bluenrg();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		Run_Bluenrg();
		if( TimeBase_DelayMs( &Timer, Period, TRUE ) )
		{
			static uint8_t bdflag = FALSE;
			static uint8_t enable = FALSE;

			Period = 1000UL;

			//if( !bdflag )
			//{
			//  uint8_t address[6] = {1,2,3,4,5,6};
			// ACI_Hal_Write_Config_Data( 0, 6, &address[0] );
			//}
			//bdflag++;

			/* TODO: alguns comandos deixam o m�dulo "maluco": se enviar dois ACI_Hal_Write_Config_Data na sequ�ncia, pira o cabe��o, mas isso n�o
			 * acontece com 2 HCI_LE_Set_Advertising_Data...  */

			//uint8_t Vector[6] = {0,1,2,3,4,8};
			//HCI_LE_Set_Advertising_Data( sizeof(Vector), &Vector[0] );

			//HCI_Read_Local_Version_Information();
			//HCI_LE_Set_Advertising_Data( sizeof(Vector), &Vector[0] );

			//HCI_LE_Set_Advertising_Data( sizeof(Vector), &Vector[0] );

			//HCI_LE_Set_Advertising_Data( sizeof(Vector), &Vector[0] );

			//HCI_LE_Set_Advertising_Data( sizeof(Vector), &Vector[0] );
			//uint8_t address[6] = {0,0,0,0,0,0};
			//ACI_Hal_Write_Config_Data( 0, 6, &address[0] );

			//ACI_Hal_Write_Config_Data( 0, 6, &address[0] );


			/* D� pau quando estas duas mensagens s�o enviadas na sequ�ncia por algumas vezes */
			//uint8_t address[6] = {0,0,0,0,0,0};
			//ACI_Hal_Write_Config_Data( 0, 6, &address[0] );

			//uint8_t mode = 1;
			//ACI_Hal_Write_Config_Data( 0x06, 2, &mode );



			//HCI_LE_Set_Advertising_Data( sizeof(Vector), &Vector[0] );
			//HCI_Read_Local_Version_Information();
			// if(bdflag % 5 == 0)
			//{
			//ACI_Hal_Read_Config_Data( 0x00 );
			//}

			//  ACI_Hal_Set_Tx_Power_Level( 0, 0 );

			// ACI_Hal_Device_Standby();

			// ACI_Hal_LE_Tx_Test_Packet_Number();
			if( !enable )
			{
				//ACI_Hal_Tone_Stop();
			}
			//if( !bdflag )
			//{
			if( enable )
			{
				bdflag++;
				if( bdflag > 39 )
				{
					bdflag = 0;
				}
				//ACI_Hal_Tone_Start( bdflag );
			}

			enable = !enable;

			//ACI_Hal_Get_Link_Status(  );

			//	  bdflag = 1;
			//}

			//ACI_Hal_Get_Fw_Build_Number( );

			//ACI_Hal_Get_Link_Status( );

			//ACI_Hal_Get_Anchor_Period( );

			//HCI_Disconnect( 0, REM_DEV_TERM_CONN_LOW_RESOURCES );

			//HCI_Read_Local_Version_Information(  );
			//uint8_t address[] = {1,2,3,4,5,6};

			//HCI_LE_Set_Advertising_Data( sizeof(address), &address[0] );

			//HCI_Read_Remote_Version_Information( 0 );

//			EVENT_MASK Event_Mask;
//			SET_EVENT_MASK_DEFAULT(Event_Mask);
//
//			HCI_Set_Event_Mask( Event_Mask );

			//HCI_Reset(  );

			//HCI_Read_Transmit_Power_Level( 0, 0 );

			//HCI_Read_Local_Supported_Commands(  );

			//HCI_Read_Local_Supported_Features(  );

			//HCI_Read_BD_ADDR(  );

			//HCI_Read_RSSI( 0 );

//			LE_EVENT_MASK LE_Event_Mask;
//			SET_LE_EVENT_MASK_DEFAULT(LE_Event_Mask);
//			HCI_LE_Set_Event_Mask( LE_Event_Mask );

			HCI_LE_Read_Buffer_Size(  );

			HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );
		}
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
	RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
	{
		Error_Handler();
	}
	/** Enables the Clock Security System
	 */
	HAL_RCC_EnableCSS();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
