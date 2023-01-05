/*
 * @Author: Headmaster1615  e-mail:hm-218@qq.com
 * @Date: 2022-05-23 09:34:57
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2023-01-04 12:16:36
 * @FilePath: \USERd:\STM32\My Project\Flight Controller\HARDWARE\GPS.h
 * @Description: 
 * 
 * Copyright (c) 2023 by Headmaster1615, All Rights Reserved. 
 */
#ifndef __GPS_H
#define __GPS_H

#include "sys.h"
#include <delay.h>

extern struct GPS_Data_ GPS_Data;
void GPS_Init(void);
void GPS_DMA_Init(void);

//RMC_DATA�ṹ��

struct GPS_Data_
{
	u8 time[3]; //h,m,s
	float lon_f, lat_f;
	double lon_real,lat_real;
	u8 num;	//��������
	short height;
	short speed;//km/h
};



#endif
