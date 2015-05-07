#include "common.h"
#include "uart.h"

/*
** 函数功能：初始化循环队列缓冲区
** 输入参数：queue_buf=欲初始化的循环队列结构体指针 buf=待初始化缓冲区指针 len=循环队列长度
** 输出参数：无
** 返回值：0=成功 -1=失败
** 备注：len必须为128/256/512/1024/2048/4096
*/
int queue_buf_init(struct queue_buf * queue_buf, char * buf, unsigned int len)
{
	int i = 0;
	int flag = 0;

	// 判断缓冲区大小
	for(i = 32; i > 0 ; i--){
		if(flag == 0){
			if( len&(1<<(i-1)) ){
				flag = 1;
			}
		}else{
			if( len & (1<<(i-1)) ){
				//printf("struct queue_buf init len is error.%x\r\n", len);
				return -1;
			}
		}
	}
	// 判断缓冲区是否为0
	if(len == 0){
		//printf("struct queue_buf init error. len = 0.\r\n");
		return -1;
	}
	// 初始化缓冲区
	queue_buf->len = len;
	queue_buf->head = 0;
	queue_buf->tail = 0;
	queue_buf->buf = buf;
	return 0;
}
/*
** 函数功能：删除循环队列缓冲区
** 输入参数：queue_buf=欲删除的循环队列结构体指针 
** 输出参数：无
** 返回值：0=成功 -1=失败
*/
void * queue_buf_exit(void * arg)
{
	struct queue_buf * queue_buf = (struct queue_buf *)arg;
	// 判断循环队列和缓冲区是否为空
	if((queue_buf != NULL)&&(queue_buf->buf != NULL)){
		// 释放缓冲区资源
		queue_buf->len = 0;
		queue_buf->head = 0;
		queue_buf->tail = 0;
		queue_buf->buf = NULL;
		//printf("free queue_buf ok.\r\n");
		return (void *)0;
	}
	//printf("free error.\r\n");
	return (void *)-1;
}
/*
** 函数功能：获取循环队列缓冲区有效数据长度
** 输入参数: queue_buf=循环队列结构体指针
** 输出参数：无
** 返回值：有效数据长度
*/
int get_queue_buf_len(struct queue_buf * queue_buf)
{
	return (((queue_buf->head-queue_buf->tail)+queue_buf->len)&(queue_buf->len-1));
}
/*
** 函数功能：循环队列缓冲区入桟
** 输入参数: queue_buf=循环队列结构体指针 buf=待入桟缓冲区指针 len=待入桟数据长度
** 输出参数：无
** 返回值：0=成功 -1=失败
*/
int push_queue_buf(struct queue_buf * queue_buf, char * buf, unsigned int len)
{
	unsigned int i = 0;
	unsigned int cnt = 0;

	// 检查缓冲区是否有足够的空间
	cnt = get_queue_buf_len(queue_buf);
	if((len+cnt) > (queue_buf->len-1)){
		//printf("缓冲区空间不足，入桟失败.\r\n");
		return -1;
	}
	// 将数据存入缓冲区，并更改下标
	for(i = 0; i < len; i++){
		queue_buf->buf[queue_buf->head++] = buf[i];
		queue_buf->head &= (queue_buf->len - 1);
	}
	return 0;
}
/*
** 函数功能：循环队列缓冲区出桟
** 输入参数: queue_buf=循环队列结构体指针 buf=待出桟缓冲区指针 len=待出桟数据长度
** 输出参数：无
** 返回值：0=成功 -1=失败
*/
int pop_queue_buf(struct queue_buf * queue_buf, char * buf, unsigned int len)
{
	unsigned int i = 0;
	unsigned int cnt = 0;

	// 检查缓冲区有效数据长度
	cnt = get_queue_buf_len(queue_buf);
	if(len > cnt){
		//printf("缓冲区有效数据太少，len=%d, cnt=%d.\r\n", len, cnt);
		return -1;
	}
	// 将缓冲区中的有效数据取出，存入buf中
	for(i = 0; i < len; i++){
		buf[i] = queue_buf->buf[queue_buf->tail++];
		queue_buf->tail &= (queue_buf->len-1);
	}
	return 0;
}
/*
** 函数功能：将HEX转为unsigned short int 型，倒序排列
** 输入参数: p=hex数据指针
** 输出参数：无
** 返回值：转换值
*/
unsigned short int hex2uint16(unsigned char * p)
{
	return ((unsigned short int)p[0] | (unsigned short int)(p[1]<<8));
}
/*
** 函数功能：将HEX转为ASCII
** 输入参数: p=hex数据 len=数据长度
** 输出参数：out=ascii数据
** 返回值：转换结果长度
*/
const char ascii_tb[] = "0123456789ABCDEF";
int hex2ascii(unsigned char * p, int len, unsigned char * out)
{
	int i = 0;
	int cnt = 0;
	
	for( i = 0; i < len; i++){
		out[cnt++] = ascii_tb[((p[i]>>4)&0x0f)];
		out[cnt++] = ascii_tb[((p[i])&0x0f)];
	}
	return cnt;
}
int hex2ascii_ex(unsigned char * p, int len)
{
	int i = 0;
	int cnt = len*2 - 1;
	
	for( i = len-1; i >= 0; i--){
		p[cnt--] = ascii_tb[((p[i])&0x0f)]; 
		p[cnt--] = ascii_tb[((p[i]>>4)&0x0f)];
	}
	return len*2;
}
// ascii转为HEX
unsigned char _ascii2hex(unsigned char p)
{
	unsigned char tmp = 0;

	if((p >= '0')&&(p <= '9')){
		tmp = p - '0';
	}else if((p >= 'a')&&(p <= 'f')){
		tmp = p - 'a' + 10;
	}else if((p >= 'A')&&(p <= 'F')){
		tmp = p - 'A' + 10;
	}
	return tmp;
}
/*
** 函数功能：将ASCII转为HEX
** 输入参数: p=ascii数据 len=数据长度
** 输出参数：out=hex数据
** 返回值：转换结果长度
*/
int ascii2hex(unsigned char * p, int len, unsigned char * out)
{
	int i = 0;
	int cnt = 0;
	
	for( i = 0; i < len; i += 2){
		out[cnt++] = (_ascii2hex(p[i])<<4) | _ascii2hex(p[i+1]);
	}
	return cnt;
}
int ascii2hex_ex(unsigned char * p, int len)
{
	int i = 0;
	int cnt = 0;
	
	for( i = 0; i < len; i += 2){
		p[cnt++] = (_ascii2hex(p[i])<<4) | _ascii2hex(p[i+1]);
	}
	return cnt;
}
/*
** 函数功能：将单字节有符号数转为4字节有符号数
** 输入参数: val=单字节有符号数
** 输出参数：无
** 返回值：4字节有符号数
*/
int signed_1to4(char val)
{
	int cnt = val;
	
	if(val&0x80){
		val = ~val;
		val = val+1;
		cnt = 0-val;
	}
	return cnt;
}
/*
** 函数功能：将2字节有符号数转为4字节有符号数
** 输入参数: val=单字节有符号数
** 输出参数：无
** 返回值：4字节有符号数
*/
int signed_2to4(short val)
{
	int cnt = val;
	
	if(val&0x8000){
		val = ~val;
		val = val+1;
		cnt = 0-val;
	}
	return cnt;
}

// 字节交换
void swap(char *p, char *q)
{
	char tmp;

	tmp = *p;
	*p = *q;
	*q = tmp;
}

// 数组左移
// str=数组指针  len=数组有效数据长度  cnt=左移长度
void shift_left(char * str, int len, int cnt)
{
	int i, j;
	
	if(cnt > 0){
		for(i = 0, j = cnt; i < len; i++, j++){
			swap(&str[i], &str[j]);
		}
	}
}
// 计算crc
const unsigned int crc_ta[256]={ /* CRC 余式表 */
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
	};
unsigned short crc16(unsigned char *ptr, unsigned short len) 
{
	unsigned int crc;
	unsigned char da;	
	crc=0;
	while(len--!=0) 
	{
		da=(unsigned char) (crc/256); /* 以8 位二进制数的形式暂存CRC 的高8 位 */
		crc<<=8; /* 左移8 位，相当于CRC 的低8 位乘以8个2 */
		crc^=crc_ta[da^*ptr]; /* 高8 位和当前字节相加后再查表求CRC ，再加上以前的CRC */
		ptr++;
	}
	return(crc);
}

typedef char * uart_va_list; 
/* Private define ------------------------------------------------------------*/
#define uart_sizeof_int(n)   		((sizeof(n) +  sizeof(int) - 1) & ~(sizeof(int) - 1))
#define uart_va_start(ap,v)  		(ap = (uart_va_list)&v +uart_sizeof_int(v)) 
#define uart_va_arg(ap,t)    		(*(t *)((ap += uart_sizeof_int(t)) - uart_sizeof_int(t))) 
#define uart_va_end(ap)      		(ap = (uart_va_list)0) 
const unsigned char table[] = "0123456789ABCDEF";

/*********************************************************************************************************
;** 函数名称: my_itoa
;** 功能描述: 将整型数按10进制或16进制转换成字符串，存入pstr.
;** 		  注意：pstr指向的内存有足够的空间，否则内存操作溢出。
;** 参    数: value:整型数 pstr：字符串存储空间，函数的返回值  radix: 进制;
;**  
;** 返 回 值: char *pstr
;**         
;** 作　  者: 胡玉斌
;** 日  　期: 2009年09月22日
;**-------------------------------------------------------------------------------------------------------
;** 修 改 人: 
;** 日　  期: 
;**------------------------------------------------------------------------------------------------------
;********************************************************************************************************/
char * my_itoa( int value, char * pstr, unsigned char radix )
{
	int    i, d;
    unsigned char     flag = 0;
    char      *ptr = pstr;
    /* This implementation  works for decimal numbers and hex numbers. */
    if ( (radix != 10) && (radix != 16) )
    {
        *ptr = 0;
        return pstr;
    }

    if ( value == 0 )
    {
        *ptr++ = 0x30;
        *ptr = 0;
        return pstr;
    }

	if ( radix == 10 )
	{
	    /* if this is a negative value insert the minus sign. */
	    if (value < 0)
	    {
	        *ptr++ = '-';
	        /* Make the value positive. */
	        value *= -1;
	    }
	    for (i = 1000000000; i > 0; i /= 10)
	    {
	        d = value / i;
	        if (d || flag)
	        {
	            *ptr++ = (char)(d + 0x30);
	            value -= (d * i);
	            flag = 1;
	        }
	    }
	}
	else
	{
		for ( i = 7; i >= 0; i-- )
		{
			d = ( value >> ( i * 4 ) ) & 0x0f;
			if ( d || flag )
			{
				*ptr++ = table[d];
				flag = 1;
			}
		}
	}
    /* Null terminate the string. */
    *ptr = 0;
    return pstr;
}
/*
unsigned int my_atoi( char * pstr, unsigned char radix )
{
	unsigned char c = 0;
	unsigned int total = 0;

	while( c = *pstr++ )
	{
		if ( (c >= '0') && (c <= '9') )
		{
			c -= '0';
		}
		else if ( (c >= 'a') && (c <= 'f') ) 
		{
			c = c - 'a' + 10;
		}
		else if ( (c >= 'A') && (c <= 'F') ) 
		{
			c = c - 'A' + 10;
		}
		total = total*radix + c;			
	}
	return total;	
}
*/
/*********************************************************************************************************
;** 函数名称: my_printf
;** 功能描述: 以查询的方式向串口发送字符串
;** 
;** 参    数: format 串口;
;**  
;** 返 回 值: 无
;**         
;** 作　  者: 胡玉斌
;** 日  　期: 2009年09月22日
;**-------------------------------------------------------------------------------------------------------
;** 修 改 人: 
;** 日　  期: 
;**------------------------------------------------------------------------------------------------------
;********************************************************************************************************/

void my_printf(const char * format, ...)
{
	char *			s = 0;
	int 			d = 0;		
	char			ch16[11] = {0};
	char			ch10[11] = {0};
	uart_va_list 	ap = 0;
	
	uart_va_start(ap, format);
	
	while (* format) 
	{
		if (* format != '%') 
		{
			my_putchar((unsigned char)*(format++) );
			continue;
		}
		switch (*(++format)) 
		{
			case 's':
			case 'S':				
				s = uart_va_arg(ap, char *);
				my_putstr(s);
				break;
			case 'c':
			case 'C':				
				my_putchar((unsigned char)uart_va_arg(ap, char) );
				break;	
			case 'd':
				d = uart_va_arg(ap, int);
				my_putstr(my_itoa(d, ch10, 10) );
				break;
			case 'D':				
				d = uart_va_arg(ap, int);
				my_putstr(my_itoa(d, ch16, 16) );
				break;
			default:
				my_putchar((unsigned char)*(format) );
				break;
		}
		format++;
	}
	uart_va_end(ap);	
}

// float->int
int float2int(float f)
{
	int tmp = 0;
	
	tmp = f;
	if(((float)tmp+0.5) > f){
		return tmp;
	}
	return tmp+1;
}
