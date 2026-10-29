/* 
 * File:   dashboard.c
 * Purpose: Dashboard display, speed tracking, and gear state control implementation
 */

#include "dashboard.h"
#include "lcd.h"
#include "adc.h"
#include "rtc.h"
#include "event_log.h"

// Keep track of the current gear state (defaults to EVENT_ON at start)
static unsigned char current_gear = EVENT_ON;

void run_dashboard(unsigned char key) {
    unsigned char clock_reg[3];
    unsigned char speed;
    unsigned short adc_val;
    char time_str[9];
    char speed_str[3];
    unsigned char gear_changed = 0;
    
    // Display header line (16 chars)
    lcd_print(" TIME     EV  SP", LINE1(0));
    
    // Fetch current time from RTC
    get_time(clock_reg);
    
    // Format BCD time register values into a string "HH:MM:SS"
    time_str[0] = (char)(((clock_reg[0] >> 4) & 0x03) + '0'); // Hour tens
    time_str[1] = (char)((clock_reg[0] & 0x0F) + '0');        // Hour ones
    time_str[2] = ':';
    time_str[3] = (char)(((clock_reg[1] >> 4) & 0x07) + '0'); // Minute tens
    time_str[4] = (char)((clock_reg[1] & 0x0F) + '0');        // Minute ones
    time_str[5] = ':';
    time_str[6] = (char)(((clock_reg[2] >> 4) & 0x07) + '0'); // Second tens
    time_str[7] = (char)((clock_reg[2] & 0x0F) + '0');        // Second ones
    time_str[8] = '\0';
    
    // Print time on line 2 (columns 0 to 7)
    lcd_print(time_str, LINE2(0));
    
    // Read speed from ADC channel 0 and scale to 0-99 using 32-bit math to prevent overflow
    adc_val = read_adc(CHANNEL0);
    speed = (unsigned char)(((unsigned long)adc_val * 99) / 1023);
    
    // Format speed value to string
    speed_str[0] = (char)((speed / 10) + '0');
    speed_str[1] = (char)((speed % 10) + '0');
    speed_str[2] = '\0';
    
    // Process Switch Inputs for Gear Control / Collision
    if (key == SW1) { // Collision Event
        current_gear = EVENT_COLLISION;
        gear_changed = 1;
    } 
    else if (key == SW2) { // Gear UP Increment
        if (current_gear == EVENT_COLLISION || current_gear == EVENT_ON || current_gear == EVENT_GR) {
            current_gear = EVENT_GN; // Shift from Collision/Power-On/Reverse to Neutral
            gear_changed = 1;
        } else if (current_gear < EVENT_G4) {
            current_gear++; // GN -> G1 -> G2 -> G3 -> G4
            gear_changed = 1;
        }
    } 
    else if (key == SW3) { // Gear DOWN Decrement
        if (current_gear == EVENT_COLLISION || current_gear == EVENT_ON) {
            current_gear = EVENT_GN; // Shift from Collision/Power-On to Neutral
            gear_changed = 1;
        } else if (current_gear > EVENT_GN && current_gear <= EVENT_G4) {
            current_gear--; // G4 -> G3 -> G2 -> G1 -> GN
            gear_changed = 1;
        } else if (current_gear == EVENT_GN) {
            current_gear = EVENT_GR; // Shift from Neutral to Reverse
            gear_changed = 1;
        }
    }
    
    // If a switch triggered a state change, write to EEPROM event log
    if (gear_changed) {
        log_event(current_gear, speed, clock_reg);
    }
    
    // Render the event name (columns 10 to 11)
    lcd_print(event_names[current_gear], LINE2(10));
    
    // Render the speed (columns 14 to 15)
    lcd_print(speed_str, LINE2(14));
}