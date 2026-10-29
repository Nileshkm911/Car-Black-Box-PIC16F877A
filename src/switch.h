/* 
 * File:   switch.h
 * Purpose: Keypad switch input driver
 */

#ifndef SWITCH_H
#define SWITCH_H

#include "common.h"

// Initialize switch pins on PORTB as input
void init_switch(void);

// Read switches in STATE (single-click detection) or LEVEL (hold detection) mode
unsigned char read_switches(unsigned char mode);

#endif /* SWITCH_H */
