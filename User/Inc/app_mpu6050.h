#ifndef APP_MPU6050_H
#define APP_MPU6050_H
#include "main.h"

void App_MPU6050_Init(void);
/*
 * app_encoder.h
 *
 *  Created on: May 6, 2025
 *      Author: gaoxi
 */

#ifndef INC_APP_ENCODER_H_
#define INC_APP_ENCODER_H_

#include "main.h"

void App_MPU6050_Init(void);
void App_MPU6050_Proc(void);


void App_MPU6050_Update(void);

float App_MPU6050_GetAx(void);
float App_MPU6050_GetAy(void);
float App_MPU6050_GetAz(void);

float App_MPU6050_GetTemperature(void);

float App_MPU6050_GetGx(void);
float App_MPU6050_GetGy(void);
float App_MPU6050_GetGz(void);

float App_MPU6050_GetRoll(void);
float App_MPU6050_GetPitch(void);
float App_MPU6050_GetYaw(void);
#endif /* INC_APP_ENCODER_H_ */

#endif // APP_MPU6050_H