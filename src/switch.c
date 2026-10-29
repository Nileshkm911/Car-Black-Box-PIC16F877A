/* 
 * File:   switch.c
 * Purpose: Keypad switch input driver
 */

#include "switch.h"

void init_switch(void) {
    // Set digital keypad pins (RB0-RB5) as input lines
    KEYPAD_PORT_DDR |= INPUT_LINES;
    
    // Enable PORTB internal weak pull-ups (nRBPU = 0)
    nRBPU = 0;
}

unsigned char read_switches(unsigned char mode) {
    static unsigned char once = 1;
    
    // Mask lower 5 bits (RB0-RB4 for SW1 to SW5) and force bit 5 HIGH (0x20)
    // This guarantees rock-solid compatibility in Proteus and PICSimLab
    // even if RB5 is unconnected.
    unsigned char port_val = (KEYPAD_PORT & 0x1F) | 0x20;
    
    if (mode == LEVEL_DETECTION) {
        // Return PORTB status of active switches (active-low)
        return port_val;
    } else { // STATE_DETECTION
        // Trigger only when a button is newly pressed (active-low means != ALL_RELEASED)
        if ((port_val != ALL_RELEASED) && once) {
            once = 0;
            return port_val;
        } else if (port_val == ALL_RELEASED) {
            // Reset trigger when all buttons are released
            once = 1;
        }
    }
    return ALL_RELEASED;
}
