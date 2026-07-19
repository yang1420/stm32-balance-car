/*
 * encoder_test.c
 *
 *  Created on: May 6, 2025
 *      Author: gaoxi
 */

#include "encoder_test.h"
#include "app_usart2.h"
#include "delay.h"
#include "app_encoder.h"

//
// @简介：对编码器的位置信号进行测试
//        他会通过调试用的串口把编码器的当前位置显示在Vofa上
//
void Encoder_Test(void) // main
{
	App_Encoder_Init();

	while(1)
	{
		float pos_l = App_Encoder_GetPos_L();
		float pos_r = App_Encoder_GetPos_R();

		App_USART2_Printf("%.2f,%.2f\n", pos_l, pos_r);

		Delay(50);
	}
}
//
// @简介：T法测速的测试代码
//        通过串口把T法测速的Omega值发送到Vofa显示
//
void Encoder_T_Method_Test(void)
{
	App_Encoder_Init();

	while(1)
	{
		Delay(1);

		float omega_l = App_Encoder_GetSpeed_L();
		float omega_r = App_Encoder_GetSpeed_R();

		App_USART2_Printf("%f,%f\n", omega_l, omega_r);
	}
}
