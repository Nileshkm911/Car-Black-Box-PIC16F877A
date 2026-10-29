/* 
 * File:   rtc.h
 * Purpose: DS1307 RTC I2C driver
 */

#ifndef RTC_H
#define RTC_H

#include "common.h"

// I2C Slave Addresses for DS1307
#define SLAVE_WRITE             0b11010000 // 0xD0
#define SLAVE_READ              0b11010001 // 0xD1

// DS1307 Register Addresses
#define SEC_ADDR                0x00
#define MIN_ADDR                0x01
#define HOUR_ADDR               0x02

// Function Declarations
void init_rtc(void);
unsigned char rtc_read(unsigned char addr);
void rtc_write(unsigned char addr, unsigned char data);
void get_time(unsigned char *time_bcd);
void set_time(unsigned char *time_bcd);

#endif /* RTC_H */
