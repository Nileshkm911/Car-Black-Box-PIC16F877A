/* 
 * File:   password.c
 * Purpose: Security login screen and lockout management implementation
 */

#include "password.h"
#include "eeprom.h"
#include "lcd.h"

static unsigned char failed_attempts = 0;
static unsigned char pwd_idx = 0;
static char entered_pass[5];
static unsigned long lockout_start_ticks = 0;
static unsigned long last_press_ticks = 0;
static unsigned char first_entry = 1;
static unsigned long char_reveal_ticks = 0;
static unsigned char char_revealed = 0;

void init_password(void) {
    // Read the initialization flag from EEPROM
    unsigned char flag = eeprom_read(EE_PASS_INIT_FLAG_ADDR);
    
    // If EEPROM is fresh (not 0x55), initialize password to "0000"
    if (flag != 0x55) {
        eeprom_write(EE_PASS_START_ADDR + 0, '0');
        eeprom_write(EE_PASS_START_ADDR + 1, '0');
        eeprom_write(EE_PASS_START_ADDR + 2, '0');
        eeprom_write(EE_PASS_START_ADDR + 3, '0');
        eeprom_write(EE_PASS_INIT_FLAG_ADDR, 0x55);
    }
}

unsigned char is_locked_out(void) {
    if (failed_attempts >= 3) {
        return 1;
    }
    return 0;
}

void run_login_screen(unsigned char key, unsigned char *operation) {
    char saved_pass[4];
    char disp[17] = "                ";
    
    // Initialize or reset variables upon first entry to login screen
    if (first_entry) {
        first_entry = 0;
        pwd_idx = 0;
        char_revealed = 0;
        last_press_ticks = ms_ticks;
        lcd_clear();
        lcd_print(" Enter Password ", LINE1(0));
    }
    
    // 5-Second Inactivity Timeout Check
    if ((ms_ticks - last_press_ticks) >= 5000) {
        pwd_idx = 0;
        first_entry = 1;
        *operation = STATE_DASHBOARD;
        lcd_clear();
        return;
    }
    
    // Handle Switch Key Entry (SW4 = '1', SW5 = '0')
    if (key == SW4) {
        if (pwd_idx < 4) {
            entered_pass[pwd_idx++] = '1';
            last_press_ticks = ms_ticks; // Reset inactivity timer
            char_reveal_ticks = ms_ticks;
            char_revealed = 1;
        }
    } else if (key == SW5) {
        if (pwd_idx < 4) {
            entered_pass[pwd_idx++] = '0';
            last_press_ticks = ms_ticks; // Reset inactivity timer
            char_reveal_ticks = ms_ticks;
            char_revealed = 1;
        }
    }
    
    // Render password characters centered on the 2nd line: show digit for 500ms, then '*'
    for (int i = 0; i < pwd_idx; i++) {
        if (i == (pwd_idx - 1) && char_revealed) {
            if ((ms_ticks - char_reveal_ticks) < 500) {
                disp[6 + i] = entered_pass[i]; // Show '1' or '0' for 500ms
            } else {
                char_revealed = 0;
                disp[6 + i] = '*';
            }
        } else {
            disp[6 + i] = '*';
        }
    }
    lcd_print(disp, LINE2(0));
    
    // If all 4 digits have been entered and 500ms preview elapsed, verify the password
    if (pwd_idx == 4 && ((ms_ticks - char_reveal_ticks) >= 500)) {
        // Read stored password from internal EEPROM
        saved_pass[0] = (char)eeprom_read(EE_PASS_START_ADDR + 0);
        saved_pass[1] = (char)eeprom_read(EE_PASS_START_ADDR + 1);
        saved_pass[2] = (char)eeprom_read(EE_PASS_START_ADDR + 2);
        saved_pass[3] = (char)eeprom_read(EE_PASS_START_ADDR + 3);
        
        unsigned char match = 1;
        for (int i = 0; i < 4; i++) {
            if (entered_pass[i] != saved_pass[i]) {
                match = 0;
                break;
            }
        }
        
        pwd_idx = 0; // Clear index for next login attempt
        first_entry = 1;
        
        if (match) {
            failed_attempts = 0;
            *operation = STATE_MENU;
            lcd_clear();
        } else {
            failed_attempts++;
            lcd_clear();
            
            if (failed_attempts >= 3) {
                // Lockout the system
                *operation = STATE_LOCKOUT;
                lockout_start_ticks = ms_ticks;
            } else {
                // Display error message and remaining attempts
                lcd_print(" Wrong Password ", LINE1(0));
                
                char attempt_str[17] = " Attempts Left: ";
                attempt_str[15] = (char)((3 - failed_attempts) + '0');
                lcd_print(attempt_str, LINE2(0));
                
                // Show error message for exactly 2 seconds
                __delay_ms(2000);
                lcd_clear();
                
                // Reset inactivity timer after the blocking delay
                last_press_ticks = ms_ticks;
            }
        }
    }
}

void run_lockout_screen(unsigned char *operation) {
    unsigned long elapsed_ms = ms_ticks - lockout_start_ticks;
    
    // Lockout duration is 2 minutes (120,000 milliseconds)
    if (elapsed_ms >= 120000) {
        failed_attempts = 0;
        first_entry = 1;
        *operation = STATE_DASHBOARD;
        lcd_clear();
    } else {
        unsigned int remaining_sec = (unsigned int)(120 - (elapsed_ms / 1000));
        char disp2[17] = "      120s      ";
        
        // Convert remaining seconds into individual characters to print on LCD
        disp2[6] = (char)((remaining_sec / 100) + '0');
        disp2[7] = (char)(((remaining_sec / 10) % 10) + '0');
        disp2[8] = (char)((remaining_sec % 10) + '0');
        
        // Hide leading zeros for readability (e.g. "099s" -> " 99s", "005s" -> "  5s")
        if (disp2[6] == '0') {
            disp2[6] = ' ';
            if (disp2[7] == '0') {
                disp2[7] = ' ';
            }
        }
        
        lcd_print("  wait for.....  ", LINE1(0));
        lcd_print(disp2, LINE2(0));
    }
}
