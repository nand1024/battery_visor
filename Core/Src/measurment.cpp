/*
 * measurment.cpp
 *
 *  Created on: 2 вер. 2023 р.
 *      Author: 2andn
 */

#include <stdint.h>
#include "stm32g0xx_ll_adc.h"
#include "stm32g0xx_ll_rcc.h"
#include "stm32g0xx_ll_bus.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_gpio.h"



#define ADC_SAMPLES    48
#define SIZE_ADC_DMA_BUFFER (ADC_SAMPLES * 3)
#define ADC_REF_MICROVOLTS    3300000UL
#define ADC_BITRATE           4096UL
#define ADC_OVERSAMPLING	  16

#define ADC_TO_MICROVOLTS(V) (((uint64_t)(V / ADC_OVERSAMPLING) * ADC_REF_MICROVOLTS) / ADC_BITRATE)

static uint16_t rawAdcFiltr[SIZE_ADC_DMA_BUFFER];

static uint32_t approxMVolts, accuratelyMVolts, current;

uint32_t getApproxMVolts(void) { return approxMVolts;}

uint32_t getAccuratMVolts(void) { return accuratelyMVolts;}

uint32_t getCurrent(void) { return current;}

extern "C" {
	void DMA1_Channel1_IRQHandler(void)
	{
		if (LL_DMA_IsActiveFlag_TC1(DMA1)) {
			LL_DMA_ClearFlag_TC1(DMA1);
			uint64_t approxMV, accuratelyMV, cr;
			approxMV = accuratelyMV = cr = 0;
			for (uint16_t i = 0; i < SIZE_ADC_DMA_BUFFER; i+=3) {
				approxMV += ADC_TO_MICROVOLTS(rawAdcFiltr[i]) / 1000;
				accuratelyMV += ADC_TO_MICROVOLTS(rawAdcFiltr[i + 1]) / 1000;
				cr += ADC_TO_MICROVOLTS(rawAdcFiltr[i + 2]) / 1000;
			}
			approxMV /= ADC_SAMPLES;
			accuratelyMV /= ADC_SAMPLES;
			cr /= ADC_SAMPLES;
			approxMVolts = (approxMV * (9690 + 1005)) / 1005;// resistor R1(9690) R2(1005)
			accuratelyMVolts = (accuratelyMV * (340 + 330)) / 330 + 9560;// resistor R1(340) R2(330) + 9560mVolt diod zener
			current = cr;
		}
	}
}

void MX_ADC1_Init(void)
{


	//DMA for ADC
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);



	LL_DMA_ConfigAddresses(DMA1,
						   LL_DMA_CHANNEL_1,
						   (uint32_t)&(ADC1->DR),
						   (uint32_t)rawAdcFiltr,
						   LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, SIZE_ADC_DMA_BUFFER);
	LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_ADC1);
	LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_VERYHIGH);
	LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_CIRCULAR);
	LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);
	LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);
	LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_HALFWORD);
	LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_HALFWORD);

	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
	LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_ADC1);
	LL_DMAMUX_SetRequestID(DMAMUX1, LL_DMAMUX_CHANNEL_0, LL_DMAMUX_REQ_ADC1);

	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_ADC);

  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
  LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
  /**ADC1 GPIO Configuration
  PA7   ------> ADC1_IN7
  PB0   ------> ADC1_IN8
  PA11 [PA9]   ------> ADC1_IN15
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  LL_ADC_ClearFlag_ADRDY(ADC1);
  LL_ADC_EnableInternalRegulator(ADC1);
  LL_ADC_StartCalibration(ADC1);
  while(LL_ADC_IsCalibrationOnGoing(ADC1));
  LL_ADC_ClearFlag_CCRDY(ADC1);
  while(LL_ADC_IsActiveFlag_CCRDY(ADC1));
  LL_ADC_REG_SetContinuousMode(ADC1, LL_ADC_REG_CONV_CONTINUOUS);
  LL_ADC_REG_SetSequencerConfigurable(ADC1, LL_ADC_REG_SEQ_FIXED);
  LL_ADC_REG_SetSequencerScanDirection(ADC1, LL_ADC_REG_SEQ_SCAN_DIR_FORWARD);
  LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_7 | LL_ADC_CHANNEL_8 | LL_ADC_CHANNEL_15);
  //LL_ADC_REG_SetSequencerChannels(ADC1, LL_ADC_CHANNEL_8 );
  while(!LL_ADC_IsActiveFlag_CCRDY(ADC1));
  LL_ADC_ClearFlag_CCRDY(ADC1);
  LL_ADC_SetCommonClock(ADC1_COMMON, LL_ADC_CLOCK_ASYNC_DIV256);

  LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_COMMON_1, LL_ADC_SAMPLINGTIME_160CYCLES_5);
  LL_ADC_SetSamplingTimeCommonChannels(ADC1, LL_ADC_SAMPLINGTIME_COMMON_2, LL_ADC_SAMPLINGTIME_160CYCLES_5);
  LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_7 | LL_ADC_CHANNEL_8 | LL_ADC_CHANNEL_15, LL_ADC_SAMPLINGTIME_COMMON_1);
  //LL_ADC_SetChannelSamplingTime(ADC1, LL_ADC_CHANNEL_8 , LL_ADC_SAMPLINGTIME_COMMON_1);
  LL_ADC_ConfigOverSamplingRatioShift(ADC1, LL_ADC_OVS_RATIO_16, LL_ADC_OVS_SHIFT_NONE);
  LL_ADC_SetOverSamplingScope(ADC1, LL_ADC_OVS_GRP_REGULAR_CONTINUED);
  LL_ADC_SetClock(ADC1, LL_ADC_CLOCK_SYNC_PCLK_DIV2);
  LL_ADC_SetDataAlignment(ADC1, LL_ADC_DATA_ALIGN_RIGHT);
  LL_ADC_SetResolution(ADC1, LL_ADC_RESOLUTION_12B);

  LL_ADC_REG_SetDMATransfer(ADC1, LL_ADC_REG_DMA_TRANSFER_UNLIMITED);

  LL_ADC_Enable(ADC1);
  while(!LL_ADC_IsActiveFlag_ADRDY(ADC1));
  LL_ADC_REG_StartConversion(ADC1);
}


