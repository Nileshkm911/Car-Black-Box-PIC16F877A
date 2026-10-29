/* 
 * File:   eeprom.c
 * Purpose: Internal Data EEPROM read/write routines
 */

#include "eeprom.h"

unsigned char eeprom_read(unsigned char address) {
    // Load address register
    EEADR = address;
    
    // Select Data EEPROM memory (EEPGD = 0)
    EEPGD = 0;
    
    // Start read operation (RD = 1)
    RD = 1;
    
    // Read and return the data immediately (no delay needed for read)
    return EEDATA;
}

void eeprom_write(unsigned char address, unsigned char data) {
    unsigned char gie_status;

    // Load address and data registers
    EEADR = address;
    EEDATA = data;

    // Select Data EEPROM memory (EEPGD = 0)
    EEPGD = 0;

    // Enable write operations (WREN = 1)
    WREN = 1;

    // Save interrupt state and disable interrupts to guarantee execution of sequence
    gie_status = INTCONbits.GIE;
    INTCONbits.GIE = 0;

    // Required unlock sequence
    EECON2 = 0x55;
    EECON2 = 0xAA;
    
    // Start write operation (WR = 1)
    WR = 1;

    // Restore original interrupt state
    INTCONbits.GIE = gie_status;

    // Wait for the write cycle to complete (WR drops to 0)
    while (WR);

    // Disable further write operations
    WREN = 0;
}
