/*
 * LiquidCrystalUserDef.cpp
 *
 *  Created on: 27 черв. 2023 р.
 *      Author: Oleg Dus
 */
#include "LiquidCrystalUserInterface.h"
#include <cstdint>
#include "stm32g0xx_ll_gpio.h"
#include "stm32g0xx_ll_bus.h"
#include "tim.h"

void delayMicroseconds(uint16_t val) {
	delayUS (val);
}



void GpioInterface::initOut()
{
	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	  GPIO_InitStruct.Pin = Pin_Mask;
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	  LL_GPIO_Init(GPIO_x, &GPIO_InitStruct);
}

void GpioInterface::initIn()
{
	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

	  GPIO_InitStruct.Pin = Pin_Mask;
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	  LL_GPIO_Init(GPIO_x, &GPIO_InitStruct);
}

uint8_t GpioInterface::PinRead ()
{
	return LL_GPIO_ReadInputPort(GPIO_x) & Pin_Mask ? 1 : 0;
}

void GpioInterface::PinSet (uint8_t val)
{
	if (val) {
		LL_GPIO_SetOutputPin(GPIO_x, Pin_Mask);
	} else {
		LL_GPIO_ResetOutputPin(GPIO_x, Pin_Mask);
	}
}

GpioInterface::GpioInterface(GPIO_TypeDef *GPIOx, uint32_t PinMask)
{
	GPIO_x = GPIOx;
	Pin_Mask = PinMask;
	if (GPIO_x == GPIOA) {
		LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOA);
	} else if (GPIO_x == GPIOB) {
		LL_IOP_GRP1_EnableClock(LL_IOP_GRP1_PERIPH_GPIOB);
	}
}
