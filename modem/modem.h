#ifndef __MODEM_H
#define __MODEM_H

#ifdef __cplusplus
 extern "C" {
#endif
	
#define HUAWEI 1

// 接收到 OK
#define OK_FLAG 			(0x00000001)
// 接收到 ERROR
#define ERROR_FLAG		(0x00000002) 
// 接收到 CSQ
#define CSQ_FLAG			(0x00000004)
// 接收到 CGMI 厂商名称
#define GMR_FLAG			(0x00000008)
// 接收到 > 可以发送短信内容
#define SMS_READY			(0x00000010)
// SMS发送成功 ^HCMGSS:9
#define SMS_OK				(0x00000020)
// SMS发送失败 ^HCMGSF:107
#define SMS_ERROR			(0x00000040)
// 查询CREG 是否注册成功  +CREG: 0,1/4
#define CREG_FLAG     (0x00000080)
// 接收到SMS	 ^HCMGR:15369364554,2015,04,09,21,33,10,0,1,12,1,0,0,1
#define HCMGR_FLAG    (0x00000100)


// modem电源控制IO初始化 PD6
void modem_power_init(void);
// MODEM 状态机
void modem_state_machine(void);
/*
** 函数功能：处理接收MODEM数据
** 输入参数：无 
** 输出参数：无
** 返回值：无
*/
void recv_modem(void);

#ifdef __cplusplus
}
#endif

#endif 
