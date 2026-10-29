/* 
 * File:   lcd.h
 * Purpose: 16x2 Character LCD driver
 */

#ifndef LCD_H
#define LCD_H

#include "common.h"

// Pin Mappings for CLCD (4-bit mode)
#define LCD_DATA_PORT_DDR          TRISD
#define LCD_RS_DDR                 TRISE2
#define LCD_EN_DDR                 TRISE1

#define LCD_DATA_PORT              PORTD
#define LCD_RS                     RE2
#define LCD_EN                     RE1

// Modes
#define INST_MODE                   0
#define DATA_MODE                   1

#define HI                          1
#define LOW                         0

// Line Addresses
#define LINE1(x)                    (0x80 + (x))
#define LINE2(x)                    (0xC0 + (x))

// Commands
#define EIGHT_BIT_MODE              0x33
#define FOUR_BIT_MODE               0x02
#define TWO_LINES_5x8_4_BIT_MODE    0x28
#define CLEAR_DISP_SCREEN           0x01
#define DISP_ON_AND_CURSOR_OFF      0x0C

// Function Declarations
void init_lcd(void);
void lcd_write(unsigned char byte, unsigned char mode);
void lcd_print(const char *str, unsigned char addr);
void lcd_putch(const char data, unsigned char addr);
void lcd_clear(void);

#endif /* LCD_H */
