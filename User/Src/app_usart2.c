#include <stdarg.h>
#include "main.h"
#include <string.h>
#include <stdio.h>

extern UART_HandleTypeDef huart2;

//
// @summary: Print formatted data to USART2
//
void App_USART2_Printf(const char* Format, ...)
{
	char format_buffer[256];
	
	va_list argptr;
	va_start(argptr, Format);
	
	// 直接使用 vsprintf，不用 vsnprintf，看看是否是大小参数的问题
	vsprintf(format_buffer, Format, argptr);
	va_end(argptr);
	
	// 发送整个字符串
	HAL_UART_Transmit(&huart2, (uint8_t *)format_buffer, strlen(format_buffer), HAL_MAX_DELAY);
}