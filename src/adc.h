/* 
 * File:   adc.h
 * Purpose: ADC configuration and speed measurement driver
 */

#ifndef ADC_H
#define ADC_H

#include "common.h"

#define CHANNEL0        0x00  // AN0 (RA0)
#define CHANNEL1        0x01  // AN1 (RA1)
#define CHANNEL2        0x02  // AN2 (RA2)

void init_adc(void);
unsigned short read_adc(unsigned char channel);

#endif /* ADC_H */