/*
 * @Author: Headmaster1615  e-mail:hm-218@qq.com
 * @Date: 2022-05-17 00:21:42
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2023-01-03 18:47:19
 * @FilePath: \USERd:\STM32\My Project\Flight Controller\HARDWARE\24G.h
 * @Description: 
 * 
 * Copyright (c) 2023 by Headmaster1615, All Rights Reserved. 
 */
#ifndef __24G_H
#define __24G_H
#include "delay.h"

//frequency 4Hz
struct send_data_//14 Byte
{
    u8 height;// /10
    u8 spd;//km/h

    u8 voltage;//(V-10.6)*50(percent)
    u8 temperature;//(C+100)*2

    u8 ECC_Code;//sum
    u8 reserved[3];

    short latitude;//(degree*100)
    short longitude;//(degree*100)
	  short pitch;//*100
    short roll;//*100
};

//frequency 20Hz
struct receive_data_//8 Byte
{
    u8 acc;//engine 0-200
    u8 LR;//left-right 0-200
    u8 UD;//up-down 0-200
    u8 HRL;//horizontal wing right-left 0-200
    u8 ECC_Code;//sum
    u8 reserved[3];
};

void Wireless_Init(void);
void Wireless_UART_Init(u32);
void Wireless_DMA_Init(void);
void Wireless_Send_Data();


#endif
