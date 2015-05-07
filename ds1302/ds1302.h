#ifndef __DS1302_H
#define __DS1302_H

#ifdef __cplusplus
 extern "C" {
#endif
	

void Initial_DS1302(void);
void DS1302_GetTime(unsigned char *Time);
void DS1302_SetTime(unsigned char *Time);

#ifdef __cplusplus
}
#endif

#endif 
