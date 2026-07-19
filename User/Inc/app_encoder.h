/*
 * app_encoder.h
 *
 *  Created on: May 6, 2025
 *      Author: gaoxi
 */

#ifndef INC_APP_ENCODER_H_
#define INC_APP_ENCODER_H_

#include "main.h"

void App_Encoder_Init(void);
float App_Encoder_GetPos_L(void);
float App_Encoder_GetPos_R(void);
float App_Encoder_GetSpeed_L(void);
float App_Encoder_GetSpeed_R(void);


#endif /* INC_APP_ENCODER_H_ */
