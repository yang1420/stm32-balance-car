#include "app_mpu6050.h"
#include "task.h"
#include "math.h"
#include "q_math.h"
static void reg_write(uint8_t reg, uint8_t value);
static uint8_t reg_read(uint8_t reg);

extern I2C_HandleTypeDef hi2c1;
static float ax, ay, az; // ax, ay, az are the acceleration values in the x, y, and z axes, respectively, in units of g (gravitational acceleration)
static float temperature; // temperature value in degrees Celsius
static float gx, gy, gz; // gyroscope values in the x, y, and z axes, respectively, in units of degrees per second
static float roll, pitch, yaw; // roll, pitch, and yaw angles in degrees



// @简介：对MPU6050模块进行初始化
//@summary: Initializes the MPU6050 module
void App_MPU6050_Init(void)
{
    //1. initialize I2C interface on PB8,PB9
    //
    //2. Use I2C to write to the MPU6050's power management register to wake it up
    // Write value to register 0x1B(GYRO_CONFIG), and MPU6050 address:0xd0
    reg_write(0x6b, 0x80); // Reset the MPU6050
    HAL_Delay(100); // Wait for 100ms
    reg_write(0x6b, 0x00); // Wake up the MPU6050
    reg_write(0x1b, 0x18); // Set the gyro range to ±2000°/s
    reg_write(0x1c, 0x00); // Set the accelerometer range to ±2g

}
//@Summary: MPU6050 processing function, called in the main loop
void App_MPU6050_Proc(void)
{
    // static uint32_t next = 0;
    // if(HAL_GetTick() < next) {
    //     return;
    // }
    PERIODIC(5);
    App_MPU6050_Update();
    //BY gyroscope to calculate euler angles
    float yaw_g=yaw +gz*0.005;
    float pitch_g=pitch +gx*0.005;
    float roll_g=roll -gy*0.005;

    //By accelerometer to calculate euler angles
    
    float pitch_a=qatan2(ay,az)/3.1415926f*180.0f;
    float roll_a= qatan2(ax,az)/3.1415926f*180.0f;

    //cto fuse gyroscope and accelerometer data
    yaw=yaw_g;
    pitch=0.95238*pitch_g+(1-0.95238)*pitch_a;
    roll=0.95238*roll_g+(1-0.95238)*roll_a;
    
}


//
//@summary: Updates the MPU6050 data
//
void App_MPU6050_Update(void)
{
    int16_t ax_raw = (int16_t)((reg_read(0x3b) << 8) | reg_read(0x3c));
    int16_t ay_raw = (int16_t)((reg_read(0x3d) << 8) | reg_read(0x3e));
    int16_t az_raw = (int16_t)((reg_read(0x3f) << 8) | reg_read(0x40));
    ax= ax_raw *6.1035e-5f; // Convert to g
    ay= ay_raw *6.1035e-5f; // Convert to g 
    az= az_raw *6.1035e-5f; // Convert to g

    int16_t temp_raw = (int16_t)((reg_read(0x41) << 8) | reg_read(0x42));

     // temperature = temperature_raw / 340 + 36.53; // MPU6050
	temperature = temp_raw / 333.87f + 21.0f; // MPU6500


    int16_t gx_raw = (int16_t)((reg_read(0x43) << 8) | reg_read(0x44));
    int16_t gy_raw = (int16_t)((reg_read(0x45) << 8) | reg_read(0x46)); 
    int16_t gz_raw = (int16_t)((reg_read(0x47) << 8) | reg_read(0x48));

    gx = gx_raw * 6.1035e-2f;
	gy = gy_raw * 6.1035e-2f;
	gz = gz_raw * 6.1035e-2f;
}

float App_MPU6050_GetAx(void)
{
    return ax;
}
float App_MPU6050_GetAy(void)
{
    return ay;
}
float App_MPU6050_GetAz(void)
{
    return az;
}           

float App_MPU6050_GetTemperature(void)
{
    return temperature;
}

float App_MPU6050_GetGx(void)
{
    return gx;
}
float App_MPU6050_GetGy(void)
{
    return gy;
}
float App_MPU6050_GetGz(void)
{
    return gz;
}

float App_MPU6050_GetRoll(void)
{
    return roll;
}
float App_MPU6050_GetPitch(void)
{
    return pitch;
}
float App_MPU6050_GetYaw(void)
{
    return yaw;
}







//
//@summary: Writes a value to a register of the MPU6050
//@paramater: reg - the register address to write to, value - the value to write
//
static void reg_write(uint8_t reg, uint8_t value)
{
    uint8_t data[2] = {reg, value};
    HAL_I2C_Master_Transmit(&hi2c1, 0xd0, data, 2, HAL_MAX_DELAY);
}
//
//@summary: Reads a value from a register of the MPU6050
//@paramater: reg - the register address to read from
//@return: the value read from the register
//
static uint8_t reg_read(uint8_t reg)
{
    HAL_I2C_Master_Transmit(&hi2c1, 0xd0, &reg, 1, HAL_MAX_DELAY);
    uint8_t value;
    HAL_I2C_Master_Receive(&hi2c1, 0xd0, &value, 1, HAL_MAX_DELAY);
    return value;
}