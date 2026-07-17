#include "app_bat.h"

#include "main.h"
//#include "task.h"

static volatile float volt = 0; // Battery voltage value


extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc1;

//
//@Summary: Initialize the battery monitoring functionality
//
void App_Bat_Init(void)
{
	HAL_TIM_Base_Start(&htim2); // Open TIM2, TIM2 will overflow periodically to trigger ADC sampling(update event)
	HAL_ADCEx_InjectedStart_IT(&hadc1); // Open ADC1 injected conversion
}

//
// @summary: Return the battery voltage value
//
float App_Bat_Get(void)
{
	return volt;
}

//
//@Summary: Slice for battery event processing.
//

void App_Bat_Proc(void)
{
	// PERIODIC(20)  // 注释掉，此宏未定义

	// 5.8 6.6 7.4 8.2
	static uint32_t nxtBlinkTime = 0;
	static uint8_t blinkStage = 0; // 灭

	if(volt < 6) // 亏电，闪灯100ms
	{
		if(HAL_GetTick() > nxtBlinkTime)
		{
			nxtBlinkTime = HAL_GetTick() + 100;//delay time 100ms

			if(blinkStage == 0)
			{
				blinkStage = 1;
				// Full light up
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
			}
			else
			{
				blinkStage = 0;

				// All off
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
			}
		}
	}
	else if(volt < 6.6) // light 0 <33 %
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	}
	else if(volt < 7.3) // light 1 <66 %
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	}
	else if(volt < 8.0) // light 2 <95 %
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	}
	else // light 3 >=95 %
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	}
}






//
//@Summary: Handle the completion of an injected ADC conversion，calculate the battery voltage by ADC
//
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if (hadc->Instance == ADC1)
  {
   	if(hadc == &hadc1)
	{
		//uint16_t dr = HAL_ADC_GetValue(&hadc1);//get the ADC value, this is regular case 
        uint16_t dr = HAL_ADCEx_InjectedGetValue(&hadc1,ADC_INJECTED_RANK_1);
		volt = dr / 4095.0f * 8.4;  
	}
  }
}