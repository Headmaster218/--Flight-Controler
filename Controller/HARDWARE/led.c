/*
 * @Author: Headmaster1615  e-mail:hm-218@qq.com
 * @Date: 2023-01-10 10:18:35
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2023-01-10 10:21:17
 * @FilePath: \USERd:\STM32\My Project\Flight Controller\Controller\HARDWARE\led.c
 * @Description: 
 * 
 * Copyright (c) 2023 by Headmaster1615, All Rights Reserved. 
 */
#include "led.h"


void LED_Init(void)
{

 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB,PE�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14;				 //LED-->PC13 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		 //IO���ٶ�Ϊ50MHz
	
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;				 //LED-->PC13 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		 //IO���ٶ�Ϊ50MHz
	
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5
	
 GPIO_SetBits(GPIOC,GPIO_Pin_13);						 //PB.5 �����
GPIO_SetBits(GPIOC,GPIO_Pin_14);

}
 
