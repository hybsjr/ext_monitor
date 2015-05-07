/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/main.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */  

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "uart.h"
#include "common.h"
#include "protocol.h"
#include "flash.h"
#include "ds1302.h"
#include "adc.h"
#include "time.h"
#include "stm32f10x_it.h"
#include "modem.h"
#include <string.h>


__IO uint32_t LsiFreq = 40000;
void iwdg_init(void)
{
	RCC->CSR |= 0x1;
	do{
	}while((RCC->CSR & 0x2) == 0);
	
	/* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
     dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_256);

  /* Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(LsiFreq/128);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}
// LED run
void run_led(void)
{
	unsigned int cnt = get_time6_cnt();
	
	if(cnt&0x1){
		GPIO_SetBits(GPIOB, GPIO_Pin_8);
	}else{		
		GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	}
}

// LED init
void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

  /* Configure PD0 and PD2 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, GPIO_Pin_8);
}
// init flash flag
unsigned int flash_empty_flag = 0;
char * logo = "hello  huyubin!\r\n";
// MAIN
int main(void)
{	
	led_init();		// init led
	gpio_init();
	Initial_DS1302();		 // init ds1302 rtc
  uart_init();				 // init uart
	flash_read(&flash_empty_flag, 1);	 // 判断flash是否已经写入数据
	if(flash_empty_flag == 0xffffffff){
		write_to_flash();		// 空FALSH,写入初始化数据
	}else{
		read_from_flash();	// 读取已经存储在FLASH中的数据
	}
	adc_init();		 // 初始化ADC, 使用DMA方式转换
	time6_init();
	modem_power_init();
	my_putdata((unsigned char *)logo, strlen(logo));
	//iwdg_init();
  /* Infinite loop */
  while (1)
  {
		run_led();
		recv_local_omc();//处理PC发送下来的数据
		local_deal();	// 解析本地数据数据
		check_alarm();	// 检查是否有告警发生
		check_report();
		recv_modem();	 // 接收MODEM数据
		modem_state_machine();	// MODEM 状态机
		/* Reload IWDG counter */
    IWDG_ReloadCounter();
  }
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
