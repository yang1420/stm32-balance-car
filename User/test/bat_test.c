#include "main.h"
#include "app_bat.h"
#include "app_usart2.h"

//
// @简介：电池电压监控模块的测试程序
//        通过串口2把电压发送给电脑
//        用Vofa显示曲线
//
void Bat_Test(void)
{
	App_Bat_Init();

	while(1)
	{
		float volt = App_Bat_Get();

		// 转换为整数（毫伏），比如 3.142V -> 3142
		int volt_mv = (int)(volt * 1000);
		App_USART2_Printf("Volt: %d mV\n", volt_mv);

		HAL_Delay(5);
	}
}
