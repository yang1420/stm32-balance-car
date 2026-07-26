#include "app_control.h"
#include "pid.h"
#include "task.h"
#include "app_mpu6050.h"
#include "q_math.h"
#include "app_motor.h"
#include "delay.h"
static PID_TypeDef pid_theta;// PID for angle control
static PID_TypeDef pid_theta_dot;// PID for angular velocity control
static const float l=0.062f; // Length of the pendulum in meters
static const float g=9.8f; // Acceleration due to gravity, in m/s^2
static const float r=0.032f; // Radius of the wheel in meters
static float omega_ref=0.0f; // Reference angular velocity of the wheel, in rad/s


void App_Control_Init(void)
{
    PID_Init(&pid_theta, 4.0f, 0.0f, 0.0f);
    PID_SetOutputLimits(&pid_theta, -12.57f, 12.57f); // Limit output to -4π to 4π radians

    PID_Init(&pid_theta_dot, 10.0f, 10.0f, 0.0f);
    PID_SetOutputLimits(&pid_theta_dot, -125.7f, 125.7f); // Limit output to -40π to 40π radians

}


static float last_time = 0.0f;
void App_Control_Proc(void)
{
    PERIODIC(5); // Run every 5ms, because MPU6050 need to get data every 5ms
    


    PID_ChangeSP(&pid_theta, 0.0f); // Set desired angle to 0 radians (upright position)

    // Get current angle and angular velocity from MPU6050
    float current_theta = App_MPU6050_GetPitch()/180.0f*3.1415926f; // Convert degrees to radians
    float current_theta_dot = App_MPU6050_GetGx()/180.0f*3.1415926f; // Convert degrees/s to radians/s

    //计算pid的外环，Compute PID for outer loop
    float theta_dot_ref= PID_Compute(&pid_theta, current_theta);
    //改变pid内环的设定值, Change the setpoint of the inner loop PID
    PID_ChangeSP(&pid_theta_dot, theta_dot_ref);
    //计算pid的内环，Compute PID for inner loop
    float theta_dot_dot_ref= PID_Compute(&pid_theta_dot, current_theta_dot);
    
    //倒立摆的逆解算， Inverse kinematics for inverted pendulum
    float x_dot_dot_ref=(g*qsin(current_theta)-l*theta_dot_dot_ref)/qcos(current_theta);

    //将加速度转换为电机的转速, 通过微积分算， Convert acceleration to motor speed，by integration
    uint64_t current_time = GetUs();
    float dt = (current_time - last_time) * 1.0e-6f; // Convert microseconds to seconds
    omega_ref+=(1.0f/r)*x_dot_dot_ref*dt; // Integrating acceleration to get speed
    last_time = current_time;

    //设置电机的转速, Set the motor speed
    App_Motor_SetSpeed_L(omega_ref);
    App_Motor_SetSpeed_R(omega_ref);




}