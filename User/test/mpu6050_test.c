/*
 * mpu6050_test.c
 *
 *  Created on: May 6, 2025
 *      Author: gaoxi
 */

#include "mpu6050_test.h"
#include "app_usart2.h"
#include "app_mpu6050.h"
#include "delay.h"

void MPU6050_Test(void)
{
	App_MPU6050_Init();

	while(1)
	{
		App_MPU6050_Update();

		float ax = App_MPU6050_GetAx();
		float ay = App_MPU6050_GetAy();
		float az = App_MPU6050_GetAz();

		float temperature = App_MPU6050_GetTemperature();

		float gx = App_MPU6050_GetGx();
		float gy = App_MPU6050_GetGy();
		float gz = App_MPU6050_GetGz();

		App_USART2_Printf("%f,%f,%f,%f,%f,%f,%f\n", ax, ay, az, temperature, gx, gy, gz);

		Delay(10);
	}
}

