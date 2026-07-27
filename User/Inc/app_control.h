#ifndef APP_CONTROL_H
#define APP_CONTROL_H
#include "main.h"

void App_Control_Init(void);
void App_Control_Proc(void);
void App_Control_Reset(void);
void App_Control_SetMoveSpeed(float speed);
void App_Control_SetTurnSpeed(float speed);
#endif