/*
 * @Author: Headmaster1615  e-mail:hm-218@qq.com
 * @Date: 2022-12-31 12:43:50
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2022-12-31 13:56:41
 * @FilePath: \USERd:\STM32\My Project\Flight Controller\SYSTEM\adc.h
 * @Description: 
 * 
 * Copyright (c) 2022 by Headmaster1615, All Rights Reserved. 
 */
#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

extern u16 ADC_TempValue[4];

void Adc_Init(void);
void ADC_DMA_Init(void);
u16 Get_Bat_Vol(void);//��Ҫ10ms
u16  Get_Adc(u8 ch); 
float Get_Adc_Average(u8 ch,u8 times); 
float Get_Temperature(void);

#endif 
