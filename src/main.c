/*
 * ==============================================================================
 * PROJECT NAME:        Car Black Box (CBB) Telemetry Logger
 * AUTHOR:              Nilesh Mahajan
 * TARGET DEVICE:       PIC16F877A Microcontroller
 * COMPILER:            Microchip MPLAB XC8 (C99)
 * SIMULATION ENGINE:   PICSimLab (PICGenios Configuration)
 * 
 * DESCRIPTION:
 * A vehicle telemetry logging system that monitors and records critical events
 * (Power On, Gear Shifts, and Collisions) along with current vehicle speed and
 * timestamps. Data is stored in a circular buffer in the internal EEPROM.
 * 
 * ==============================================================================
 * DEFAULT SYSTEM PASSWORD:
 *    - Default Password: 0000 (four zeros)
 *    - How to Enter: Press SW5 four times (SW4 enters '1', SW5 enters '0')
 * 
 * ==============================================================================
 * KEYPAD SWITCH MAPPINGS BY SYSTEM STATE:
 * 
 * 1. DASHBOARD MODE (Default display mode showing Time, Event, and Speed)
 *    - SW1 (RB0): Registers a Collision event ('C'), logs it, and updates display.
 *    - SW2 (RB1): Gear Shift UP (GN -> G1 -> G2 -> G3 -> G4), updates display and logs.
 *    - SW3 (RB2): Gear Shift DOWN (G4 -> G3 -> G2 -> G1 -> GN -> GR), updates display and logs.
 *    - SW4 (RB3): [LONG PRESS - 2 seconds] Opens the Security Login Screen.
 * 
 * 2. SECURITY LOGIN SCREEN (Enter 4-digit binary code)
 *    - SW4 (RB3): Enters a binary '1'.
 *    - SW5 (RB4): Enters a binary '0'.
 *    * Inactivity Timeout: If no switch is pressed for 5 seconds, returns to Dashboard.
 *    * Lockout: 3 failed attempts blocks key entry for 2 minutes (displays countdown).
 * 
 * 3. LOCKOUT SCREEN (Security block state)
 *    - All Switches (SW1 - SW5): Hard-blocked. All input is ignored.
 *    * System counts down from 120s to 0s, then returns to Dashboard.
 * 
 * 4. MAIN MENU MODE (Scrolling menu list: View Log, Clear Log, Download, Set Time, Change Pass)
 *    - SW1 (RB0): Scroll UP (highlight previous item).
 *    - SW2 (RB1): Scroll DOWN (highlight next item).
 *    - SW4 (RB3): Select/Enter the highlighted menu option.
 *    - SW5 (RB4): Exit back to the Dashboard.
 * 
 * 5. VIEW LOG SUBMENU
 *    - SW1 (RB0): Scroll UP (read older log records).
 *    - SW2 (RB1): Scroll DOWN (read newer log records). No wrap-around.
 *    - SW5 (RB4): Exit view logs and return to Main Menu.
 * 
 * 6. SET TIME SUBMENU
 *    - SW1 (RB0): Increment the value of the active field.
 *    - SW2 (RB1): Shift selection field (Hours -> Minutes -> Seconds -> Hours).
 *    - SW4 (RB3): Save adjustments to RTC and exit directly to Dashboard.
 *    - SW5 (RB4): Cancel changes and return to Main Menu.
 * 
 * 7. CHANGE PASSWORD SUBMENU
 *    - SW4 (RB3): Input binary '1'.
 *    - SW5 (RB4): Input binary '0'.
 *    * User enters new password (4 digits) and confirms it. Matches save to EEPROM.
 * ==============================================================================
 */

#include "main.h"

// --- CONFIGURATION BITS ---
#if defined(__XC) || defined(__XC8)
#pragma config FOSC = HS        // HS Oscillator (20MHz crystal)
#pragma config WDTE = OFF       // Watchdog Timer Disabled
#pragma config PWRTE = OFF      // Power-up Timer Disabled
#pragma config BOREN = OFF      // Brown-out Reset Disabled
#pragma config LVP = OFF        // Low Voltage Programming Disabled (RB3 is digital I/O)
#pragma config CPD = OFF        // Data EEPROM Code Protection Disabled
#pragma config WRT = OFF        // Flash Write Protection Disabled
#pragma config CP = OFF         // Flash Code Protection Disabled
#elif defined(HI_TECH_C) || defined(__PICC__)
__CONFIG(FOSC_HS & WDTE_OFF & PWRTE_OFF & BOREN_OFF & LVP_OFF & CPD_OFF & WRT_OFF & CP_OFF);
#endif
#include "adc.h"
#include "lcd.h"
#include "switch.h"
#include "rtc.h"
#include "uart.h"
#include "eeprom.h"
#include "event_log.h"
#include "password.h"
#include "dashboard.h"
#include "menu.h"

// --- Global Variables ---
volatile unsigned long ms_ticks = 0; // Global millisecond counter incremented by Timer0 ISR

// --- Interrupt Service Routine (ISR) ---
#if defined(__XC) || defined(__XC8)
void __interrupt() isr(void) {
#else
void interrupt isr(void) {
#endif
    // Check if Timer0 overflow flag is set
    if (TMR0IF) {
        // Reload Timer0 to maintain 1 ms period
        // Instruction cycle = Fosc/4 = 5MHz (0.2 us cycle time)
        // Prescaler = 1:32 -> Timer0 increments every 6.4 us
        // 1 ms / 6.4 us = 156.25 counts. 
        // 256 - 156 = 100. Adding +2 for register write delay compensation.
        TMR0 = 102;
        
        // Clear flag
        TMR0IF = 0;
        
        // Increment millisecond tick counter
        ms_ticks++;
    }
}

// --- Initialization Routine ---
void init_config(void) {
    // 1. Initialize character LCD (16x2)
    init_lcd();
    
    // 2. Initialize ADC for potentiometer speed tracking
    init_adc();
    
    // 3. Initialize Keypad Switches (SW1 to SW5)
    init_switch();
    
    // 4. Initialize I2C Bus at 100kHz for RTC
    init_i2c(100000);
    
    // 5. Initialize DS1307 RTC and start clock oscillator
    init_rtc();
    
    // 6. Initialize UART at 9600 Baud for downloading logs
    init_uart(9600);
    
    // 7. Initialize Password in EEPROM if first-time boot
    init_password();
    
    // 8. Configure Timer0 for 1 ms tick timebase
    // OPTION_REG: Prescaler assigned to Timer0 (PSA = 0), Rate 1:32 (PS2:PS0 = 100)
    // T0CS = 0 (Internal instruction clock source), nRBPU = 0 (PORTB pull-ups enabled)
    OPTION_REG = (unsigned char)((OPTION_REG & 0x40) | 0x04);
    nRBPU = 0;
    TMR0 = 102;
    
    // Enable Timer0 interrupt
    TMR0IE = 1;
    
    // Enable Global Interrupts (GIE) and Peripheral Interrupts (PEIE)
    PEIE = 1;
    GIE = 1;
}

void main(void) {
    // Run hardware configurations
    init_config();
    
    unsigned char key_state;
    unsigned char key_level;
    unsigned char operation = STATE_DASHBOARD;
    unsigned long lp_start_ticks = 0;
    unsigned char lp_active = 0;
    
    // Log the initial boot-up event ("ON")
    unsigned char boot_time[3];
    get_time(boot_time);
    
    unsigned short adc_val = read_adc(CHANNEL0);
    unsigned char boot_speed = (unsigned char)(((unsigned long)adc_val * 99) / 1023);
    
    log_event(EVENT_ON, boot_speed, boot_time);
    
    // Main System Execution Loop
    while (1) {
        // Read switches: state (for clicking) and level (for holding)
        key_state = read_switches(STATE_DETECTION);
        key_level = read_switches(LEVEL_DETECTION);
        
        // System State Machine
        switch (operation) {
            case STATE_DASHBOARD:
                // Check if SW4 is pressed (Active-Low: SW4 is low, others high)
                if (key_level == SW4) {
                    if (!lp_active) {
                        lp_active = 1;
                        lp_start_ticks = ms_ticks; // Capture start time
                    }
                    
                    // If SW4 is held down for 2 seconds (2000 milliseconds)
                    if ((ms_ticks - lp_start_ticks) >= 2000) {
                        lp_active = 0;
                        operation = STATE_LOGIN; // Transition to Login Screen
                        lcd_clear();
                    } else {
                        // Keep updating dashboard with speed and time, 
                        // but pass ALL_RELEASED to prevent registering key actions
                        run_dashboard(ALL_RELEASED);
                    }
                } else {
                    // Reset long press timer and run standard dashboard updates
                    lp_active = 0;
                    run_dashboard(key_state);
                }
                break;
                
            case STATE_LOGIN:
                // Run Login passcode screen
                run_login_screen(key_state, &operation);
                break;
                
            case STATE_LOCKOUT:
                // Run Lockout cooldown screen (2-minute lockout countdown)
                run_lockout_screen(&operation);
                break;
                
            case STATE_MENU:
                // Run Main Menu selection and submenus
                run_menu(key_state, &operation);
                break;
                
            default:
                operation = STATE_DASHBOARD;
                break;
        }
    }
}