#ifndef APP_MOTOR_H
#define APP_MOTOR_H
#include "main.h"
void App_Motor_Init(void);
void App_Motor_Proc(void);
void App_Motor_SetSpeed_L(float speed);
void App_Motor_SetSpeed_R(float speed);
void App_Motor_Cmd(uint8_t on);
#endif /* APP_MOTOR_H */