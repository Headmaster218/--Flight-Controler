/*
 * @Author: Headmaster1615  e-mail:hm-218@qq.com
 * @Date: 2022-12-30 22:09:25
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2023-01-03 19:38:24
 * @FilePath: \USERd:\STM32\My Project\Flight Controller\HARDWARE\serio.h
 * @Description:
 *
 * Copyright (c) 2023 by Headmaster1615, All Rights Reserved.
 */
#ifndef __SERIO_H
#define __SERIO_H
#include "sys.h"

struct serio_Data_
{
    u16 pwm_output_offset[5]; // �������������ƽβ����βpwm��׼ֵ
    short pwm_output[5];      // �����(��������//+-170max,+-130normal)��ƽβ����β
    short last_pwm_output[5]; // �����(��������//+-170max,+-130normal)��ƽβ����β
};

void PWM_Init(u16 arr, u16 psc);
void PWM_Output(void);
#endif
