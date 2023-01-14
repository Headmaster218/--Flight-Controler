#include <iic.h>
#define RCC_I2C1 RCC_APB2Periph_GPIOB
#define I2C1_PORT GPIOB
#define I2C1_Pin_SCL GPIO_Pin_8
#define I2C1_Pin_SDA GPIO_Pin_9

u8 I2C_RX_BUF[20], I2C_TX_BUF[20];

void Soft_IIC1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_I2C1 | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = I2C1_Pin_SCL | I2C1_Pin_SDA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(I2C1_PORT, &GPIO_InitStructure);

	I2C_DeInit(I2C1);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_16_9;
	I2C_InitStructure.I2C_ClockSpeed = 300000; // 400,000 / 8
	I2C_InitStructure.I2C_OwnAddress1 = 0x01;
	I2C_Init(I2C1, &I2C_InitStructure);
	I2C_Cmd(I2C1, ENABLE);
	I2C_GenerateSTOP(I2C1, ENABLE);
	I2C_GenerateSTART(I2C1, ENABLE);
	// IIC2_DMA_Init();
}

void IIC2_DMA_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	DMA_InitTypeDef DMA_Initstructure;

	// ʱ��
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;  // Ƕ��ͨ��ΪDMA1_Channel4_IRQn
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // ��ռ���ȼ�Ϊ 2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  // ��Ӧ���ȼ�Ϊ 7
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // ͨ���ж�ʹ��
	NVIC_Init(&NVIC_InitStructure);

	/*����DMAʱ��*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Channel6);
	DMA_DeInit(DMA1_Channel7);
	/*DMA����*/													 // Tx
	DMA_Initstructure.DMA_PeripheralBaseAddr = (u32)(&I2C1->DR); // �����ַ
	DMA_Initstructure.DMA_MemoryBaseAddr = (u32)I2C_TX_BUF;		 // �洢����ַ
	DMA_Initstructure.DMA_DIR = DMA_DIR_PeripheralDST;			 // mem to dev
	DMA_Initstructure.DMA_BufferSize = 20;
	DMA_Initstructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; // �����ַ����
	DMA_Initstructure.DMA_MemoryInc = DMA_MemoryInc_Enable;			 // mem pointer ++
	DMA_Initstructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_Initstructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_Initstructure.DMA_Mode = DMA_Mode_Normal;
	DMA_Initstructure.DMA_Priority = DMA_Priority_High;
	DMA_Initstructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel6, &DMA_Initstructure);
	// Rx
	DMA_Initstructure.DMA_PeripheralBaseAddr = (u32)(&I2C1->DR);
	DMA_Initstructure.DMA_MemoryBaseAddr = (u32)I2C_TX_BUF;
	DMA_Initstructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_Initstructure.DMA_BufferSize = 20;
	DMA_Initstructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_Initstructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Initstructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_Initstructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_Initstructure.DMA_Mode = DMA_Mode_Normal;
	DMA_Initstructure.DMA_Priority = DMA_Priority_High;
	DMA_Initstructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel7, &DMA_Initstructure);

	// DMA_Cmd(DMA1_Channel6,ENABLE);
	// DMA_Cmd(DMA1_Channel7,ENABLE);
	I2C_DMACmd(I2C1, ENABLE);
}

// 0 send, 1 receive
// ���ֽ�д��
int I2C1_Soft_Single_Write(u8 SlaveAddress, u8 REG_Address, u8 REG_data)
{
	
	return I2C1_Soft_Mult_Write(SlaveAddress, REG_Address, &REG_data, 1);
}

int I2C1_Soft_Mult_Write(u8 SlaveAddress, u8 REG_Address, u8 *ptChar, u8 size)
{
	u32 delay = 0;
	u8 i = 0;
	I2C_GenerateSTART(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if (delay > TIME_OUT)
			return ERROR;
		delay++;
	}
	delay = 0;
	I2C_Send7bitAddress(I2C1, SlaveAddress << 1, I2C_Direction_Transmitter);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (delay > TIME_OUT)
			return ERROR;
		delay++;
	}
	delay = 0;
	I2C_SendData(I2C1, REG_Address);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if (delay > TIME_OUT)
			return ERROR;
		delay++;
	}
	delay = 0;

	for (i = 0; i < size; i++)
	{
		I2C_SendData(I2C1, ptChar[i]);
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if (delay > TIME_OUT)
				return ERROR;
			delay++;
		}
		delay = 0;
	}
	I2C_GenerateSTOP(I2C1, ENABLE);
	return SUCCESS;
}

// ���ֽڶ�ȡ
int I2C1_Soft_Single_Read(u8 SlaveAddress, u8 REG_Address)
{
	u8 back;
	I2C1_Soft_Mult_Read(SlaveAddress, REG_Address, &back, 1);
	return back;
}

// ���ֽڶ�ȡ
int I2C1_Soft_Mult_Read(u8 SlaveAddress, u8 REG_Address, u8 *ptChar, u8 size)
{
	uint32_t delay = 0;
	u8 i = 0;
	I2C_GenerateSTART(I2C1, ENABLE); // ��ʼ�ź�
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if (delay > TIME_OUT)
			return ERROR;
		delay++;
	}
	delay = 0;
	I2C_Send7bitAddress(I2C1, SlaveAddress << 1, I2C_Direction_Transmitter); // �����豸��ַ+д�ź�
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if (delay > TIME_OUT)
			return ERROR;
		delay++;
	}
	delay = 0;
	I2C_SendData(I2C1, REG_Address);								  // ���ʹ洢��Ԫ��ַ����0��ʼ
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTING)) //////////////////////////ed,ing
	{
		if (delay > TIME_OUT)
			return ERROR;
		delay++;
	}
	delay = 0;
	if (size != 1)
		I2C_AcknowledgeConfig(I2C1, ENABLE);

	I2C_GenerateSTART(I2C1, ENABLE); // ��ʼ�ź�
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if (delay > TIME_OUT)
			return ERROR;
		delay++;
	}
	I2C1->SR1;
	delay = 0;
	I2C_Send7bitAddress(I2C1, SlaveAddress << 1, I2C_Direction_Receiver); // �����豸��ַ+���ź�
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
		if (delay > TIME_OUT)
			return ERROR;
		delay++;
	}
	I2C1->SR1;
	I2C1->SR2;
	delay = 0;

	for (i = 1; i < size; i++)
	{
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
			; // EV7
		*ptChar++ = I2C_ReceiveData(I2C1);
	}
	if (size != 1)
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
			; // EV7_1
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_GenerateSTOP(I2C1, size == 1 ? DISABLE : ENABLE);
	*ptChar = I2C_ReceiveData(I2C1);

	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
		; // EV7
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	return SUCCESS;
}
