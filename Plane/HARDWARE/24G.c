/*
 * @Author: Headmaster1615  e-mail:hm-218@qq.com
 * @Date: 2022-05-17 00:21:42
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2023-01-05 15:21:03
 * @FilePath: \USERd:\STM32\My Project\Flight Controller\Plane\HARDWARE\24G.c
 * @Description:
 *
 * Copyright (c) 2023 by Headmaster1615, All Rights Reserved.
 */
#include "sys.h"
#include "24G.h"
#include "GPS.h"
#include "MPU6050.h"
#include "adc.h"
struct send_data_ send_Data;
struct receive_data_ receive_Data,DMA_receive_Data;

void Wireless_Send_Data()
{
    u8 i=0;
    send_Data.ECC_Code = 0;
    send_Data.height = (u8)(GPS_Data.height/10);
    send_Data.spd = (u8)(GPS_Data.speed);
    send_Data.voltage = (u8)((ADC_Value[0].num-9500)*7/100);
    send_Data.pitch = (short)(Mpu_Data.pitch*100);
    send_Data.roll = (short)(Mpu_Data.roll*100);
    send_Data.temperature = (u8)((Mpu_Data.temp+10000)/200);
    send_Data.latitude = (short)(GPS_Data.lat_f*100);
    send_Data.longitude = (short)(GPS_Data.lon_f*100);
    for(;i<sizeof(send_Data);i++)
        send_Data.ECC_Code += *((u8*)&send_Data+i);
	
	/*	for(;i<sizeof(send_Data);i++)
	{
		((u8*)&send_Data)[i]=i;
	}*/

	
	DMA1_Channel4->CCR &= 0xFE; // disable dma
	DMA1_Channel4->CNDTR = sizeof(send_Data);
	DMA1_Channel4->CCR |= 1; // enable dma
}
u8 controler_offline_cnt=0,controler_offline_flag=0;
void DMA1_Channel5_IRQHandler()
{
		 u8 temp=0,i;
	DMA_ClearFlag(DMA1_IT_TC5);
	controler_offline_cnt=0;
	for(i=0;i<sizeof(DMA_receive_Data);i++)
        DMA_receive_Data.ECC_Code -= *((u8*)&DMA_receive_Data+sizeof(DMA_receive_Data)-1-i);
    if(DMA_receive_Data.ECC_Code == 0)
    {
        receive_Data = DMA_receive_Data;
    }
}

u8 uart_time_cnt=0;
void USART1_IRQHandler(void) //�����ж�
{

	USART1->SR;
	USART1->DR;
	uart_time_cnt=0;
}

void Wireless_Init()
{
	Wireless_UART_Init(19200);
	Wireless_DMA_Init();
	send_Data.end_of_this=0xffff;
}

void Wireless_UART_Init(u32 bound)
{
    // GPIO�˿�����
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // ʹ��USART1��GPIOAʱ��
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);
	
	
    ////USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; // PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // �����������
    GPIO_Init(GPIOB, &GPIO_InitStructure);          // ��ʼ��GPIOA.9

    // USART1_RX	  GPIOA.10��ʼ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // ��������
    GPIO_Init(GPIOB, &GPIO_InitStructure);        // ��ʼ��GPIOA.10

    // Usart1 NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // ��ռ���ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        // �����ȼ�3
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQͨ��ʹ��
    NVIC_Init(&NVIC_InitStructure);                           // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

    // USART ��ʼ������

    USART_InitStructure.USART_BaudRate = bound;                                     // ���ڲ�����
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // �ֳ�Ϊ8λ���ݸ�ʽ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // һ��ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // ����żУ��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
    USART_InitStructure.USART_Mode = USART_Mode_Rx |USART_Mode_Tx;              // �շ�ģʽ

    USART_Init(USART1, &USART_InitStructure);      // ��ʼ������1
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // �������ڿ����ж�
    USART_Cmd(USART1, ENABLE);                     // ʹ�ܴ���1
}

void Wireless_DMA_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    // ʱ��
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;  // Ƕ��ͨ��ΪDMA1_Channel4_IRQn
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // ��ռ���ȼ�Ϊ 2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        // ��Ӧ���ȼ�Ϊ 7
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // ͨ���ж�ʹ��
    NVIC_Init(&NVIC_InitStructure);

	//����
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE); // DMA1ʱ��ʹ��
    DMA_DeInit(DMA1_Channel4);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);       // DMA�����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&send_Data;            // ���ͻ���ָ��
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;                      // ���䷽��
    DMA_InitStructure.DMA_BufferSize = sizeof(send_Data);                   // ���䳤��
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // �������
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �ڴ����
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݿ�ȣ�BYTE
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �ڴ����ݿ�ȣ�BYTE
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;                           // ѭ��ģʽ����//��ע��DMA_Mode_NormalΪ����ģʽ��DMA_Mode_CircularΪѭ��ģʽ��
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // ���ȼ�����
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // �ڴ棺�ڴ棨����
    DMA_Init(DMA1_Channel4, &DMA_InitStructure);                            // ��ʼ��DMA1_Channel4
    DMA_ClearFlag(DMA1_FLAG_GL4);
    DMA_Cmd(DMA1_Channel4, DISABLE);
    // ����DMA����

	//����
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);                      // DMA1ʱ��ʹ��
    DMA_DeInit(DMA1_Channel5);                                              // ��λDMA1_Channel5
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(&USART1->DR);     // DMA�����ַ
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&DMA_receive_Data;         // ���ջ���ָ��
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      // ���䷽��
    DMA_InitStructure.DMA_BufferSize = sizeof(DMA_receive_Data);                // �����С
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;        // �����ַ����
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;                 // �ڴ��ַ����
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; // �������ݿ�ȣ�BYTE
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;         // �ڴ����ݿ�ȣ�BYTE
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//DMA_Mode_Normal;                           // ��ע��DMA_Mode_NormalΪ����ģʽ��DMA_Mode_CircularΪѭ��ģʽ��
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;                 // ���ȼ�����
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;                            // �ڴ棺�ڴ棨����
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
	DMA_ITConfig(DMA1_Channel5,DMA_IT_TC,ENABLE);
    DMA_ClearFlag(DMA1_IT_TC5);
    DMA_Cmd(DMA1_Channel5, ENABLE);

    // ����DMA���ͷ����ж�
    USART_Cmd(USART1, DISABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx|USART_DMAReq_Tx, ENABLE);
    USART_Cmd(USART1, ENABLE);
}
