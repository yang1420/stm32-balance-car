/*
 * app_encoder.c
 *
 *  Created on: May 6, 2025
 *      Author: gaoxi
 */


#include "app_encoder.h"
#include "delay.h"

static volatile int64_t encoder_l = 0; // 左电机编码器的值
static volatile int64_t encoder_r = 0; // 右电机编码器的值

static volatile int8_t direction_l = 1; // 左电机旋转的方向，1 - 正转，-1 - 反转
static volatile int8_t direction_r = 1; // 右电机旋转的方向，1 - 正转，-1 - 反转
static volatile uint64_t t0_l = 0, t1_l = 0; // 左电机编码器发生变化的时间，单位us
static volatile uint64_t t0_r = 0, t1_r = 0; // 右电机编码器发生变化的时间，单位us

//
// @简介：对编码器模块进行初始化
//
void App_Encoder_Init(void)
{
}

//
// @简介：读取左轮胎旋转的角度
//
float App_Encoder_GetPos_L(void)
{
	return encoder_l/22.0f/(30613.0f/1500.0f)*360.0f;
	//encoder_l/22.0是线圈转了多少角度，
	//然后/(30613.0f/1500.0f)是减速比，*360.0f是转成角度,所以对应的是左轮胎转的角度
}

//
// @简介：读取右轮胎旋转的角度
//
float App_Encoder_GetPos_R(void)
{
	return encoder_r/22.0f/(30613.0f/1500.0f)*360.0f;
	//encoder_r/22.0是线圈转了多少角度，
	//然后/(30613.0f/1500.0f)是减速比，*360.0f是转成角度,所以对应的是右轮胎转的角度
}

//@简介：读取左轮胎旋转的角速度，使用的T法
float App_Encoder_GetSpeed_L(void)
{	if(t0_l == t1_l) return 0.0f; // 防止除以0
	return direction_l / ((t0_l - t1_l) * 10.0e-6f) / 22.0f / (30613.0f / 1500.0f) * 360.0f;
}
//@简介：读取右轮胎旋转的角速度，使用的T法
float App_Encoder_GetSpeed_R(void)
{
	if(t0_r == t1_r) return 0.0f; // 防止除以0
	return direction_r / ((t0_r - t1_r) * 10.0e-6f) / 22.0f / (30613.0f / 1500.0f) * 360.0f;
}




//
// @简介：EXTI中断响应函数
//
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_14) // 左编码器
	{
		t1_l = t0_l;
		t0_l = GetUs();

		GPIO_PinState a = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14); // A相的当前电压
		GPIO_PinState b = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15); // B相的当前电压

		if(a == GPIO_PIN_SET) // 上升沿
		{
			if(b == GPIO_PIN_RESET)
			{
				encoder_l--;
				direction_l = -1;
			}
			else
			{
				encoder_l++;
				direction_l = 1;
			}
		}
		else // 下降沿
		{
			if(b == GPIO_PIN_SET)
			{
				encoder_l--;
				direction_l = -1;
			}
			else
			{
				encoder_l++;
				direction_l = 1;
			}
		}
	}

	if(GPIO_Pin == GPIO_PIN_3) // 右编码器
	{
		t1_r = t0_r;
		t0_r = GetUs();

		GPIO_PinState a = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3); // A相的当前电压
		GPIO_PinState b = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4); // B相的当前电压

		if(a == GPIO_PIN_SET) // 上升沿
		{
			if(b == GPIO_PIN_RESET)
			{
				encoder_r++;
				direction_r = 1;
			}
			else
			{
				encoder_r--;
				direction_r = -1;
			}
		}
		else // 下降沿
		{
			if(b == GPIO_PIN_RESET)
			{
				encoder_r--;
				direction_r = -1;
			}
			else
			{
				encoder_r++;
				direction_r = 1;
			}
		}
	}
}



