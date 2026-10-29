/* 
 * File:   menu.c
 * Purpose: Main menu display and submenu operations implementation
 */

#include "menu.h"
#include "lcd.h"
#include "rtc.h"
#include "eeprom.h"
#include "event_log.h"
#include "password.h"

// --- Menu Item Constants ---
static const char *menu_items[] = {
    "View log",
    "Clear log",
    "Download log",
    "Set time",
    "Change password"
};
#define MENU_SIZE 5

// --- Menu State Variables ---
static unsigned char menu_select = 0;
static unsigned char menu_offset = 0;
static unsigned char submenu_state = SUB_NONE;
static unsigned char first_draw = 1;

// --- Submenu Variables ---
// 1. View Log Variables
static unsigned char view_idx = 0;

// 2. Set Time Variables
static unsigned char edit_time_bin[3]; // [Hour, Min, Sec] as binary values (0-23, 0-59, 0-59)
static unsigned char selected_field = 0; // 0=Hour, 1=Minute, 2=Second
static unsigned char blink_flag = 0;
static unsigned long blink_start_ticks = 0;

// 3. Change Password Variables
static unsigned char change_pass_phase = 0; // 0=Enter New, 1=Confirm New
static char new_pass[5];
static char confirm_pass[5];
static unsigned char pass_idx = 0;
static unsigned long change_char_ticks = 0;
static unsigned char change_char_revealed = 0;
static char menu_buf[17];

// Helper to format and print a line of the menu list
static void print_menu_line(unsigned char item_idx, unsigned char lcd_line) {
    // Put '*' marker if selected, otherwise space
    menu_buf[0] = (menu_select == item_idx) ? '*' : ' ';
    
    // Copy menu text and pad with spaces to fill 16 chars
    int idx = 0;
    while (menu_items[item_idx][idx] != '\0' && idx < 15) {
        menu_buf[1 + idx] = menu_items[item_idx][idx];
        idx++;
    }
    while (idx < 15) {
        menu_buf[1 + idx] = ' ';
        idx++;
    }
    menu_buf[16] = '\0';
    
    lcd_print(menu_buf, lcd_line);
}

void run_menu(unsigned char key, unsigned char *operation) {
    if (submenu_state == SUB_NONE) {
        // --- 1. Main Menu Navigation ---
        if (first_draw) {
            first_draw = 0;
            lcd_clear();
        }
        
        // Print the two visible menu items
        print_menu_line(menu_offset, LINE1(0));
        print_menu_line((unsigned char)(menu_offset + 1), LINE2(0));
        
        // Process scroll keys
        if (key == SW1) { // Scroll Up
            if (menu_select > 0) {
                menu_select--;
                if (menu_select < menu_offset) {
                    menu_offset = menu_select;
                }
            }
        } 
        else if (key == SW2) { // Scroll Down
            if (menu_select < (MENU_SIZE - 1)) {
                menu_select++;
                if (menu_select > (menu_offset + 1)) {
                    menu_offset = (unsigned char)(menu_select - 1);
                }
            }
        } 
        else if (key == SW4) { // Enter Submenu
            // Set transition states
            submenu_state = (unsigned char)(menu_select + 1);
            first_draw = 1;
            lcd_clear();
        } 
        else if (key == SW5) { // Exit back to Dashboard
            first_draw = 1;
            menu_select = 0;
            menu_offset = 0;
            *operation = STATE_DASHBOARD;
            lcd_clear();
        }
    } 
    else {
        // --- 2. Submenu Logic ---
        switch (submenu_state) {
            case SUB_VIEW_LOG: {
                unsigned char count = get_log_count();
                if (count == 0) {
                    lcd_print("   No logs!     ", LINE1(0));
                    lcd_print("                ", LINE2(0));
                    if (key == SW5) {
                        submenu_state = SUB_NONE;
                        first_draw = 1;
                    }
                } else {
                    if (first_draw) {
                        first_draw = 0;
                        view_idx = 0; // Start at oldest log (index 0)
                        lcd_print("#  Time      E SP", LINE1(0));
                    }
                    
                    // Fetch and print the selected log entry
                    unsigned char log_entry[5];
                    get_log(view_idx, log_entry);
                    
                    // Format Log Index
                    menu_buf[0] = (char)(view_idx + '0');
                    menu_buf[1] = ' ';
                    
                    // Format Time: BCD to ASCII
                    menu_buf[2] = (char)(((log_entry[0] >> 4) & 0x03) + '0');
                    menu_buf[3] = (char)((log_entry[0] & 0x0F) + '0');
                    menu_buf[4] = ':';
                    menu_buf[5] = (char)(((log_entry[1] >> 4) & 0x07) + '0');
                    menu_buf[6] = (char)((log_entry[1] & 0x0F) + '0');
                    menu_buf[7] = ':';
                    menu_buf[8] = (char)(((log_entry[2] >> 4) & 0x07) + '0');
                    menu_buf[9] = (char)((log_entry[2] & 0x0F) + '0');
                    menu_buf[10] = ' ';
                    
                    // Format Event name
                    menu_buf[11] = event_names[log_entry[3]][0];
                    menu_buf[12] = event_names[log_entry[3]][1];
                    menu_buf[13] = ' ';
                    
                    // Format Speed
                    menu_buf[14] = (char)((log_entry[4] / 10) + '0');
                    menu_buf[15] = (char)((log_entry[4] % 10) + '0');
                    menu_buf[16] = '\0';
                    
                    lcd_print(menu_buf, LINE2(0));
                    
                    // Scroll between records
                    if (key == SW1) { // Scroll UP (move towards oldest, index 0)
                        if (view_idx > 0) {
                            view_idx--;
                        }
                    } 
                    else if (key == SW2) { // Scroll DOWN (move towards newest, count-1)
                        if (view_idx < (count - 1)) {
                            view_idx++;
                        }
                    } 
                    else if (key == SW5) { // Exit view log
                        submenu_state = SUB_NONE;
                        first_draw = 1;
                    }
                }
                break;
            }
            
            case SUB_CLEAR_LOG: {
                lcd_print("  Clearing...   ", LINE1(0));
                lcd_print("                ", LINE2(0));
                clear_logs();
                __delay_ms(500); // Small aesthetic delay
                
                lcd_clear();
                lcd_print("  Logs cleared  ", LINE1(0));
                lcd_print("  successfully  ", LINE2(0));
                __delay_ms(1500);
                
                submenu_state = SUB_NONE;
                first_draw = 1;
                break;
            }
            
            case SUB_DOWNLOAD_LOG: {
                lcd_print(" Downloading... ", LINE1(0));
                lcd_print("                ", LINE2(0));
                download_logs();
                __delay_ms(500);
                
                lcd_clear();
                lcd_print(" Download       ", LINE1(0));
                lcd_print(" Complete       ", LINE2(0));
                __delay_ms(1500);
                
                submenu_state = SUB_NONE;
                first_draw = 1;
                break;
            }
            
            case SUB_SET_TIME: {
                if (first_draw) {
                    first_draw = 0;
                    selected_field = 0; // Start at Hour editing
                    blink_flag = 0;
                    blink_start_ticks = ms_ticks;
                    
                    // Load current RTC BCD values and convert to binary integers
                    unsigned char cur_bcd[3];
                    get_time(cur_bcd);
                    edit_time_bin[0] = (unsigned char)(((cur_bcd[0] >> 4) & 0x03) * 10 + (cur_bcd[0] & 0x0F));
                    edit_time_bin[1] = (unsigned char)(((cur_bcd[1] >> 4) & 0x07) * 10 + (cur_bcd[1] & 0x0F));
                    edit_time_bin[2] = (unsigned char)(((cur_bcd[2] >> 4) & 0x07) * 10 + (cur_bcd[2] & 0x0F));
                }
                
                // Toggle blinking flag every 500ms
                if ((ms_ticks - blink_start_ticks) >= 500) {
                    blink_flag = !blink_flag;
                    blink_start_ticks = ms_ticks;
                }
                
                // Process input keys
                if (key == SW1) { // Increment selected field
                    if (selected_field == 0) {
                        edit_time_bin[0] = (unsigned char)((edit_time_bin[0] + 1) % 24);
                    } else if (selected_field == 1) {
                        edit_time_bin[1] = (unsigned char)((edit_time_bin[1] + 1) % 60);
                    } else if (selected_field == 2) {
                        edit_time_bin[2] = (unsigned char)((edit_time_bin[2] + 1) % 60);
                    }
                    blink_flag = 0; // Solid instantly on press
                    blink_start_ticks = ms_ticks;
                } 
                else if (key == SW2) { // Change editing field (Hour -> Min -> Sec -> Hour)
                    selected_field = (unsigned char)((selected_field + 1) % 3);
                    blink_flag = 0; // Solid instantly on press
                    blink_start_ticks = ms_ticks;
                } 
                else if (key == SW4 || key == SW5) { // Save and Exit to Dashboard
                    unsigned char new_bcd[3];
                    new_bcd[0] = (unsigned char)(((edit_time_bin[0] / 10) << 4) | (edit_time_bin[0] % 10));
                    new_bcd[1] = (unsigned char)(((edit_time_bin[1] / 10) << 4) | (edit_time_bin[1] % 10));
                    new_bcd[2] = (unsigned char)(((edit_time_bin[2] / 10) << 4) | (edit_time_bin[2] % 10));
                    
                    set_time(new_bcd);
                    
                    first_draw = 1;
                    menu_select = 0;
                    menu_offset = 0;
                    submenu_state = SUB_NONE;
                    *operation = STATE_DASHBOARD;
                    lcd_clear();
                    return; // Avoid executing render block below
                }
                
                // Format the blinking time display
                for (int i = 0; i < 16; i++) menu_buf[i] = ' ';
                menu_buf[16] = '\0';
                menu_buf[6] = ':';
                menu_buf[9] = ':';
                
                // Hours field
                if (selected_field == 0 && blink_flag) {
                    menu_buf[4] = ' ';
                    menu_buf[5] = ' ';
                } else {
                    menu_buf[4] = (char)((edit_time_bin[0] / 10) + '0');
                    menu_buf[5] = (char)((edit_time_bin[0] % 10) + '0');
                }
                
                // Minutes field
                if (selected_field == 1 && blink_flag) {
                    menu_buf[7] = ' ';
                    menu_buf[8] = ' ';
                } else {
                    menu_buf[7] = (char)((edit_time_bin[1] / 10) + '0');
                    menu_buf[8] = (char)((edit_time_bin[1] % 10) + '0');
                }
                
                // Seconds field
                if (selected_field == 2 && blink_flag) {
                    menu_buf[10] = ' ';
                    menu_buf[11] = ' ';
                } else {
                    menu_buf[10] = (char)((edit_time_bin[2] / 10) + '0');
                    menu_buf[11] = (char)((edit_time_bin[2] % 10) + '0');
                }
                
                lcd_print("Time (HH:MM:SS) ", LINE1(0));
                lcd_print(menu_buf, LINE2(0));
                break;
            }
            
            case SUB_CHANGE_PASSWORD: {
                if (first_draw) {
                    first_draw = 0;
                    change_pass_phase = 0; // Reset to Enter New
                    pass_idx = 0;
                    change_char_revealed = 0;
                    lcd_clear();
                }
                
                if (change_pass_phase == 0) {
                    lcd_print("New Password:   ", LINE1(0));
                    
                    // Enter binary character (SW4 = '1', SW5 = '0')
                    if (key == SW4) {
                        if (pass_idx < 4) {
                            new_pass[pass_idx++] = '1';
                            change_char_ticks = ms_ticks;
                            change_char_revealed = 1;
                        }
                    } else if (key == SW5) {
                        if (pass_idx < 4) {
                            new_pass[pass_idx++] = '0';
                            change_char_ticks = ms_ticks;
                            change_char_revealed = 1;
                        }
                    }
                    
                    // Print characters: show pressed digit for 500ms, then '*'
                    for (int i = 0; i < 16; i++) menu_buf[i] = ' ';
                    menu_buf[16] = '\0';
                    for (int i = 0; i < pass_idx; i++) {
                        if (i == (pass_idx - 1) && change_char_revealed) {
                            if ((ms_ticks - change_char_ticks) < 500) {
                                menu_buf[6 + i] = new_pass[i];
                            } else {
                                change_char_revealed = 0;
                                menu_buf[6 + i] = '*';
                            }
                        } else {
                            menu_buf[6 + i] = '*';
                        }
                    }
                    lcd_print(menu_buf, LINE2(0));
                    
                    if (pass_idx == 4 && ((ms_ticks - change_char_ticks) >= 500)) {
                        new_pass[4] = '\0';
                        pass_idx = 0;
                        change_char_revealed = 0;
                        change_pass_phase = 1; // Transition to Confirm
                        lcd_clear();
                    }
                } 
                else if (change_pass_phase == 1) {
                    lcd_print("Confirm Pass:   ", LINE1(0));
                    
                    // Enter binary character (SW4 = '1', SW5 = '0')
                    if (key == SW4) {
                        if (pass_idx < 4) {
                            confirm_pass[pass_idx++] = '1';
                            change_char_ticks = ms_ticks;
                            change_char_revealed = 1;
                        }
                    } else if (key == SW5) {
                        if (pass_idx < 4) {
                            confirm_pass[pass_idx++] = '0';
                            change_char_ticks = ms_ticks;
                            change_char_revealed = 1;
                        }
                    }
                    
                    // Print characters: show pressed digit for 500ms, then '*'
                    for (int i = 0; i < 16; i++) menu_buf[i] = ' ';
                    menu_buf[16] = '\0';
                    for (int i = 0; i < pass_idx; i++) {
                        if (i == (pass_idx - 1) && change_char_revealed) {
                            if ((ms_ticks - change_char_ticks) < 500) {
                                menu_buf[6 + i] = confirm_pass[i];
                            } else {
                                change_char_revealed = 0;
                                menu_buf[6 + i] = '*';
                            }
                        } else {
                            menu_buf[6 + i] = '*';
                        }
                    }
                    lcd_print(menu_buf, LINE2(0));
                    
                    if (pass_idx == 4 && ((ms_ticks - change_char_ticks) >= 500)) {
                        confirm_pass[4] = '\0';
                        pass_idx = 0;
                        change_char_revealed = 0;
                        change_pass_phase = 0; // reset phase
                        
                        lcd_clear();
                        
                        // Compare entered password buffers
                        unsigned char match = 1;
                        for (int i = 0; i < 4; i++) {
                            if (new_pass[i] != confirm_pass[i]) {
                                match = 0;
                                break;
                            }
                        }
                        
                        if (match) {
                            // Write new password digits into EEPROM
                            eeprom_write(EE_PASS_START_ADDR + 0, (unsigned char)new_pass[0]);
                            eeprom_write(EE_PASS_START_ADDR + 1, (unsigned char)new_pass[1]);
                            eeprom_write(EE_PASS_START_ADDR + 2, (unsigned char)new_pass[2]);
                            eeprom_write(EE_PASS_START_ADDR + 3, (unsigned char)new_pass[3]);
                            
                            lcd_print("   pw changed   ", LINE1(0));
                            lcd_print("  successfully  ", LINE2(0));
                        } else {
                            lcd_print("  pass is not   ", LINE1(0));
                            lcd_print("    matching    ", LINE2(0));
                        }
                        
                        // Display status message for 2 seconds
                        __delay_ms(2000);
                        
                        // Return to menu root
                        submenu_state = SUB_NONE;
                        first_draw = 1;
                        lcd_clear();
                    }
                }
                break;
            }
            
            default:
                submenu_state = SUB_NONE;
                first_draw = 1;
                break;
        }
    }
}
