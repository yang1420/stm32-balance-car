#include "app_control.h"
#include "pid.h"
#include "task.h"
#include "app_mpu6050.h"
#include "q_math.h"
#include "app_motor.h"
#include "delay.h"
#include "app_encoder.h"
static PID_TypeDef pid_velocity;// PID for velocity control
static PID_TypeDef pid_theta;// PID for angle control
static PID_TypeDef pid_theta_dot;// PID for angular velocity control


static const float l=0.062f; // Length of the pendulum in meters
static const float g=9.8f; // Acceleration due to gravity, in m/s^2
static const float r=0.032f; // Radius of the wheel in meters
static float omega_ref=0.0f; // Reference angular velocity of the wheel, in rad/s



void App_Control_Init(void)
{
    PID_Init(&pid_velocity, 10.0f, 1.0f, 0.0f);
    PID_SetOutputLimits(&pid_velocity, -0.4f*g , 0.4f*g); // Limit output for velocity control

    PID_Init(&pid_theta, 4.0f, 0.0f, 0.0f);
    PID_SetOutputLimits(&pid_theta, -12.57f, 12.57f); // Limit output to -4π to 4π radians

    PID_Init(&pid_theta_dot, 20.0f, 20.0f, 0.0f);
    PID_SetOutputLimits(&pid_theta_dot, -125.7f, 125.7f); // Limit output to -40π to 40π radians

}


static uint64_t last_time = 0;
void App_Control_Proc(void)
{
    PERIODIC(5); // Run every 5ms, because MPU6050 need to get data every 5ms

/*
   计算速度环：calculate the velocity loop:
*/
    // Set the desired velocity 
    //PID_ChangeSP(&pid_velocity, 0.0f); // Set desired velocity
    //read the current velocity from the encoder
    float current_omega = (App_Encoder_GetSpeed_L() + App_Encoder_GetSpeed_R()) / 2.0f; // Average speed of both wheels, in rad/s
    //计算速度环的反馈：需要算出w1，用w-w2，但是需要用到current_theta和current_theta_dot
     // Get current angle and angular velocity from MPU6050
    float current_theta = App_MPU6050_GetPitch()/180.0f*3.1415926f; // Convert degrees to radians
    float current_theta_dot = App_MPU6050_GetGx()/180.0f*3.1415926f; // Convert degrees/s to radians/s

    float omega2=-current_theta_dot*(l+r)/r; // calculate w2
    float omega1=current_omega-omega2; // calculate w1
    float velocity_feedback=omega1*r; // feedback for velocity PID

    //计算pid的速度环，得到加速度，x_dot_dot，Compute PID for velocity loop
    float theta_ref= qatan(PID_Compute(&pid_velocity, velocity_feedback)/g);


/*
    计算角度环：calculate the angle loop:
*/

    // Set the desired angle  (upright position)
    PID_ChangeSP(&pid_theta,theta_ref); // Set desired angle to theta_ref radians

   
    //计算pid的外环，Compute PID for outer loop
    float theta_dot_ref= PID_Compute(&pid_theta, current_theta);
    //改变pid内环的设定值, Change the setpoint of the inner loop PID

/*
    计算角速度环：calculate the angular velocity loop:
*/

    PID_ChangeSP(&pid_theta_dot, theta_dot_ref);
    //计算pid的内环，Compute PID for inner loop
    float theta_dot_dot_ref= PID_Compute(&pid_theta_dot, current_theta_dot);
    
    //倒立摆的逆解算， Inverse kinematics for inverted pendulum
    float x_dot_dot_ref=(g*qsin(current_theta)-l*theta_dot_dot_ref)/qcos(current_theta);

    //将加速度转换为电机的转速, 通过微积分算， Convert acceleration to motor speed，by integration
    uint64_t current_time = GetUs();
    float dt = (current_time - last_time) * 1.0e-6f; // Convert microseconds to seconds

    if (last_time != 0) {
        omega_ref += (1.0f/r) * x_dot_dot_ref * dt; // Integrating acceleration to get speed
    }
    if (omega_ref > 20.0f)  omega_ref = 20.0f;
    if (omega_ref < -20.0f) omega_ref = -20.0f;
    last_time = current_time;

    //设置电机的转速, Set the motor speed
    App_Motor_SetSpeed_L(omega_ref);
    App_Motor_SetSpeed_R(omega_ref);


}

void App_Control_Reset(void)
{
    last_time = 0;
    omega_ref = 0.0f; //角速度等于0，angular velocity is zero

    PID_Reset(&pid_velocity);
    PID_Reset(&pid_theta);
    PID_Reset(&pid_theta_dot);
}
//
//@Set the balance car move speed, in m/s， the maximum speed is 0.7m/s
//
void App_Control_SetMoveSpeed(float speed)
{
    PID_ChangeSP(&pid_velocity, speed); // Set desired velocity
}
void App_Control_SetTurnSpeed(float speed)
{

}