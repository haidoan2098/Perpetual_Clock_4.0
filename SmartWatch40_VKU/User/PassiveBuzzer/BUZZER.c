/*
 * BUTTON.c
 *
 *  Created on: Oct 3, 2024
 *      Author: haidoan2098
 */

#include "BUZZER.h"

uint16_t presForFrequency (uint16_t frequency)
{
	if (frequency == 0) return 0;
	return ((TIM_FREQ/(1000*frequency))-1);
}

void noTone(TIM_HandleTypeDef *htim)
{
	__HAL_TIM_SET_PRESCALER(htim, 0);
}

void playTone (uint16_t tone, TIM_HandleTypeDef *htim)
{
	uint16_t pres = presForFrequency(tone);
	__HAL_TIM_SET_PRESCALER(htim, pres);
}
