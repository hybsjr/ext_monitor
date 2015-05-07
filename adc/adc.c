
#include "stm32f10x.h"
#include "adc.h"

#define ADC1_DR_Address    ((uint32_t)0x4001244C)
#define BufferLenght       8
__IO uint16_t ADC1ConvertedValue[BufferLenght];

void adc_init(void)
{
	DMA_InitTypeDef   DMA_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 72M/6=12M
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_ADC1, ENABLE);
	
	// adc gpio configure   PA5、PA6、PA7、PC0、PC1、PC2、PC4、PC5
	//                      IN5  IN6  IN7  IN10 IN11 IN12
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	/* DMA1 channel1 configuration ---------------------------------------------*/
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC1ConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = BufferLenght;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	/* Enable DMA1 channel1 */
  DMA_Cmd(DMA1_Channel1, ENABLE);

	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = BufferLenght;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channel5/6/7/10/11/12 configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 3, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 4, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 5, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 6, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 7, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_15, 8, ADC_SampleTime_239Cycles5);
	
	/* Enable ADC1 DMA */
  ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);
	
	/* Enable TempSensor and Vrefint channels: channel16 and Channel17 */
	ADC_TempSensorVrefintCmd(ENABLE);
	
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	 
	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
	/* Test on Channel 1 DMA1_FLAG_TC flag */
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));
	
	/* Clear Channel 1 DMA1_FLAG_TC flag */
	DMA_ClearFlag(DMA1_FLAG_TC1);
}

int get_voltage(int index)
{
	int tmp = 0;
	int j = 0;
	int i = 0;
	int tmp_all = 0;
	
	for(i = 0; i < 10; i++){
		tmp = ADC1ConvertedValue[index]*3300/4096;
		tmp_all += tmp;
		for(j = 0; j < 0xfff; j++){
					__ASM("NOP");	
		}		
	}
	tmp = tmp_all/10;
	
	return tmp;
}
