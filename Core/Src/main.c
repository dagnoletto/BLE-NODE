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
#include "ble_states.h"
#include "ble_utils.h"
#include "gap.h"
#include "security_manager.h"
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
	static uint32_t Period = 500UL;
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
		Run_BLE();

		if( TimeBase_DelayMs( &Timer, Period, TRUE )  )
		{
			DEVICE_IDENTITY Id;

			Id.Peer_Identity_Address.Type = PEER_PUBLIC_DEV_ADDR;
			Id.Peer_Identity_Address.Address.Bytes[0] = 0;
			Id.Peer_Identity_Address.Address.Bytes[1] = 10;
			Id.Peer_Identity_Address.Address.Bytes[2] = 20;
			Id.Peer_Identity_Address.Address.Bytes[3] = 30;
			Id.Peer_Identity_Address.Address.Bytes[4] = 40;
			Id.Peer_Identity_Address.Address.Bytes[5] = 50;

			memset( &Id.Local_IRK.Bytes[0], 0, sizeof(Id.Local_IRK) );
			Id.Local_IRK.Bytes[0] = 0;
			Id.Local_IRK.Bytes[15] = 15;

			memset( &Id.Peer_IRK.Bytes[0], 0, sizeof(Id.Peer_IRK) );
			Id.Peer_IRK.Bytes[0] = 5;
			Id.Peer_IRK.Bytes[15] = 51;

			RESOLVING_RECORD Record;
			Record.Peer = Id;
			Record.Local_Identity_Address = Get_Identity_Address( PEER_PUBLIC_DEV_ADDR );

			Add_Record_To_Resolving_List( &Record );




			/* ADVERTISING */
			ADVERTISING_PARAMETERS Adv;

			Adv.Advertising_Interval_Min = 160;
			Adv.Advertising_Interval_Max = 320;
			Adv.Advertising_Type = ADV_IND;
			Adv.Own_Address_Type = OWN_RESOL_OR_PUBLIC_ADDR;
			Adv.Own_Random_Address_Type = STATIC_DEVICE_ADDRESS;
			Adv.Peer_Address_Type = PEER_PUBLIC_DEV_ADDR;
			memcpy( &Adv.Peer_Address, &Id.Peer_Identity_Address.Address, sizeof(Adv.Peer_Address) );
			Adv.Advertising_Channel_Map.Val = DEFAULT_LE_ADV_CH_MAP;
			Adv.Advertising_Filter_Policy = 0;
			Adv.connIntervalmin = NO_SPECIFIC_MINIMUM;
			Adv.connIntervalmax = NO_SPECIFIC_MAXIMUM;
			Adv.Privacy = FALSE;
			Adv.Role = PERIPHERAL;
			Adv.DiscoveryMode = GENERAL_DISCOVERABLE_MODE;

			Enter_Advertising_Mode( &Adv );

			HAL_GPIO_TogglePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin );



			/* SCANNING */
//			SCANNING_PARAMETERS Scan;
//
//			Scan.LE_Scan_Type = ACTIVE_SCANNING;
//			Scan.LE_Scan_Interval = 320;
//			Scan.LE_Scan_Window = 320;
//			Scan.Own_Address_Type = OWN_RANDOM_DEV_ADDR;
//			Scan.Own_Random_Address_Type = NON_RESOLVABLE_PRIVATE;
//			Scan.PeerId = Record.Peer.Peer_Identity_Address;
//			Scan.Scanning_Filter_Policy = 0;
//			Scan.Filter_Duplicates = 0;
//			Scan.Privacy = FALSE;
//			Scan.Role = OBSERVER;
//
//			Enter_Scanning_Mode( &Scan );




		}

//		if( Get_Bluenrg_IRQ_Pin() )
//		{
//			HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_SET );
//		}else
//		{
//			HAL_GPIO_WritePin( HEART_BEAT_GPIO_Port, HEART_BEAT_Pin, GPIO_PIN_RESET );
//		}
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
