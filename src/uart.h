/* 
 * File:   uart.h
 * Purpose: Hardware UART transmitter/receiver driver
 */

#ifndef UART_H
#define UART_H

#include "common.h"

// Initialize hardware UART module at the specified baud rate
void init_uart(unsigned long baud);

// Transmit a single character over UART
void uart_putch(const char data);

// Transmit a null-terminated string over UART
void uart_print(const char *str);

#endif /* UART_H */
