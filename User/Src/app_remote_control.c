#include "app_remote_control.h"
#include <string.h>
#include <stdio.h>

#define REMOTE_CONTROL_BUF_SIZE 64

extern UART_HandleTypeDef huart3; // 蓝牙模块接在USART3上（9600波特率）

static char intBuf[REMOTE_CONTROL_BUF_SIZE];//接收中断缓冲区
static char transBuf[REMOTE_CONTROL_BUF_SIZE];//在中断程序和进程函数转移数据的
static char procBuf[REMOTE_CONTROL_BUF_SIZE];//进程函数处理数据的缓冲区

static uint8_t volatile lineReceivedFlag = 0;//标志位，表示接收到了完整的一行数据
static uint16_t volatile intBufCursor = 0;//接收中断缓冲区的游标，指向下一个空白位置
static uint8_t rx_byte;

void App_Remote_Control_Init(void)
{   
    HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
}
void App_Remote_Control_Proc(void)
{
    
    if (lineReceivedFlag)
    {
        //1. read the data from the buffer and process it
        lineReceivedFlag = 0; // 清除标志位，表示已经处理了接收到的一行数据
        strcpy(procBuf, transBuf); // 将处理缓冲区的数据复制到进程函数的缓冲区

        //2. process the data
        if(strncmp(procBuf,"move ",5) ==0)
        {
            int turnSpeed, moveSpeed;
            if (sscanf(procBuf, "move %d %d", &turnSpeed, &moveSpeed) == 2)
            {
                // Successfully parsed turnSpeed and moveSpeed
                
            }
        }

    }


}



void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3) // 只处理蓝牙模块（USART3）的接收中断
    {
        if (rx_byte != '\n') // 没有检测到换行符，表示接收了一行数据
        {
            intBuf[intBufCursor]= (char)rx_byte;
            intBufCursor++;
        }
        else // 检测到换行符，表示接收了一行数据
        {
            intBuf[intBufCursor] = '\0'; // 在接收缓冲区的末尾添加字符串结束符
            strcpy(transBuf, intBuf); // 将接收缓冲区的数据复制到处理缓冲区

            lineReceivedFlag = 1;
            intBufCursor = 0; // 重置游标，准备接收下一行数据
        }
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1); // 重新开启下一次接收中断
    }
}
