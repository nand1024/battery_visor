/*
 * measurment.h
 *
 *  Created on: 2 вер. 2023 р.
 *      Author: 2andn
 */

#ifndef INC_MEASURMENT_H_
#define INC_MEASURMENT_H_
#include <stdint.h>

uint32_t getApproxMVolts(void);

uint32_t getAccuratMVolts(void);

uint32_t getCurrent(void);

void MX_ADC1_Init(void);

#endif /* INC_MEASURMENT_H_ */
