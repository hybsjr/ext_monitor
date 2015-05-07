/**************************************************************************
  
                     THE REAL TIMER DS1302 DRIVER LIB
  
               COPYRIGHT (c)   2005 BY JJJ.
                         --  ALL RIGHTS RESERVED  --
  
   File Name:       DS1302.h
   Author:          Jiang Jian Jun
   Created:         2003/7/21
   Modified:  NO
   Revision:   1.0
  
***************************************************************************/
#include "stm32f10x.h"
#include "ds1302.h"

GPIO_InitTypeDef GPIO_InitStructure;

//时钟线引脚
#define DS1302_SET_CLK GPIO_SetBits(GPIOC, GPIO_Pin_10)               
#define DS1302_CLR_CLK GPIO_ResetBits(GPIOC, GPIO_Pin_10)

//数据线引脚
#define DS1302_SET_IO GPIO_SetBits(GPIOC, GPIO_Pin_11)                
#define DS1302_CLR_IO GPIO_ResetBits(GPIOC, GPIO_Pin_11)
#define DS1302_READ_IO GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_11) 
#define DS1302_IO_IN    {\
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  \
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  \
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  \
        GPIO_Init(GPIOC, &GPIO_InitStructure); }
#define DS1302_IO_OUT   {\
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;  \
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  \
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  \
        GPIO_Init(GPIOC, &GPIO_InitStructure); }

//复位线引脚          
#define DS1302_SET_RST GPIO_SetBits(GPIOC, GPIO_Pin_12)                
#define DS1302_CLR_RST GPIO_ResetBits(GPIOC, GPIO_Pin_12)


#define AM(X) X
#define PM(X) (X+12)               // 转成24小时制
#define DS1302_SECOND 0x80
#define DS1302_MINUTE 0x82
#define DS1302_HOUR  0x84 
#define DS1302_WEEK  0x8A
#define DS1302_DAY  0x86
#define DS1302_MONTH 0x88
#define DS1302_YEAR  0x8C
#define DS1302_RAM(X) (0xC0+(X)*2)    //用于计算 DS1302_RAM 地址的宏

void DS1302InputByte(unsigned char d)  //实时时钟写入一字节(内部函数)
{ 
    unsigned char i;

    for(i = 0; i < 8; i++)
    {
        if((d>>i)&0x1){
          DS1302_SET_IO;
        }else{
          DS1302_CLR_IO;
        }
        DS1302_SET_CLK;
        DS1302_CLR_CLK;
    } 
}

unsigned char DS1302OutputByte(void)  //实时时钟读取一字节(内部函数)
{ 
    unsigned char i = 0;
    unsigned char d = 0;
    unsigned char j = 0;
    
    for(i = 0; i < 8; i++)
    {
        j= DS1302_READ_IO;
        d |= (j<<i);
        DS1302_SET_CLK;
        DS1302_CLR_CLK;
    } 
    return(d); 
}

//ucAddr: DS1302地址, ucData: 要写的数据
void Write1302(unsigned char ucAddr, unsigned char ucDa) 
{
    DS1302_CLR_RST;
    DS1302_CLR_CLK;
    DS1302_SET_RST;
    DS1302_IO_OUT;
    DS1302InputByte(ucAddr);        // 地址，命令 
    DS1302InputByte(ucDa);        // 写1Byte数据
    DS1302_SET_CLK;
    DS1302_CLR_RST;
}

unsigned char Read1302(unsigned char ucAddr) //读取DS1302某地址的数据
{
    unsigned char ucData;
    DS1302_CLR_RST;
    DS1302_CLR_CLK;
    DS1302_SET_RST;
    DS1302_IO_OUT;
    DS1302InputByte(ucAddr|0x01);        // 地址，命令
    DS1302_IO_IN; 
    ucData = DS1302OutputByte();         // 读1Byte数据
    DS1302_SET_CLK;
    DS1302_CLR_RST;
    return(ucData);
}

//是否写保护
void DS1302_SetProtect(unsigned char flag)        
{
 if(flag)
  Write1302(0x8E,0x10);
 else
  Write1302(0x8E,0x00);
}

void DS1302_GetTime(unsigned char *Time)
{
 Time[5] = Read1302(DS1302_SECOND);
 Time[4] = Read1302(DS1302_MINUTE);
 Time[3] = Read1302(DS1302_HOUR);
 Time[2] = Read1302(DS1302_DAY);
 Time[1] = Read1302(DS1302_MONTH);
 Time[0] = Read1302(DS1302_YEAR); 
}
void DS1302_SetTime(unsigned char *Time)
{

	DS1302_SetProtect(0); // 禁止写保护
	Write1302(DS1302_SECOND,Time[5]);
	Write1302(DS1302_MINUTE,Time[4]);
	Write1302(DS1302_HOUR,Time[3]);
	Write1302(DS1302_DAY,Time[2]);
	Write1302(DS1302_MONTH,Time[1]);
	Write1302(DS1302_YEAR,Time[0]);
	DS1302_SetProtect(1); 
}
void Initial_DS1302(void)
{
	// DS1302 引脚初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	/* Configure PC10、PC11 and PC12 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_10);
	GPIO_ResetBits(GPIOC, GPIO_Pin_11);
	GPIO_ResetBits(GPIOC, GPIO_Pin_12);
	
	
}

//往DS1302写入时钟数据(多字节方式)
void BurstWrite1302(unsigned char *pWClock) 
{
    unsigned char i;
    Write1302(0x8e,0x00);          // 控制命令,WP=0,写操作?
    DS1302_CLR_RST;
    DS1302_CLR_CLK;
    DS1302_SET_RST;
    DS1302_IO_OUT;
    DS1302InputByte(0xbe);         // 0xbe:时钟多字节写命令
    for (i = 8; i>0; i--)       //8Byte = 7Byte 时钟数据 + 1Byte 控制
    {
        DS1302InputByte(*pWClock);  // 写1Byte数据
        pWClock++;
    }
    DS1302_SET_CLK;
    DS1302_CLR_RST;
}

//读取DS1302时钟数据(时钟多字节方式)
void BurstRead1302(unsigned char *pRClock) 
{
    unsigned char i;
    DS1302_CLR_RST;
    DS1302_CLR_CLK;
    DS1302_SET_RST;
    DS1302_IO_OUT;
    DS1302InputByte(0xbf);              // 0xbf:时钟多字节读命令 
    for (i=8; i>0; i--) 
    {
       DS1302_IO_IN;
       *pRClock = DS1302OutputByte();   // 读1Byte数据 
       pRClock++;
    }
    DS1302_SET_CLK;
    DS1302_CLR_RST;
}

// 是否将时钟停止
void DS1302_TimeStop(unsigned char flag)           
{
 unsigned char Data;
 Data=Read1302(DS1302_SECOND);
 DS1302_SetProtect(0);
 if(flag)
  Write1302(DS1302_SECOND, Data|0x80);
 else
  Write1302(DS1302_SECOND, Data&0x7F);
}
