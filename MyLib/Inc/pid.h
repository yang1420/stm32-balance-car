#ifndef PID_H
#define PID_H
#include "main.h"

typedef struct
{
    float Kp; // proportional coefficient
    float Ki; // integral coefficient
    float Kd; // derivative coefficient
    float setpoint; // desired value

    uint64_t t_k_last; // last time run pid, t[k-1]
    float error_last; // last error, error[k-1]
    float error_integral_last; // last integral of error, integral_error[k-1]
    float UpperLimit; // upper limit of output
    float LowerLimit; // lower limit of output
}PID_TypeDef;

void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd);
void PID_ChangeSP(PID_TypeDef *pid, float setpoint);
float PID_Compute(PID_TypeDef *pid, float FB);
void PID_SetOutputLimits(PID_TypeDef *pid, float lower, float upper);
void PID_Reset(PID_TypeDef *pid);
#endif