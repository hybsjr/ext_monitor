#ifndef __FLASH_H
#define __FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

int flash_write(unsigned int * buf_32bit, int len);
int flash_read(unsigned int * buf_32bit, int len);

void local_deal(void);
void recv_local_omc(void);

#ifdef __cplusplus
}
#endif

#endif 
