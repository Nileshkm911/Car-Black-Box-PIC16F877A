/* 
 * File:   uart.c
 * Purpose: Hardware UART transmitter/receiver driver implementation
 */

#include "uart.h"

void init_uart(unsigned long baud) {
    // Configure TX (RC6) and RX (RC7) pins as inputs according to PIC16F877A datasheet
    TRISC6 = 1;
    TRISC7 = 1;
    
    // Set Baud Rate (high speed mode BRGH = 1)
    // Formula: SPBRG = (FOSC / (16 * baud)) - 1
    SPBRG = (unsigned char)((FOSC / (16 * baud)) - 1);
    
    // Configure TXSTA (Transmit Status and Control Register):
    // TX9 = 0 (8-bit transmission), TXEN = 1 (Enable transmitter), 
    // SYNC = 0 (Asynchronous mode), BRGH = 1 (High speed baud rate)
    TXEN = 1;
    BRGH = 1;
    SYNC = 0;
    
    // Configure RCSTA (Receive Status and Control Register):
    // SPEN = 1 (Enable serial port), RX9 = 0 (8-bit reception),
    // CREN = 1 (Enable continuous receive)
    SPEN = 1;
    CREN = 1;
}

void uart_putch(const char data) {
    // Wait for the Transmit Interrupt Flag (TXIF) to become 1, 
    // which indicates that the TXREG buffer is empty and ready for new data.
    while (!TXIF);
    
    // Load data into the transmit register
    TXREG = data;
}

void uart_print(const char *str) {
    // Transmit characters sequentially until the null character is reached
    while (*str != '\0') {
        uart_putch(*str++);
    }
}
