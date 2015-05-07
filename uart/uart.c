/**
  ******************************************************************************
  * @file    USART/Interrupt/main.c 
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
#include "uart.h"
#include "common.h"
#include <string.h>
#include "stm32f10x.h"


/** @addtogroup STM32F10x_StdPeriph_Examples
  * @{
  */

/** @addtogroup USART_Interrupt
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
USART_InitTypeDef USART_InitStructure;
uint8_t TxBuffer1[TxBufferSize1];
uint8_t TxBuffer2[TxBufferSize2];
uint8_t TxBuffer3[TxBufferSize3];
uint8_t RxBuffer1[RxBufferSize1];
uint8_t RxBuffer2[RxBufferSize2];
//uint8_t RxBuffer3[RxBufferSize3] = {0x7E, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x01, 0x03, 0xFF, 0x05, 0x09, 0x00, 0x01, 0x01, 0x12, 0x34, 0x7E, 0x7E, 0x03};
uint8_t RxBuffer3[RxBufferSize3];

struct queue_buf uart1;
struct queue_buf uart2;
struct queue_buf uart3;
/* Private function prototypes -----------------------------------------------*/
void RCC_Configuration(void);
void GPIO_Configuration(void);
void NVIC_Configuration(void);

/* Private functions ---------------------------------------------------------*/

void uart_buf_init(void)
{
	memset(RxBuffer1, 0, RxBufferSize1);
	memset(RxBuffer2, 0, RxBufferSize2);
	memset(RxBuffer3, 0, RxBufferSize3);
	queue_buf_init(&uart1, (char *)RxBuffer1, RxBufferSize1);
	queue_buf_init(&uart2, (char *)RxBuffer2, RxBufferSize2);
	queue_buf_init(&uart3, (char *)RxBuffer3, RxBufferSize3);
}
/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
void uart_init(void)
{
  uart_buf_init();
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
       
  /* System Clocks Configuration */
  RCC_Configuration();
       
  /* NVIC configuration */
  NVIC_Configuration();

  /* Configure the GPIO ports */
  GPIO_Configuration();

/* USARTy and USARTz configuration ------------------------------------------------------*/
  /* USARTy and USARTz configured as follow:
        - BaudRate = 9600 baud  
        - Word Length = 8 Bits
        - One Stop Bit
        - No parity
        - Hardware flow control disabled (RTS and CTS signals)
        - Receive and transmit enabled
  */
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  /* Configure USART1 */
  USART_Init(USART1, &USART_InitStructure);
  /* Configure USART2 */
  USART_Init(USART2, &USART_InitStructure);
  /* Configure USART3 */
  USART_Init(USART3, &USART_InitStructure);
  
  /* Enable USART1 Receive and Transmit interrupts */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  //USART_ITConfig(USART1, USART_IT_TXE, ENABLE);

  /* Enable USART2 Receive and Transmit interrupts */
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
  //USART_ITConfig(USART2, USART_IT_TXE, ENABLE);

  /* Enable USART3 Receive and Transmit interrupts */
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
  //USART_ITConfig(USART3, USART_IT_TXE, ENABLE);

  /* Enable the USART1 */
  USART_Cmd(USART1, ENABLE);
  /* Enable the USART2 */
  USART_Cmd(USART2, ENABLE);
  /* Enable the USART3 */
  USART_Cmd(USART3, ENABLE);
}

/**
  * @brief  Configures the different system clocks.
  * @param  None
  * @retval None
  */
void RCC_Configuration(void)
{   
  /* Enable GPIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);


  /* Enable USART1 Clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); 

  /* Enable USART2 Clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);  

  /* Enable USART3 Clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
}

/**
  * @brief  Configures the different GPIO ports.
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;


  /* Configure USART1 Rx as input floating */	// ISP uart1
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure USART1 Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);


  /* Configure USART2 Rx as input floating */  // modem uart2
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  /* Configure USART2 Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure); 
  
  /* Configure USART3 Rx as input floating */  // pc uart3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  /* Configure USART3 Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
}

/**
  * @brief  Configures the nested vectored interrupt controller.
  * @param  None
  * @retval None
  */
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USART1 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the USART2 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Enable the USART3 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
// ²éÑ¯·¢ËÍ
void uart_send(USART_TypeDef * USARTx, unsigned char * buf, int len)
{
	int i = 0;
	unsigned short tmp;

	for(i = 0; i < len; i++){
		/* Send one byte from USARTy to USARTz */
			tmp = buf[i];
	    USART_SendData(USARTx, tmp);
		/* Loop until USARTy DR register is empty */ 
	    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
	    {
	    }
	}
	#ifdef DEBUG
	my_putdata((unsigned char *)buf, len);
	#endif
}

void my_putchar(unsigned char data)
{
	USART_SendData(USART3, data);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET)
	{
	}	
}
void my_putstr(const char *pstr )
{
	const char * ptr = pstr;

	while ( *ptr )
	{
		my_putchar((unsigned char)*ptr++ );
	}
}
void my_putdata(const unsigned char *pstr, int len)
{
	int i = 0;
	
	for(i = 0; i < len; i++){
		my_putchar((unsigned char)pstr[i]);
	}
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
