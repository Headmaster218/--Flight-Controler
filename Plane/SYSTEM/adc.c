/*
 * @Author: Headmaster1615  e-mail:hm-218@qq.com
 * @Date: 2022-12-31 12:43:50
 * @LastEditors: error: git config user.name && git config user.email & please set dead value or install git
 * @LastEditTime: 2023-01-03 13:30:54
 * @FilePath: \USERd:\STM32\My Project\Flight Controller\SYSTEM\adc.c
 * @Description:
 *
 * Copyright (c) 2022 by Headmaster1615, All Rights Reserved.
 */
#include "adc.h"
#include "delay.h"

// ��ʼ��ADC
// �������ǽ��Թ���ͨ��Ϊ��
// ����Ĭ�Ͻ�����ͨ��0~3
void Adc_Init(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE); // ʹ��ADC1ͨ��ʱ��

	RCC_ADCCLKConfig(RCC_PCLK2_Div6); // ����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	// PA1 ��Ϊģ��ͨ����������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // ģ����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_DeInit(ADC1); // ��λADC1

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;					// ADC����ģʽ:ADC1��ADC1�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;						// ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;					// ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;				// ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 4;								// ˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);									// ����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���

	// �������ͨ�����������ʱ��

	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_239Cycles5);

	ADC_DMACmd(ADC1, ENABLE); // ADC�����DMA����

	ADC_Cmd(ADC1, ENABLE); // ʹ��ADC

	ADC_ResetCalibration(ADC1); // ��λADCУ׼�Ĵ���
	while (ADC_GetResetCalibrationStatus(ADC1))
		; // �ȴ�У׼�Ĵ�����λ���

	ADC_StartCalibration(ADC1); // ��ʼADCУ׼
	while (ADC_GetCalibrationStatus(ADC1))
		; // �ȴ�У׼���

	ADC_SoftwareStartConvCmd(ADC1, ENABLE); // ע�͵��������ADת��
	//ADC_ExternalTrigConvCmd(ADC1, ENABLE);	// ʹ���ⲿ��ʱ������
}

// DMA����
u16 ADC_DMA_Value[4]; // DMA������
void ADC_DMA_Init(void)
{

	DMA_InitTypeDef DMA_InitStruct;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	// DMA init;  Using DMA1 channel 1
	DMA_DeInit(DMA1_Channel1);												 // ��λDMA1�ĵ�1ͨ��
	DMA_InitStruct.DMA_PeripheralBaseAddr = (u32)&ADC1->DR;					 // DMA��Ӧ���������ַ
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; // ת�����16bits
	DMA_InitStruct.DMA_MemoryBaseAddr = (u32)ADC_DMA_Value;
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;					 // DMA��ת��ģʽ��SRCģʽ������to�ڴ�
	DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;						 // M2Mģʽ��ֹ��memory to memory
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; // DMA���˵�����16bits

	// ����һ�����ݺ�Ŀ���ڴ��ַ���ƣ������ɼ�������ݵ�
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	// ����һ�����ݺ��豸��ַ�Ƿ����
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;

	// ת��ģʽ������ѭ������ģʽ�����M2M�����ˣ������ģʽʧЧ
	// ��һ����Normalģʽ����ѭ������һ��DMA
	 DMA_InitStruct.DMA_Mode  = DMA_Mode_Circular;
	//DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;

	DMA_InitStruct.DMA_Priority = DMA_Priority_High; // DMA���ȼ�����
	DMA_InitStruct.DMA_BufferSize = 4;				 // DMA�����С
	DMA_Init(DMA1_Channel1, &DMA_InitStruct);

	// DMA_ClearITPendingBit(DMA1_IT_TC1);
	// DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);                //����DMA1CH1�ж�

	DMA_Cmd(DMA1_Channel1, ENABLE);
}

u16 adc_vol[4];
int tmpvol[4];
int nummm[4];
//��Ҫ10ms
u8 adc_times=0;
u16 Get_Adc(void)
{
	if(adc_times==10)
	{
		adc_times=0;
		adc_vol[0]=tmpvol[0]/10*3195/10000;
		adc_vol[1]=tmpvol[1]/10*nummm[1];
		tmpvol[0]=0;
		tmpvol[1]=0;
		tmpvol[2]=0;
		tmpvol[3]=0;
	}
	else
	{
		adc_times++;
		tmpvol[0]+=ADC_DMA_Value[0];
		tmpvol[1]+=ADC_DMA_Value[1];
		tmpvol[2]+=ADC_DMA_Value[2];
		tmpvol[3]+=ADC_DMA_Value[3];
	}
		return tmpvol[0]/10*3195/10000;
}


/*
// ���ADCֵ
// ch:ͨ��ֵ 0~3
u16 Get_Adc(u8 ch)
{
	// ����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5); // ADC1,ADCͨ��,����ʱ��Ϊ239.5����

	ADC_SoftwareStartConvCmd(ADC1, ENABLE); // ʹ��ָ����ADC1�����ת����������

	while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
		; // �ȴ�ת������

	return ADC_GetConversionValue(ADC1); // �������һ��ADC1�������ת�����
}*/
/*
float Get_Adc_Average(u8 ch, u8 times)
{
	float temp_val = 0;
	u8 t;
	for (t = 0; t < times; t++)
	{
		temp_val += Get_Adc(ch);
	}
	temp_val /= times;
	temp_val = temp_val / 4096 * 3.3;

	return temp_val;
}

float Get_Temperature(void)
{
	float temp;
	temp = Get_Adc_Average(9, 3);
	return 1.42 * temp * temp * temp - 9.299 * temp * temp + 39.23 * temp - 25.93;
}
*/