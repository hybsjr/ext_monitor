#ifndef __USER_UART_H
#define __USER_UART_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

#define TxBufferSize1   128
#define TxBufferSize2   128
#define TxBufferSize3   128
#define RxBufferSize1   128
#define RxBufferSize2   256
#define RxBufferSize3   128

extern struct queue_buf uart1;
extern struct queue_buf uart2;
extern struct queue_buf uart3;

void uart_init(void);
// ≤È—Ø∑¢ÀÕ
void uart_send(USART_TypeDef * USARTx, unsigned char * buf, int len);
void my_putchar(unsigned char data);
void my_putstr(const char *pstr );
void my_putdata(const unsigned char *pstr, int len);
#ifdef __cplusplus
}
#endif

#endif 


