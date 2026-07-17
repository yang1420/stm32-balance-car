/*
 * delay.c
 *
 *  Created on: 2025年5月6日
 *      Author: gaoxi
 */


#include "delay.h"

__IO uint32_t ulTicks;

static uint8_t delay_initialized_flag = 0;

void Delay_Init(void)
{
	if(!delay_initialized_flag)
	{
		delay_initialized_flag = 1;
	}
}

void Delay(uint32_t Delay)
{
	Delay_Init();
	HAL_Delay(Delay);
}

uint32_t GetTick(void)
{
	Delay_Init();
	return HAL_GetTick();
}

uint64_t GetUs(void)
{
	return (uint64_t)HAL_GetTick() * 1000U;
}

void DelayUs(uint32_t us)
{
	Delay_Init();
	HAL_Delay(us / 1000U);
}
