#include "modem.h"
#include "protocol.h"
#include "string.h"
#include <stdlib.h>
#include <stdio.h>
#include "uart.h"
#include "common.h"
#include "stm32f10x_it.h"
#include "stm32f10x.h"

struct queue_buf sms_index; // modem 接收缓冲队列
#define SMS_BUF_SIZE 128
unsigned char sms_buf[SMS_BUF_SIZE];
unsigned char sms_send_buf[256];
int sms_send_cnt = 0;

__IO unsigned char g_modem_idx = 0;	// modem at指令执行序号
unsigned char g_modem_at_cnt = 0;	  // modem at指令个数

unsigned char recv_tel_num[20];

struct _sms_st{
	char * at;
	unsigned int flag;
	unsigned char dly;
	int (*exel)(struct _sms_st *);
};

int at_cmd(struct _sms_st * p_st);
int at_cmd_sms(struct _sms_st * p_st);
// 设置MODEM 状态机的状态
void set_modem_state_machine(unsigned char sta);
struct _sms_st sms_st[] = {
		{.at="AT\r", .dly=3, .flag=OK_FLAG, .exel=at_cmd,},
		{.at="ATE0\r", .dly=3, .flag=OK_FLAG, .exel=at_cmd,},
		{.at="AT+CGMI\r", .dly=3, .flag=GMR_FLAG, .exel=at_cmd,},
		{.at="AT^RSSIREP=0\r", .dly=3, .flag=OK_FLAG, .exel=at_cmd,},
		{.at="AT+CREG?\r", .dly=3, .flag=CREG_FLAG, .exel=at_cmd,},
		{.at="AT+CPMS=\"SM\",\"SM\",\"SM\"\r", .dly=3, .flag=OK_FLAG, .exel=at_cmd,},
		{.at="AT+CNMI=1,1,0,0,0\r", .dly=3, .flag=OK_FLAG, .exel=at_cmd,},
		{.at="AT^HSMSSS=0,0,1,0\r", .dly=3, .flag=OK_FLAG, .exel=at_cmd,},
		{.at="AT^HCMGL=4\r", .dly=3, .flag=OK_FLAG, .exel=at_cmd,},
};
#define MODEM_AT_CNT (sizeof(sms_st)/sizeof(struct _sms_st))


// 打开modem电源
void modem_power_open(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_6);
}
// 关闭modem电源
void modem_power_close(void)
{
	GPIO_ResetBits(GPIOD, GPIO_Pin_6);
}
void reset_modem(void)
{
	unsigned int i = 0;
	unsigned int j = 0;
	
	modem_power_close();
	for(i = 0; i < 0x3ff; i++){
		IWDG_ReloadCounter();
		for(j = 0; j < 0xffff; j++){
				__ASM("NOP");	
		}
	}
	set_modem_state_machine(0);
	g_modem_idx = 0;
	modem_power_open();
}
// modem电源控制IO初始化 PD6
void modem_power_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	queue_buf_init(&sms_index, (char *)sms_buf, SMS_BUF_SIZE);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
  /* Configure PD0 and PD6 in output pushpull mode */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
	modem_power_open();	
	g_modem_at_cnt = MODEM_AT_CNT;
}

unsigned char modem_buf[256];
__IO unsigned int modem_cnt = 0;
unsigned char modem_sta = 0;
unsigned char modem_vendor; // 模块厂家
unsigned char rssi = 0; // 信号强度
__IO unsigned int g_modem_flag = 0;
__IO unsigned char modem_state = 0;	// MODEM状态机的状态指示

// 清除MODEM全部接收标志
void clear_flag_all(void)
{
	g_modem_flag = 0;
}
// 设置MODEM接收标志
void set_flag(int flag)
{
 	g_modem_flag |= flag;
}
// 清除MODEM接收标志 
void clear_flag(int flag)
{
 	g_modem_flag &= (~flag);
}
// 查询MDOEM接收标志
unsigned char read_flag(int flag)
{
 	if(g_modem_flag&flag){
		return 1;
	}
	return 0;
}

// 检查MODEM_FLAG是否置位
// 0=成功，1=失败, 2=正在检测
unsigned char check_flag(int flag, unsigned int dly)
{
	unsigned int t = get_time6_cnt();

	if(read_flag(flag)){
	 	return 0;  // 检查成功
	}else{
		if(time_after(t, dly)){ // 超时
			return 1;
		}
	}
	return 2;
}
// 准备需要发送的数据
// CRC校验和拆分
int sms_prepare_send(unsigned char * buf, int len)
{
	unsigned short tmp1 = 0;
	int cnt = 0;

	// 计算CRC
	tmp1 = crc16(buf+1, len-4);
	buf[len-3] = (unsigned char)(tmp1&0xff);
	buf[len-2] = (unsigned char)((tmp1>>8)&0xff);
	buf[len-1] = 0x21;
	// 拆分
	cnt = hex2ascii_ex(buf+1, len-2);
	buf[0] = 0x21;
	buf[cnt+2-1] = 0x21;
	
	return (cnt+2);	
}
// CRC校验
extern int check_crc(unsigned char * buf, int len);
// 短信解析
int deal_sms(unsigned char * buf, int len)
{
 	int i = 0;
	int cnt = -1;
	int start, end;
	unsigned char sta = 0;
	int ret = 0;

	while(i < len){
		switch(sta){
		 	case 0: // 找包头0x21
				if(0x21 == buf[i]){
					start = i;
					sta = 1;
				}
				break;
			case 1: // 找包尾0x21
				if(0x21 == buf[i]){
					end = i;
					sta = 2;
					cnt = end-start+1;
					i = len;
				}
				break;
			default:
				;
		}
		i += 1;
	}
	if(cnt > 10){// 解析
		cnt = ascii2hex_ex(&buf[start+1], cnt-2);
		cnt += 2;
		buf[cnt-1] = 0x21;
		// crc 
		if(1 == check_crc(buf, cnt)){
			ret = ap_deal(&buf[start], &cnt, AP_B);
			if(ret >= 0){ // 
				sms_send_cnt = sms_prepare_send(&buf[start], cnt);
				memcpy(sms_send_buf, &buf[start], sms_send_cnt);
				sms_send_buf[sms_send_cnt] = 0x1A;
				sms_send_buf[sms_send_cnt+1] = 0;
			}
		}
	}
	return cnt;
}
extern unsigned char query_tel1[];
extern unsigned char query_tel2[];
extern unsigned char query_tel3[];
extern unsigned char query_tel4[];
extern unsigned char query_tel5[];
extern unsigned char notify_tel[];

int check_telnum(void)
{
	if(strstr((char *)recv_tel_num, (char *)query_tel1) == NULL){
		if(strstr((char *)recv_tel_num, (char *)query_tel2) == NULL){
			if(strstr((char *)recv_tel_num, (char *)query_tel3) == NULL){
				if(strstr((char *)recv_tel_num, (char *)query_tel4) == NULL){
					if(strstr((char *)recv_tel_num, (char *)query_tel5) == NULL){
						if(strstr((char *)recv_tel_num, (char *)notify_tel) == NULL){
							return 0;
						}
					}
				}
			}
		}
	}
	return 1;
}
/*
** 函数功能：处理接收MODEM数据
** 输入参数：无 
** 输出参数：无
** 返回值：无
*/
char sms_init_idx[3];
unsigned char sms_init_cnt = 0;
void recv_modem(void)
{
	int len = 0;
	int i = 0;
	char * p1 = NULL;
	char * p2 = NULL;
	char * p3 = NULL;
	char * p4 = NULL;
	int sms_len = 0;
	unsigned char tmp_idx[3] = {0};
	int sms_idx_len = 0;
	unsigned char sms_idx = 0;

	len = get_queue_buf_len(&uart2);
	if(len != 0){
		pop_queue_buf(&uart2, (char *)(modem_buf+modem_cnt), len);
		#ifdef DEBUG
		my_putdata((unsigned char *)(modem_buf+modem_cnt), len);
		#endif
		if(strstr((char *)modem_buf, "OK") != NULL){ // OK 返回
			if(strstr((char *)modem_buf, "HUAWEI") !=NULL){
				modem_vendor = HUAWEI;
				set_flag(GMR_FLAG);
			}else if((p1 = strstr((char *)modem_buf, "^HCMGR:")) != NULL){
				if((p2 = strstr((char *)(p1+7), ",")) != NULL){
					memset((char *)recv_tel_num, 0 ,20); // 读取电话号码
					memcpy((char *)recv_tel_num, p1+7, p2-p1-7); 
				}
				if((p2 = strstr((char *)(p1+7), "\r\n")) != NULL){
					if((p3 = strstr((char *)(p2+2), "\r\n")) != NULL){
						if(1 == check_telnum()){// 电话号码鉴权
							sms_len = p3-p2-2; // sms 长度
							deal_sms((unsigned char *)(p2+2), sms_len);
						}
						set_flag(HCMGR_FLAG);
					}	
				}
			}else if((p1 = strstr((char *)modem_buf, "+CREG:")) != NULL){
				 if((p2 = strstr((char *)(p1+6), ",")) != NULL){
				   if((*(p2+1) == 0x31) || (*(p2+1) == 0x34)){
					 		set_flag(CREG_FLAG);
					 }
				 }	
			}else if((p1 = strstr((char *)modem_buf, "^HCMGL:")) != NULL){
				do{
					if((p2 = strstr((char *)(p1+7), ",")) != NULL){
						memset((char *)sms_init_idx, 0, 3);
						memcpy((char *)sms_init_idx, p1+7, p2-p1-7);
						sms_init_cnt = atoi((char *)sms_init_idx);
						push_queue_buf(&sms_index, (char *)&sms_init_cnt, 1);
						#ifdef DEBUG
						my_printf("push sms index = %d\n", sms_init_cnt);
						#endif
					}
				}while((p1 = strstr((char *)p2, "^HCMGL:")) != NULL);
				set_flag(OK_FLAG);
			}else{
			 	set_flag(OK_FLAG);
			}
			memset((char *)modem_buf, 0, 256);
			modem_cnt = 0;
		}else if(strstr((char *)modem_buf, "ERROR") != NULL){	 // ERROR
			modem_cnt = 0;
			memset((char *)modem_buf, 0, 256);
			set_flag(ERROR_FLAG);
		}else if(strstr((char *)modem_buf, "^HCMGSS:") != NULL){	 // SMS发送成功
			modem_cnt = 0;
			memset((char *)modem_buf, 0, 256);
			set_flag(SMS_OK);
		}else if(strstr((char *)modem_buf, "^HCMGSF:") != NULL){	 // SMS发送失败
			modem_cnt = 0;
			memset((char *)modem_buf, 0, 256);
			set_flag(SMS_ERROR);
		}else if((p4 = strstr((char *)modem_buf, "+CMTI:")) != NULL){ // 接收到SMS提示
			if((p1 = strstr((char *)p4, "\r\n")) != NULL){ 			// +CMTI:"SM",1  
				if((p2 = strstr((char *)p4, ",")) != NULL){ 			// +CMTI:"SM",1  
					if((p3 = strstr((char *)(p2+1), "\r\n")) != NULL){
						memset((char *)tmp_idx, 0, 3);
						sms_idx_len = p3-p2-1;
						memcpy((char *)tmp_idx, p2+1, sms_idx_len);
						sms_idx = atoi((char *)tmp_idx);
						push_queue_buf(&sms_index, (char *)&sms_idx, 1);
						modem_cnt = 0;
						memset((char *)modem_buf, 0, 256);
					}
				}
			}else{
				modem_cnt += len;
			}
		}else if(strstr((char *)modem_buf, ">") != NULL){ // ready SMS提示
			set_flag(SMS_READY);
			modem_cnt = 0;
			memset((char *)modem_buf, 0, 256);
		}else{
		 	modem_cnt += len;
			for(i = 0; i < modem_cnt; i++){
					if(modem_buf[i] == 0){
						modem_cnt = 0;
						memset((char *)modem_buf, 0, 256);
					}
			}
		}
	}
}
// 设置MODEM 状态机的状态
void set_modem_state_machine(unsigned char sta)
{
 	modem_state = sta;
}
// MODEM AT 指令初始化

void modem_init(void)
{
	int ret = 2;
	ret = sms_st[g_modem_idx].exel(&sms_st[g_modem_idx]);
	if(ret == 0){
			g_modem_idx++;
			if(g_modem_idx == g_modem_at_cnt){
					set_modem_state_machine(1);
					//reset_modem();
			}
	}
}
// MODEM 读取短信状态
struct _sms_st at_hcmgr = {.dly=5, .flag=HCMGR_FLAG, .exel=at_cmd};
struct _sms_st at_cmgd = {.dly=5, .flag=OK_FLAG, .exel=at_cmd};
unsigned char modem_read_sms_sta = 0;
char at_read_sms[64];
unsigned char sms_idx = 0;
void modem_read_sms(void)
{
	int cnt = 0;	
	int ret = 2;

	switch(modem_read_sms_sta){
		case 0: // 组织读短信AT指令
			cnt = get_queue_buf_len(&sms_index);
			if(cnt != 0){
				pop_queue_buf(&sms_index, (char *)&sms_idx, 1);
				sprintf(at_read_sms, "AT^HCMGR=%d\r", sms_idx);
				at_hcmgr.at = at_read_sms;
				modem_read_sms_sta = 1;
			}else{
				set_modem_state_machine(2);// 查看是否有需要发送的短信
			}
			break;
		case 1: // 发送读短信指令
			ret = at_hcmgr.exel(&at_hcmgr);
			if(ret == 0){ // 成功
				modem_read_sms_sta = 2;
			}else if(ret == 1){
				reset_modem(); // 超时
			}
			break;
		case 2: // 组织删除短信AT指令
			sprintf(at_read_sms, "AT+CMGD=%d\r", sms_idx);
			at_cmgd.at = at_read_sms;
			modem_read_sms_sta = 3;
		case 3: // 发送删除短信AT指令
			ret = at_cmgd.exel(&at_cmgd);
			if(ret == 0){ // 成功
				modem_read_sms_sta = 0;
				set_modem_state_machine(2);
			}else if(ret == 1){
				reset_modem(); // 超时
			}
			break;
		default:
			modem_read_sms_sta = 0;
	}
}
// MODEM 发送短信
//memcpy(sms_send_buf, &buf[start], sms_send_cnt);
//	apb_buf[len++] = 0x21;
//	apb_hex2asc(apb_buf, len);
//	apb_buf_cnt = len*2-2;
extern unsigned char notify_tel[];
extern unsigned char apb_buf[];
extern unsigned int apb_buf_cnt;
unsigned char modem_send_sms_sta = 0;
struct _sms_st at_hcmgs = {.dly=10, .flag=SMS_READY, .exel=at_cmd};
struct _sms_st at_hcmgs_data = {.dly=20, .flag=SMS_OK, .exel=at_cmd_sms};
char at_send_sms[64];
void modem_send_sms(void)
{
	int ret = 2;
	
	switch(modem_send_sms_sta){
		case 0:
			if(apb_buf_cnt){
					sprintf(at_send_sms, "AT^HCMGS=\"%s\"\r", notify_tel);
					at_hcmgs.at = at_send_sms;
					at_hcmgs_data.at = (char *)apb_buf;
					modem_send_sms_sta = 1;
			}else if(sms_send_cnt){
					sprintf(at_send_sms, "AT^HCMGS=\"%s\"\r", recv_tel_num);
					at_hcmgs.at = at_send_sms;
				  at_hcmgs_data.at = (char *)sms_send_buf;
					modem_send_sms_sta = 1;
			}else{
				set_modem_state_machine(1);// 查看是否有需要读取的短信
			}
			break;
		case 1:
			ret = at_hcmgs.exel(&at_hcmgs);
			if(ret == 0){ // 成功 接收到 >
				modem_send_sms_sta = 2;
			}else if(ret == 1){
				reset_modem(); // 超时
				#ifdef DEBUG
				my_putdata("at_hcmgs time out!\r\n", 21);
				#endif
			}
			break;
		case 2:
			ret = at_hcmgs_data.exel(&at_hcmgs_data);
			if(ret == 0){ // 成功 接收到 >
				modem_send_sms_sta = 0;
				if(apb_buf_cnt){
					apb_buf_cnt = 0;
				}else if(sms_send_cnt){
					sms_send_cnt = 0;
				}
			}else if(ret == 1){
				reset_modem(); // 超时
				#ifdef DEBUG
				my_putdata("at_hcmgs_data time out!\r\n", 26);
				#endif
			}else if(ret == 3){
				// 失败
			}
			break;
		default:
			modem_send_sms_sta = 0;
	}
}
// MODEM 状态机
void modem_state_machine(void)
{
	switch(modem_state){
		case 0: // MODEM 初始化
			modem_init();
			break;
		case 1: // 查询是否收到短信提示，并读取短信
			modem_read_sms();
			break;
		case 2: // 发送短信
			modem_send_sms();
			break;
		default:
			set_modem_state_machine(0);
	}
}
unsigned char at_cmd_sta = 0;
unsigned int send_time = 0;
// 发送AT命令，并检测返回值
// 0:成功 1：超时 2：正在进行
int at_cmd(struct _sms_st * p_st)
{
	int ret = 2;
	
	if(at_cmd_sta == 0){
		clear_flag(p_st->flag);
		uart_send(USART2, (unsigned char *)p_st->at, strlen(p_st->at));
		at_cmd_sta = 1;
		send_time = get_time6_cnt();
	}else{
		ret = check_flag(p_st->flag, send_time+p_st->dly);
		if(ret == 0){
			clear_flag(p_st->flag);
			at_cmd_sta = 0;
		}else if(ret == 1){
			at_cmd_sta = 0;
		}
	}		
	return ret;
}
// 已经接收到 > 符号
// 0:成功 1：超时 2：正在进行 3:失败
unsigned char at_cmd_sms_sta = 0;
int at_cmd_sms(struct _sms_st * p_st)
{
	int ret = 2;
	int ret2 = 2;
	
	if(at_cmd_sms_sta == 0){
		uart_send(USART2, (unsigned char *)p_st->at, strlen(p_st->at));
		at_cmd_sms_sta = 1;
		send_time = get_time6_cnt();
	}else{
		ret = check_flag(p_st->flag, send_time+p_st->dly);
		if(ret == 0){
			clear_flag(p_st->flag);
			at_cmd_sms_sta = 0;
		}else if(ret == 1){
			at_cmd_sms_sta = 0;
		}else if(ret == 2){
			ret2 = check_flag(SMS_ERROR, send_time+p_st->dly);
			if(ret2 == 0){
				ret = 3;
			}
		}
	}		
	return ret;
}
