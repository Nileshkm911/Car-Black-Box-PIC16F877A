/* 
 * File:   adc.c
 * Purpose: ADC driver implementation
 */

#include "adc.h"

void init_adc(void) {
    // Configure ADCON1: 
    // ADFM = 1 (Right justified), ADCS2 = 0 (FOSC/32 clock source)
    // PCFG = 1110 (0x8E): AN0 is Analog (RA0 speed pot), AN1..AN7 are Digital I/O
    // This ensures RE1 (LCD_EN) and RE2 (LCD_RS) are digital pins.
    ADCON1 = 0x8E;
    
    // Turn on the ADC module
    ADON = 1;
}

unsigned short read_adc(unsigned char channel) {
    // Mask and select the desired ADC channel (CHS2:CHS0 are bits 5-3 in ADCON0)
    ADCON0 = (unsigned char)((ADCON0 & 0xC7) | (channel << 3));
    
    // Start the conversion by setting the GO/nDONE bit
    GO = 1;
    
    // Wait for the conversion to complete (nDONE bit will be cleared by hardware)
    while (nDONE);
    
    // Combine ADRESH and ADRESL registers to form the 10-bit result
    return (unsigned short)((ADRESH << 8) | ADRESL);
}