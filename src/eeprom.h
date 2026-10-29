/* 
 * File:   eeprom.h
 * Purpose: Internal Data EEPROM read and write drivers
 */

#ifndef EEPROM_H
#define EEPROM_H

#include "common.h"

#if defined(HI_TECH_C) || defined(__PICC__)
#define eeprom_write internal_eeprom_write
#define eeprom_read  internal_eeprom_read
#endif

// Read a single byte from the internal EEPROM at address (0x00 to 0xFF)
unsigned char eeprom_read(unsigned char address);

// Write a single byte to the internal EEPROM at address (0x00 to 0xFF)
void eeprom_write(unsigned char address, unsigned char data);

#endif /* EEPROM_H */
