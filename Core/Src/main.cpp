/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "LiquidCrystal.h"
#include "LiquidCrystalUserInterface.h"
#include "records.h"
#include "measurment.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
static void MX_GPIO_Init(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */

static uint16_t voltsRec[ELEMENTS_SIZE];
const uint16_t minMV = 10800;

typedef struct {
	uint8_t day;
	uint8_t hour;
	uint8_t min;
	int16_t speedChange;
}time;

float calcChangeSpeed(float a, float b, uint16_t steps)
{
	return (a - b) / steps;
}

time calcTimeRemain(record *voltsHistory)
{
	time res;
	res.day = 0;
	res.hour = 0;
	res.min = 0;
	res.speedChange = 0;
	uint16_t sizeOfWritesRec = voltsHistory->getSizeWrites();

	if (sizeOfWritesRec == 0) {
		return res;
	}
	uint16_t mVoltBefor;
	uint16_t mVoltNow = voltsHistory->getValByIndex(sizeOfWritesRec-1);
	uint16_t mVoltDiff = 0;

	uint16_t remMV = mVoltNow - minMV;
	uint32_t minutesRem = 0;

	if (sizeOfWritesRec>= 15) {
		for (uint16_t i = 15; i < sizeOfWritesRec; i++) {
			mVoltBefor = voltsHistory->getValByIndex(sizeOfWritesRec - 1 - i);
			if (mVoltBefor > mVoltNow
		        && mVoltBefor - mVoltNow >= 4) {
				mVoltDiff = mVoltBefor - mVoltNow;
				minutesRem = remMV / mVoltDiff * i;
				break;
			}
		}
	}
	if (mVoltDiff == 0) {
		minutesRem = remMV / 4 * sizeOfWritesRec;
	}
	res.day = minutesRem / 60 / 24;
	res.hour = minutesRem / 60 - (minutesRem / 60 / 24) * 24;
	res.min = minutesRem - (res.day * 60 * 24) -  (res.hour * 60);
	return res;
}

uint16_t voltToPercentBattery(uint32_t val)
{
	const uint32_t mVolts100percent = 14500000;
	const uint32_t mVolts0percent = 10800000;
	const uint32_t mVoltsPerPercent = (mVolts100percent - mVolts0percent) / 1000;
	return (val*1000 - mVolts0percent) / mVoltsPerPercent;
}




int main(void)
{
  /* USER CODE BEGIN 1 */
	uint32_t acurMVolts = 0;
	//uint32_t approxMVolts = 0; //ToDo
	//uint32_t current = 0; //ToDo
	uint64_t acurMVoltsFiltr = 0;
	uint16_t acurFiltrCnt = 0;
	uint16_t percentOfPower = 0;
	record voltHistory(voltsRec, ELEMENTS_SIZE);
	time remTime;
	remTime.day = 0;
	remTime.hour = 0;
	remTime.min = 0;
    char strBuff[17];
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  /* SysTick_IRQn interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, 3);

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
  GpioInterface pRS(GPIOA, LL_GPIO_PIN_4);
  GpioInterface pRW(GPIOA, LL_GPIO_PIN_5);
  GpioInterface pE(GPIOA, LL_GPIO_PIN_6);

  GpioInterface pD4(GPIOA, LL_GPIO_PIN_3);
  GpioInterface pD5(GPIOA, LL_GPIO_PIN_2);
  GpioInterface pD6(GPIOA, LL_GPIO_PIN_1);
  GpioInterface pD7(GPIOA, LL_GPIO_PIN_0);

LiquidCrystal display(&pRS, &pRW, &pE, &pD4, &pD5, &pD6, &pD7);

display.begin(16, 2, LCD_5x10DOTS);
display.clear();
display.setCursor(0, 0);
display.print("please wait...");
LL_mDelay(2000);
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  acurMVolts = getAccuratMVolts();
	  percentOfPower = voltToPercentBattery(acurMVolts);
    /* USER CODE END WHILE */
		display.setCursor(0, 0);
		display.print("                ");
		display.setCursor(0, 0);
		sprintf(strBuff, "%lu.%luV %u.%u%%",
				acurMVolts/1000,
				acurMVolts/100 - ((acurMVolts/1000) * 10),
				percentOfPower / 10,
				percentOfPower - ((percentOfPower/10) * 10));

		display.print(strBuff);
		display.setCursor(0, 1);
		display.print("                ");
		display.setCursor(0, 1);
		sprintf(strBuff, "rem %ud %uh %um", remTime.day, remTime.hour, remTime.min);
		display.print(strBuff);
		acurMVoltsFiltr += acurMVolts;
		acurFiltrCnt++;
		LL_mDelay(100);
		if (getSeconds() >= 60) {
			acurMVoltsFiltr /= acurFiltrCnt;
			voltHistory.put(acurMVoltsFiltr);
			acurMVoltsFiltr = 0;
			acurFiltrCnt = 0;
			remTime = calcTimeRemain(&voltHistory);
			clearSeconds();
		}
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  /* HSI configuration and activation */
  LL_RCC_HSI_Enable();
  while(LL_RCC_HSI_IsReady() != 1)
  {
  }

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);

  /* Sysclk activation on the HSI */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  }

  /* Set APB1 prescaler*/
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

  LL_Init1msTick(16000000);

  /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
  LL_SetSystemCoreClock(16000000);
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOC);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);

  /**/
  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_7);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_1);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_3);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);

  /**/
  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_6);

  /**/
  LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_3);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_1;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_6;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /**/
  GPIO_InitStruct.Pin = LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
