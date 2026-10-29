/* 
 * File:   common.h
 * Purpose: Global definitions, states, macros, and configuration settings
 */

#ifndef COMMON_H
#define COMMON_H

#if defined(__XC) || defined(__XC8)
#include <xc.h>
#elif defined(HI_TECH_C) || defined(__PICC__)
#include <htc.h>
#else
#include <xc.h>
#endif

// Undefine HTC built-in eeprom macros if present to avoid conflicting with custom eeprom.c functions
#ifdef eeprom_read
#undef eeprom_read
#endif
#ifdef eeprom_write
#undef eeprom_write
#endif

// System Clock Frequency (20MHz)
#define _XTAL_FREQ                  20000000

// FOSC constant for Baud Rate calculation
#define FOSC                        20000000

// --- System Modes (State Machine) ---
#define STATE_DASHBOARD             0
#define STATE_LOGIN                 1
#define STATE_LOCKOUT               2
#define STATE_MENU                  3

// --- Submenu Modes ---
#define SUB_NONE                    0
#define SUB_VIEW_LOG                1
#define SUB_CLEAR_LOG               2
#define SUB_DOWNLOAD_LOG            3
#define SUB_SET_TIME                4
#define SUB_CHANGE_PASSWORD         5

// --- Switch Defines (Active-Low on PORTB) ---
#define KEYPAD_PORT                 PORTB
#define KEYPAD_PORT_DDR             TRISB
#define INPUT_LINES                 0x3F // RB0 - RB5

#define SW1                         0x3E // 0b111110 -> RB0 is low
#define SW2                         0x3D // 0b111101 -> RB1 is low
#define SW3                         0x3B // 0b111011 -> RB2 is low
#define SW4                         0x37 // 0b110111 -> RB3 is low
#define SW5                         0x2F // 0b101111 -> RB4 is low
#define SW6                         0x1F // 0b011111 -> RB5 is low
#define ALL_RELEASED                INPUT_LINES

#define STATE_DETECTION             1
#define LEVEL_DETECTION             0

// --- Event Logging Settings ---
#define MAX_LOGS                    10
#define EVENT_NAME_LEN              2

// Event Types and string representation mapping
extern const char *event_names[];
#define EVENT_ON                    0
#define EVENT_GN                    1
#define EVENT_G1                    2
#define EVENT_G2                    3
#define EVENT_G3                    4
#define EVENT_G4                    5
#define EVENT_GR                    6
#define EVENT_COLLISION             7

// --- Global Variables (declared as extern, defined in main.c) ---
extern volatile unsigned long ms_ticks; // Timer0 tick counter

#endif /* COMMON_H */
