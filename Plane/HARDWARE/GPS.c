#include <GPS.h>
#include <stdlib.h>
#include <091OLED.h>
#include "usart.h"
#include "my_math.h"

struct GPS_Data_ GPS_Data;
extern int CPU_frec_tick, CPU_freq;

u8 USART_RX_BUF[256];
u8 USART_RX_STA = 0, temppointer = 0, flag_OLED_refresh = 0;
u8 home_pos_cnt=0;
// IN 30 US
void USART3_IRQHandler(void) // 空闲中断
{
	DMA1_Channel3->CCR &= 0xFE; // disable dma
	GPS_Data.offline_cnt=0;
	if (DMA1_Channel3->CNDTR < 25)
	{
		USART3->SR;
		USART3->DR;
		DMA1_Channel3->CCR &= 0xFE; // disable dma
		DMA1_Channel3->CNDTR = 200;
		DMA1_Channel3->CCR |= 1; // enable dma
		return;
	}

	GPIOC->ODR ^= GPIO_Pin_13;
	//$GPVTG,,T,,M,0.029,N,0.001,K,D*2C
	//$GPGGA,060826.00,2236.91284,N,11403.24705,E,2,08,1.03,107.8,M,-2.4,M,,0000*4A
	// 01234567890123456789012345678901234567890123456789012345678901234567890123456
	// GPS数据处理
	temppointer = 0;
	if(USART_RX_BUF[15]!='N')
	{
		while (USART_RX_BUF[temppointer++] != 'N')
			;
		GPS_Data.speed = atof(USART_RX_BUF + temppointer + 1);
		GPS_Data.no_locate_flag = 0;
		home_pos_cnt++;
		if(!GPS_Data.home_point_flag&&home_pos_cnt>15)
		{
			GPS_Data.home_point_flag=1;
			GPS_Data.home_lat=GPS_Data.lat_real;
			GPS_Data.home_lon=GPS_Data.lon_real;
			GPS_Data.home_height=GPS_Data.height;
		}
	}
	else
	{
		home_pos_cnt=0;
		GPS_Data.no_locate_flag = 1;
	}
	while (USART_RX_BUF[temppointer++] != '$' && temppointer < 199)
		;
	if (temppointer == 199)
		return;

	GPS_Data.time[0] = (8 + (USART_RX_BUF[temppointer + 6] - 0x30) * 10 + (USART_RX_BUF[temppointer + 7] - 0x30));
	if (GPS_Data.time[0] < 60)
	{
		GPS_Data.time[0] -= GPS_Data.time[0] >= 24 ? 24 : 0;
		GPS_Data.time[1] = (USART_RX_BUF[temppointer + 8] - 0x30) * 10 + (USART_RX_BUF[temppointer + 9] - 0x30);
		GPS_Data.time[2] = (USART_RX_BUF[temppointer + 10] - 0x30) * 10 + (USART_RX_BUF[temppointer + 11] - 0x30);

		temppointer += 16;
		GPS_Data.lon_f = atof(USART_RX_BUF + temppointer);
		while (USART_RX_BUF[temppointer++] != ',')
			;
		while (USART_RX_BUF[temppointer++] != ',')
			;
		GPS_Data.lat_f = atof(USART_RX_BUF + temppointer);
		while (USART_RX_BUF[temppointer++] != ',')
			;
		while (USART_RX_BUF[temppointer++] != ',')
			;
		while (USART_RX_BUF[temppointer++] != ',')
			;
		GPS_Data.num = atoi(USART_RX_BUF + temppointer);

		while (USART_RX_BUF[temppointer++] != ',')
			;
		while (USART_RX_BUF[temppointer++] != ',')
			;
		GPS_Data.height = atof(USART_RX_BUF + temppointer);

		GPS_Data.lon_real = (int)(GPS_Data.lon_f / 100) + (GPS_Data.lon_f - ((int)GPS_Data.lon_f / 100) * 100) / 60 + (GPS_Data.lon_f - (int)GPS_Data.lon_f) / 600000;
		GPS_Data.lat_real = (int)(GPS_Data.lat_f / 100) + (GPS_Data.lat_f - ((int)GPS_Data.lat_f / 100) * 100) / 60 + (GPS_Data.lat_f - (int)GPS_Data.lat_f) / 600000;
	}
	else
		GPS_Data.speed = 0;

	if(GPS_Data.home_point_flag&&!GPS_Data.no_locate_flag)
	{
		float dist_temp;
		//R*arcos[cos(Y1)*cos(Y2)*cos(X1-X2)+sin(Y1)*sin(Y2)]。
		dist_temp=6371000*acos(cos(GPS_Data.home_lat*DEG2RAD)*cos(GPS_Data.lat_real*DEG2RAD)*cos(GPS_Data.home_lon*DEG2RAD-GPS_Data.lon_real*DEG2RAD)+sin(GPS_Data.home_lat*DEG2RAD)*sin(GPS_Data.lat_real*DEG2RAD));
		GPS_Data.distance2home=sqrt((GPS_Data.height-GPS_Data.home_height)*(GPS_Data.height-GPS_Data.home_height)+ dist_temp*dist_temp);
	}
	// restart DMA
	
	DMA1_Channel3->CNDTR = 200;
	DMA1_Channel3->CCR |= 1; // enable dma

	USART3->SR;
	USART3->DR;
}


void GPS_Init(void)
{
	UART_GPS_Init(9600); // GPS
	GPS_DMA_Init();
}


void GPS_DMA_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_Initstructure;

	// 时钟
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel3_IRQn;  // 嵌套通道为DMA1_Channel4_IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级为 2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  // 响应优先级为 7
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // 通道中断使能
	NVIC_Init(&NVIC_InitStructure);

	/*开启DMA时钟*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Channel3);
	/*DMA配置*/
	DMA_Initstructure.DMA_PeripheralBaseAddr = (u32)(&USART3->DR);
	DMA_Initstructure.DMA_MemoryBaseAddr = (u32)USART_RX_BUF;
	DMA_Initstructure.DMA_DIR = DMA_DIR_PeripheralSRC; // dev to mem
	DMA_Initstructure.DMA_BufferSize = 200;
	DMA_Initstructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Initstructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Initstructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_Initstructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_Initstructure.DMA_Mode = DMA_Mode_Normal;
	DMA_Initstructure.DMA_Priority = DMA_Priority_High;
	DMA_Initstructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel3, &DMA_Initstructure);

	// 开启DMA发送发成中断
	USART_Cmd(USART3, DISABLE);
	DMA_ClearFlag(DMA1_FLAG_TC5);
	DMA_ClearFlag(DMA1_FLAG_HT5);

	DMA_Cmd(DMA1_Channel3, ENABLE);
	USART_DMACmd(USART3, USART_DMAReq_Rx, ENABLE);
	USART_Cmd(USART3, ENABLE);
}
