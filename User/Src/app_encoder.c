/*
 * app_encoder.c
 *
 *  Created on: May 6, 2025
 *      Author: gaoxi
 */


#include "app_encoder.h"
#include "delay.h"
#include "stm32f1xx_hal_gpio.h"

static volatile int64_t encoder_l = 0; // 左电机编码器的值
static volatile int64_t encoder_r = 0; // 右电机编码器的值

static volatile int8_t direction_l = 1; // 左电机旋转的方向，1 - 正转，-1 - 反转
static volatile int8_t direction_r = 1; // 右电机旋转的方向，1 - 正转，-1 - 反转
static volatile uint64_t t0_l = 0, t1_l = 0; // 左电机编码器发生变化的时间，单位us
static volatile uint64_t t0_r = 0, t1_r = 0; // 右电机编码器发生变化的时间，单位us

static float last_speed_r = 0.0f;
static float last_speed_l = 0.0f;
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

//@简介：读取左轮胎旋转的角速度，使用的T法,单位是弧度每秒，rad/s
float App_Encoder_GetSpeed_L(void)
{
	__disable_irq();//关闭总中断，防止在读取t0_r和t1_r的时候发生中断，导致数据不一致
	int8_t direction_l_copy = direction_l;
	uint64_t t0_l_copy = t0_l;
	uint64_t t1_l_copy = t1_l;
	__enable_irq();//开启总中断
	if(t0_l_copy == t1_l_copy) {
		return last_speed_l; // 防止除以0
	}
	if(direction_l_copy ==2 || direction_l_copy == -2) {
		return 0.0f; // 如果发生了换向，速度为0
	}

	uint64_t now=GetUs();
	uint64_t dt1 = t0_l_copy - t1_l_copy;
	uint64_t dt2 = now - t0_l_copy;
	uint64_t T = (dt1 > dt2) ? dt1 : dt2;
	if(T == 0) {
		return last_speed_l;
	} // 防止除以0

	last_speed_l = direction_l_copy / (T * 1.0e-6f) / 22.0f / (30613.0f / 1500.0f) * 6.283185307f;
	return last_speed_l;
	
}

//@简介：读取右轮胎旋转的角速度，使用的T法，单位是rad/s
float App_Encoder_GetSpeed_R(void)
{
	__disable_irq();//关闭总中断，防止在读取t0_r和t1_r的时候发生中断，导致数据不一致
	int8_t direction_r_copy = direction_r;
	uint64_t t0_r_copy = t0_r;
	uint64_t t1_r_copy = t1_r;
	__enable_irq();//开启总中断

	if(t0_r_copy == t1_r_copy) {
		return last_speed_r; // 防止除以0
	}
	if(direction_r_copy ==2 || direction_r_copy == -2) {
		return 0.0f; // 如果发生了换向，速度为0
	}
	uint64_t now=GetUs();
	uint64_t dt1 = t0_r_copy - t1_r_copy;
	uint64_t dt2 = now - t0_r_copy;
	uint64_t T = (dt1 > dt2) ? dt1 : dt2;

	if(T == 0) {
		return last_speed_r; // 防止除以0
	}
	last_speed_r = direction_r_copy / (T * 1.0e-6f) / 22.0f / (30613.0f / 1500.0f) * 6.283185307f;
	return last_speed_r;
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
		
		//这个代码和下面注释的代码逻辑一样
		if((a ==GPIO_PIN_SET && b ==GPIO_PIN_RESET ) || (a ==GPIO_PIN_RESET && b ==GPIO_PIN_SET)) { //当前轮胎反转
			encoder_l--;
			if(direction_l >0)//之前轮胎正转，但是现在反转
			{
				direction_l = -2;
			}
			else	
			{
				direction_l = -1; //轮胎没有发生换向
			}
			
		}
		else { //当前轮胎是正转
			
			encoder_l++;
			if(direction_l <0)//之前轮胎反转，但是现在正转
			{
				direction_l = 2;
			}
			else
			{
				direction_l = 1;//轮胎没有发生换向
			}
		}
		
		// if(a == GPIO_PIN_SET) // 上升沿
		// {
		// 	if(b == GPIO_PIN_RESET)
		// 	{
		// 		encoder_l--;
		// 		direction_l = -1;
		// 	}
		// 	else
		// 	{
		// 		encoder_l++;
		// 		direction_l = 1;
		// 	}
		// }
		// else // 下降沿
		// {
		// 	if(b == GPIO_PIN_SET)
		// 	{
		// 		encoder_l--;
		// 		direction_l = -1;
		// 	}
		// 	else
		// 	{
		// 		encoder_l++;
		// 		direction_l = 1;
		// 	}
		// }
	}

	if(GPIO_Pin == GPIO_PIN_3) // 右编码器
	{
		t1_r = t0_r;
		t0_r = GetUs();

		GPIO_PinState a = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3); // A相的当前电压
		GPIO_PinState b = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4); // B相的当前电压

		if((a ==GPIO_PIN_SET && b ==GPIO_PIN_RESET ) || (a ==GPIO_PIN_RESET && b ==GPIO_PIN_SET)) { //现在轮胎正转
			encoder_r++;
			if(direction_r <0)//之前轮胎反转，但是现在正转
			{
				direction_r = 2;
			}
			else
			{
				direction_r = 1; //轮胎没有发生换向
			}
		
		}
		else {
			
			encoder_r--;
			if(direction_r >0)//之前轮胎正转，但是现在反转
			{
				direction_r = -2;
			}
			else
			{
				direction_r = -1; //轮胎没有发生换向
			}
		}
		// if(a == GPIO_PIN_SET) // 上升沿
		// {
		// 	if(b == GPIO_PIN_RESET)
		// 	{
		// 		encoder_r++;
		// 		direction_r = 1;
		// 	}
		// 	else
		// 	{
		// 		encoder_r--;
		// 		direction_r = -1;
		// 	}
		// }
		// else // 下降沿
		// {
		// 	if(b == GPIO_PIN_RESET)
		// 	{
		// 		encoder_r--;
		// 		direction_r = -1;
		// 	}
		// 	else
		// 	{
		// 		encoder_r++;
		// 		direction_r = 1;
		// 	}
		// }
	}
}



