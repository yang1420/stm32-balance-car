/*
 * delay.h
 *
 *  Created on: 2025年5月6日
 *      Author: gaoxi
 */

#ifndef INC_DELAY_H_
#define INC_DELAY_H_

#include "main.h"

    void Delay_Init(void); // 延迟函数初始化
    void Delay(uint32_t ms); // 延迟
uint32_t GetTick(void); // 获取系统的当前时间
uint64_t GetUs(void); // 获取当前的微秒级时间 1us = 1/1000ms
    void DelayUs(uint32_t us); // 微秒级延迟


#endif /* INC_DELAY_H_ */
