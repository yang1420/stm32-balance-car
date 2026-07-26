#include "app_motor.h"
#include "pid.h"
#include "task.h"
#include "app_pwm.h"
#include "app_encoder.h"
#include "app_bat.h"

static PID_TypeDef pid_motor_l;
static PID_TypeDef pid_motor_r;


void App_Motor_Init(void)
{
    PID_Init(&pid_motor_l, 0.55, 7, 0.0f);//PI controller for left motor
    PID_SetOutputLimits(&pid_motor_l, -8.4f, 8.4f); // Set output limits for left motor
    PID_Init(&pid_motor_r, 0.55, 7, 0.0f);//PI controller for right motor
    PID_SetOutputLimits(&pid_motor_r, -8.4f, 8.4f); // Set output limits for right motor

}

void App_Motor_Proc(void)
{
    PERIODIC(1);// Run this function every 1 ms
    // Get the current speed of the left and right motors(rad/s)
    float speed_l = App_Encoder_GetSpeed_L();
    float speed_r = App_Encoder_GetSpeed_R();


    // Compute the control output for the left and right motors
    float control_output_l = PID_Compute(&pid_motor_l, speed_l);
    float control_output_r = PID_Compute(&pid_motor_r, speed_r);

    float ua_l = control_output_l;
    float ua_r = control_output_r;

    // Set the PWM duty cycle for the left and right motors
    float vbat = App_Bat_Get();
    float duty_l = ua_l / vbat * 100.0f;
    float duty_r = ua_r / vbat * 100.0f;


    App_PWM_Set_L(duty_l);
    App_PWM_Set_R(duty_r);

}
//
//@Summary: Set the speed of the left motor in rad/s
//speed: desired speed in rad/s
void App_Motor_SetSpeed_L(float speed)
{
    PID_ChangeSP(&pid_motor_l, speed);
} 
//
//@Summary: Set the speed of the right motor in rad/s
// speed: desired speed in rad/s
void App_Motor_SetSpeed_R(float speed)
{
    PID_ChangeSP(&pid_motor_r, speed);
}
//
//@Summary: turn the motor on or off
//@Parameters: on - 0 to turn off, non-zero to turn on
//
void App_Motor_Cmd(uint8_t on)
{
    App_PWM_Cmd(on);
    PID_Reset(&pid_motor_l);
    PID_Reset(&pid_motor_r);
}