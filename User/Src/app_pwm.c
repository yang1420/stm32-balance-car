#include "app_pwm.h"
#include <math.h>



extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
//
//Summary: Initialize TB6612FNG motor driver PWM control
//
void App_PWM_Init(void)
{
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
}


//
// @简介：控制TB6612进入休眠状态或者活动状态
// @参数：on    0 - 休眠状态，向STBY写L
//           非零 - 活动状态，向STBY写H
//
void App_PWM_Cmd(uint8_t on)
{
	if(on == 0)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); // 休眠
	}
	else
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); // 活动
	}
}
//
// @设置左电机的占空比
// duty： -100.0f ~ 100.0f 表示占空比
//

void App_PWM_Set_L(float Duty)
{
    //IN1 高电压， IN2 低电压，电机正转
    //IN2 低电压， IN1 高电压，电机反转
    	float sign; // 符号，正数 - +1， 负数 - -1

	if(Duty >= 0) sign = 1;
	else sign = -1;

	Duty = fabsf(Duty);

	if(sign < 0) // 反转
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);    // AIN1 - 高
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // AIN2 - 低
	}
	else //正转
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9,  GPIO_PIN_RESET);  // AIN1 - 低
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);    // AIN2 - 高
	}

	uint16_t ccr = Duty / 100.0f * (999+1);

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr);

}
void App_PWM_Set_R(float Duty)
{
    	float sign; // 符号，正数 - +1， 负数 - -1

	if(Duty >= 0) sign = 1;
	else sign = -1;

	Duty = fabsf(Duty);

	if(sign >= 0) // 正转
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);    // BIN1 - 高
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);  // BIN2 - 低
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);  // BIN1 - 低
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);    // BIN2 - 高
	}

	uint16_t ccr = Duty / 100.0f * (999+1);

	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, ccr);
}