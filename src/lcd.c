/* 
 * File:   lcd.c
 * Purpose: 16x2 Character LCD driver implementation
 */

#include "lcd.h"

void lcd_write(unsigned char byte, unsigned char mode) {
    // Set RS pin: 0 for instruction mode, 1 for data mode
    LCD_RS = mode ? 1 : 0;
    
    // Write high nibble (D4-D7) preserving low nibble of PORTD
    LCD_DATA_PORT = (unsigned char)((LCD_DATA_PORT & 0x0F) | (byte & 0xF0));
    LCD_EN = HI;
    __delay_us(100);
    LCD_EN = LOW;

    // Write low nibble (D0-D3 shifted to RD4-RD7) preserving low nibble of PORTD
    LCD_DATA_PORT = (unsigned char)((LCD_DATA_PORT & 0x0F) | ((byte & 0x0F) << 4));
    LCD_EN = HI;
    __delay_us(100);
    LCD_EN = LOW;
    
    // Standard HD44780 command/data execution delay (45-50 us)
    __delay_us(50);
}

void init_lcd(void) {
    // Ensure PORTE/PORTA pins used by LCD (RE1=EN, RE2=RS) are in digital mode
    ADCON1 = 0x8E;

    // Configure RD4-RD7, RE1, and RE2 as outputs
    LCD_DATA_PORT_DDR &= 0x0F;
    LCD_RS_DDR = 0;
    LCD_EN_DDR = 0;
    
    // Power-up stabilization delay
    __delay_ms(30);
    
    // Set 8-bit mode reset sequence (required by HD44780 controller)
    lcd_write(EIGHT_BIT_MODE, INST_MODE);
    __delay_us(4100);
    lcd_write(EIGHT_BIT_MODE, INST_MODE);
    __delay_us(100);
    lcd_write(EIGHT_BIT_MODE, INST_MODE);
    
    // Initialize 4-bit mode
    lcd_write(FOUR_BIT_MODE, INST_MODE);
    lcd_write(TWO_LINES_5x8_4_BIT_MODE, INST_MODE);
    lcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_ms(2);
    lcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
}

void lcd_print(const char *str, unsigned char addr) {
    // Select address
    lcd_write(addr, INST_MODE);
    
    // Print characters until null terminator
    while (*str != '\0') {
        lcd_write(*str++, DATA_MODE);
    }
}

void lcd_putch(const char data, unsigned char addr) {
    // Select address and print a single char
    lcd_write(addr, INST_MODE);
    lcd_write(data, DATA_MODE);
}

void lcd_clear(void) {
    // Clear screen command
    lcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_ms(2);
}
