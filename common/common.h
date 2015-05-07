#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


#ifndef NULL
	#define NULL 0
#endif

// 当时间unknown超过known时，返回真
#define time_after(unknown, known) ((int)(known) - (int)(unknown) < 0)
// 当时间unknown没超过known时，返回真
#define time_before(unknown, known) ((int)(unknown) - (int)(known) < 0)
#define time_after_eq(unknown, known) ((int)(unknown) - (int)(known) >= 0)
#define time_before_eq(unknown, known) ((int)(known) - (int)(unknown) >= 0)

/*********************** 缓冲区队列列操作 **************************/
// 循环队列缓冲区结构
struct queue_buf{
	int len;				// 申请的循环队列缓冲区长度
	volatile int head;		// 缓冲区头指针
	volatile int tail;		// 缓冲区尾指针
	char * buf;    			// 缓冲区
};
/*
** 函数功能：初始化循环队列缓冲区
** 输入参数：queue_buf=欲初始化的循环队列结构体指针 len=循环队列长度
** 输出参数：无
** 返回值：0=成功 -1=失败
** 备注：len必须为128/256/512/1024/2048/4096
*/
int queue_buf_init(struct queue_buf * queue_buf, char * buf, unsigned int len);

/*
** 函数功能：删除循环队列缓冲区
** 输入参数：queue_buf=欲删除的循环队列结构体指针 
** 输出参数：无
** 返回值：0=成功 -1=失败
*/
void * queue_buf_exit(void * arg);

/*
** 函数功能：获取循环队列缓冲区有效数据长度
** 输入参数: queue_buf=循环队列结构体指针
** 输出参数：无
** 返回值：有效数据长度
*/
int get_queue_buf_len(struct queue_buf * queue_buf);

/*
** 函数功能：循环队列缓冲区入桟
** 输入参数: queue_buf=循环队列结构体指针 buf=待入桟缓冲区指针 len=待入桟数据长度
** 输出参数：无
** 返回值：0=成功 -1=失败
*/
int push_queue_buf(struct queue_buf * queue_buf, char * buf, unsigned int len);

/*
** 函数功能：循环队列缓冲区出桟
** 输入参数: queue_buf=循环队列结构体指针 buf=待出桟缓冲区指针 len=待出桟数据长度
** 输出参数：无
** 返回值：0=成功 -1=失败
*/
int pop_queue_buf(struct queue_buf * queue_buf, char * buf, unsigned int len);

/*
** 函数功能：将HEX转为unsigned short int 型，倒序排列
** 输入参数: p=hex数据指针
** 输出参数：无
** 返回值：转换值
*/
unsigned short int hex2uint16(unsigned char * p);

/*
** 函数功能：将HEX转为ASCII
** 输入参数: p=hex数据 len=数据长度
** 输出参数：out=ascii数据
** 返回值：转换结果长度
*/
int hex2ascii(unsigned char * p, int len, unsigned char * out);
int hex2ascii_ex(unsigned char * p, int len);

// ascii转为HEX
unsigned char _ascii2hex(unsigned char p);
/*
** 函数功能：将ASCII转为HEX
** 输入参数: p=ascii数据 len=数据长度
** 输出参数：out=hex数据
** 返回值：转换结果长度
*/
int ascii2hex(unsigned char * p, int len, unsigned char * out);
int ascii2hex_ex(unsigned char * p, int len);
/*
** 函数功能：将单字节有符号数转为4字节有符号数
** 输入参数: val=单字节有符号数
** 输出参数：无
** 返回值：4字节有符号数
*/
int signed_1to4(char val);

/*
** 函数功能：将2字节有符号数转为4字节有符号数
** 输入参数: val=单字节有符号数
** 输出参数：无
** 返回值：4字节有符号数
*/
int signed_2to4(short val);

// 字节交换
void swap(char *p, char *q);

// 数组左移
// str=数组指针  len=数组有效数据长度  cnt=左移长度
void shift_left(char * str, int len, int cnt);

// 计算CRC
unsigned short crc16(unsigned char *ptr, unsigned short len);

void my_printf(const char * format, ...);

int float2int(float f);

#ifdef __cplusplus
}
#endif

#endif 


