#ifndef __ADC_H
#define __ADC_H

#ifdef __cplusplus
 extern "C" {
#endif

#define DET1 0
#define DET2 1
#define DET3 2
#define RF_REF_DET 3
#define RF_OUT_DET 4
#define RD_IN_DET  5
#define VBAT_DET   6
#define VDC_DET    7	 
	 
void adc_init(void);
int get_voltage(int index);
	 
#ifdef __cplusplus
}
#endif

#endif 
