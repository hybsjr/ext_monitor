#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "common.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define MSG_AP_ERR			-1 //AP协议类型错误
#define MSG_CRC_ERR			-2 // CRC校验错误
#define MSG_VP_ERR			-3 // VP承载层错误
#define MSG_STATION_ERR		-4 // 站点编号错
#define MSG_DEV_ERR			-4 // 设备编号错
#define MSG_VP_INTERACT_ERR	-5 // VP交互标志错
#define MSG_MCP_ERR			-6 // 监控控制层协议标志错
#define MSG_AUTHORIZATION_FAILURE -7 // 电话号码鉴权
#define MSG_MCP_RESPONSE_ERR	-8 // MCP应答标志错
#define MSG_MCP_PDU_LEN_ERR		-8 // MCP监控数据长度错
#define MSG_MCP_COMMAND_ERR		-9 // MCP命令标志错
#define MSG_MCP_DATARANGE_ERR	-10 // 监控数据超限
#define MSG_ESC_ERR				-11 // 转义错误
#define MSG_PACK_NUM_ERR        -12  // 通信包标识号错

#define MCP_LEN_ERR 0x03   // MCP应答，数据长度错
#define MCP_CMD_ERR 0x02   // MCP应答，命令编号错

#define MCP_NOID_ERR		0x10  // 1:监控数据标识无法识别
#define MCP_OVER_ERR		0x20  // 2:监控数据设置值超出范围
#define MCP_CODE_ERR		0x30  // 3:监控数据不符合要求
#define MCP_FAIL_ERR		0x40  // 4:监控数据标识与监控数据长度不匹配
#define MCP_UNDER_ERR		0x50  // 5:监控数据的检测值低于工作范围
#define MCP_UP_ERR			0x60  // 6:监控数据的检测值高于工作范围
#define MCP_DATA_ERR		0x70  // 7:监控数据无法检测

#define AP_A		0x01  // 接入层协议	  串口
#define AP_B        0x02  // SMS
#define AP_C        0x03  // 以太网
#define VP_A		0x01  // VP层，目前仅支持VP:A
#pragma pack(1)
// AP协议结构
struct ap_st{
	unsigned char start_flag; // 起始标志 AP:A=0x7E AP:B=0x21
	unsigned char ap_type;    // ap:a  ap:b ap:c
	unsigned char vp_type;    // vp:a=0x01
	unsigned char dev_num[4]; // 站点编号  4
	unsigned char sub_num;    // 设备编号
	unsigned short pack_num;  // 通信包标识号 2
	unsigned char vp_iflag;   // vp层交互标志
	unsigned char mcp_flag;   // mcp层标志
	unsigned char cmd_flag;   // 命令标志
	unsigned char rep_flag;   // 应答标志
	unsigned char buf[];	  // 零长数组
};
#pragma pack()

typedef int (*func)(void*, int, int);
typedef unsigned char (*func_a)(void);

#pragma pack(1)
// PDU格式定义
struct ltv_st{	
	unsigned char len;		  // 长度
	unsigned short tag;		  // 标志
	unsigned char *value;	  // 值
	func fun;
	int eep_addr;			  // eeprom地址
};
#pragma pack()

struct _alarm_st{		// 告警计算结构
	func_a fun;		// 判断状态
	int alarm_cnt;	 // 检测告警计数
	int all_cnt;		 // 检测技术
	unsigned char status;	 // 当前状态
	unsigned char * p_en;	 // 使能状态
	unsigned char * p_value;	// 告警状态
};

void local_deal(void);
void recv_local_omc(void);
int read_from_flash(void);
int write_to_flash(void);
// 计算设备当前参数值
void read_current_value(void);
void check_alarm(void);
// 解析APA数据
int ap_deal(unsigned char * buf, int * plen, unsigned char ap);

void gpio_init(void);

int check_report(void);
#ifdef __cplusplus
}
#endif

#endif 


