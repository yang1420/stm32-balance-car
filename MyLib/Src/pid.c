#include "pid.h"
#include "delay.h"
//
//@Summary: Initialize PID controller parameters
//
void PID_Init(PID_TypeDef *pid, float Kp, float Ki, float Kd)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->setpoint = 0.0f;
    pid->t_k_last = 0;
    pid->error_last = 0.0f;
    pid->error_integral_last = 0.0f;
    pid->UpperLimit = 3.4e+38f; // Set upper limit of output, infinity
    pid->LowerLimit = -3.4e+38f; // Set lower limit of output, negative infinity
}

//
//@Summary: Change the setpoint of the PID controller
//
void PID_ChangeSP(PID_TypeDef *pid, float setpoint)
{
    pid->setpoint = setpoint;
}
float PID_Compute(PID_TypeDef *pid, float FB)
{
    float error = pid->setpoint - FB;
    uint64_t current_time = GetUs();
    float dt = (current_time - pid->t_k_last) * 1.0e-6f; // Convert microseconds to seconds
    float err_dev = 0.0f;
    float err_int = 0.0f;
    if(pid->t_k_last != 0) 
    {
        err_dev = (error - pid->error_last) / dt; // Derivative of error
        err_int = pid->error_integral_last + (error + pid->error_last) * dt / 2.0f; // Integral of error using trapezoidal rule
    } 
    
    
    // Implement PID computation here
    float COp= pid->Kp * error; // P term
    float Coi= pid->Ki * err_int; // I term
    float Cod= pid->Kd * err_dev; // D term
    
    
    // Update last values
    pid->t_k_last = current_time;
    pid->error_last = error;
    pid->error_integral_last = err_int;

    float CO = COp + Coi + Cod;
    // Apply output limits
    if (CO > pid->UpperLimit) {
        CO = pid->UpperLimit;
    } else if (CO < pid->LowerLimit) {
        CO = pid->LowerLimit;
    }

    //Prevent integral windup by limiting the integral term
    if(pid->error_integral_last > pid->UpperLimit) {
        pid->error_integral_last = pid->UpperLimit;
    } else if(pid->error_integral_last < pid->LowerLimit) {
        pid->error_integral_last = pid->LowerLimit;
    }
    return CO;
}
//
//@Summary: Set the output limits of the PID controller
//
void PID_SetOutputLimits(PID_TypeDef *pid, float lower, float upper)
{
    pid->LowerLimit = lower;
    pid->UpperLimit = upper;
}

//@Summary: Reset the PID controller
//
void PID_Reset(PID_TypeDef *pid)
{
    pid->t_k_last = 0;
    pid->error_last = 0.0f;
    pid->error_integral_last = 0.0f;
}

