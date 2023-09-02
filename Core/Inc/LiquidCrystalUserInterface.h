/*
 * LiquidCrystalUserDef.h
 *
 *  Created on: 27 черв. 2023 р.
 *      Author:  Oleg Dus
 */

#ifndef INC_LIQUIDCRYSTALUSERINTERFACE_H_
#define INC_LIQUIDCRYSTALUSERINTERFACE_H_

#include <cstdint>
#include "stm32g0xx_ll_gpio.h"

void delayMicroseconds(uint16_t val);

class GpioInterface {
private:
	GPIO_TypeDef *GPIO_x;
	uint32_t Pin_Mask;
public:
	void initOut();
	void initIn();
	uint8_t PinRead ();
	void PinSet (uint8_t val);
	GpioInterface(GPIO_TypeDef *GPIOx, uint32_t PinMask);
};

#endif /* INC_LIQUIDCRYSTALUSERINTERFACE_H_ */
