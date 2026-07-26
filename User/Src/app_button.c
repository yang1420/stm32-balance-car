#include "app_button.h"
#include "app_pwm.h"
#include "app_usart2.h"
#include "main.h"
#include "app_motor.h"
#include "app_control.h"

#define BUTTON_DEBOUNCE_MS 20u


extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;

static uint8_t last_state = 1;   // default: pull-up input, released = 1
static uint8_t pwm_on = 0;       // 0 - motor sleep, 1 - motor active
static uint32_t last_change_time = 0;

void App_Button_Init(void)
{
    /* GPIO for PA11 is already configured by CubeMX as input with pull-up. */
    last_state = 1;
    last_change_time = HAL_GetTick();

    //对电机的PWm初始化
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);


}

void App_Button_Proc(void)
{
    uint8_t current_state = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_RESET ? 0 : 1;
    uint32_t now = HAL_GetTick();

    if (current_state != last_state)
    {
        if (now - last_change_time >= BUTTON_DEBOUNCE_MS)
        {   
            last_state = current_state;
            last_change_time = now;

            if (current_state == 0)
            {
                pwm_on ^= 1;
                if (pwm_on)
                {
                    App_Control_Reset(); // 重新使能电机前，先清空控制环的PID状态
                    //把电机也复位
                    App_Motor_SetSpeed_L(0);
                    App_Motor_SetSpeed_R(0);
                }
                App_Motor_Cmd(pwm_on);
                //App_PWM_Cmd(pwm_on);
                //App_USART2_Printf("button pressed, pwm=%u\r\n", pwm_on);
            }
        }
    }
    else
    {
        last_change_time = now;
    }
}

