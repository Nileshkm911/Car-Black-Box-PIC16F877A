/* 
 * File:   rtc.c
 * Purpose: DS1307 RTC driver with PC-Synchronized Real-Time Engine
 */

#include "rtc.h"
#include "i2c.h"

// PC-Synchronized Real-Time clock variables (driven by Timer0 ms_ticks)
static unsigned char soft_sec = 0;
static unsigned char soft_min = 0;
static unsigned char soft_hour = 0;
static unsigned long last_sec_tick = 0;

static unsigned char bcd_to_bin(unsigned char bcd) {
    return (unsigned char)(((bcd >> 4) * 10) + (bcd & 0x0F));
}

static unsigned char bin_to_bcd(unsigned char val) {
    return (unsigned char)(((val / 10) << 4) | (val % 10));
}

static void update_soft_clock(void) {
    unsigned long current_ticks = ms_ticks;
    while ((current_ticks - last_sec_tick) >= 1000) {
        last_sec_tick += 1000;
        soft_sec++;
        if (soft_sec >= 60) {
            soft_sec = 0;
            soft_min++;
            if (soft_min >= 60) {
                soft_min = 0;
                soft_hour++;
                if (soft_hour >= 24) {
                    soft_hour = 0;
                }
            }
        }
    }
}

unsigned char rtc_read(unsigned char addr) {
    unsigned char data;
    
    i2c_start();
    i2c_write(SLAVE_WRITE);
    i2c_write(addr);
    
    i2c_rep_start();
    i2c_write(SLAVE_READ);
    data = i2c_read(0);
    i2c_stop();
    
    return data;
}

void rtc_write(unsigned char addr, unsigned char data) {
    i2c_start();
    i2c_write(SLAVE_WRITE);
    i2c_write(addr);
    i2c_write(data);
    i2c_stop();
}

void init_rtc(void) {
    // 1. Initialize running clock directly from PC Wall-Clock Time at build: "__TIME__" ("HH:MM:SS")
    soft_hour = (unsigned char)((__TIME__[0] - '0') * 10 + (__TIME__[1] - '0'));
    soft_min  = (unsigned char)((__TIME__[3] - '0') * 10 + (__TIME__[4] - '0'));
    soft_sec  = (unsigned char)((__TIME__[6] - '0') * 10 + (__TIME__[7] - '0'));
    last_sec_tick = ms_ticks;
    
    // 2. Synchronize hardware DS1307 with PC time and clear CH bit
    unsigned char pc_bcd[3];
    pc_bcd[0] = bin_to_bcd(soft_hour);
    pc_bcd[1] = bin_to_bcd(soft_min);
    pc_bcd[2] = bin_to_bcd(soft_sec);
    set_time(pc_bcd);
}

void get_time(unsigned char *time_bcd) {
    update_soft_clock();
    
    // Read hardware DS1307 via sequential read
    i2c_start();
    i2c_write(SLAVE_WRITE);
    i2c_write(SEC_ADDR);
    
    i2c_rep_start();
    i2c_write(SLAVE_READ);
    
    unsigned char s = i2c_read(1);
    unsigned char m = i2c_read(1);
    unsigned char h = i2c_read(0);
    i2c_stop();
    
    h &= 0x3F;
    m &= 0x7F;
    s &= 0x7F;
    
    // If hardware DS1307 returned valid non-zero data, sync clock with DS1307
    if (h != 0 || m != 0 || s != 0) {
        soft_hour = bcd_to_bin(h);
        soft_min  = bcd_to_bin(m);
        soft_sec  = bcd_to_bin(s);
        last_sec_tick = ms_ticks;
        
        time_bcd[0] = h;
        time_bcd[1] = m;
        time_bcd[2] = s;
    } else {
        // If DS1307 returned 00:00:00 (I2C disconnected or unpowered in Proteus), return live PC clock!
        time_bcd[0] = bin_to_bcd(soft_hour);
        time_bcd[1] = bin_to_bcd(soft_min);
        time_bcd[2] = bin_to_bcd(soft_sec);
    }
}

void set_time(unsigned char *time_bcd) {
    // Write sequential burst to DS1307 (Sec, Min, Hour)
    i2c_start();
    i2c_write(SLAVE_WRITE);
    i2c_write(SEC_ADDR);
    i2c_write(time_bcd[2] & 0x7F); // Seconds, CH=0
    i2c_write(time_bcd[1] & 0x7F); // Minutes
    i2c_write(time_bcd[0] & 0x3F); // Hours
    i2c_stop();
    
    // Also update internal real-time clock
    soft_hour = bcd_to_bin(time_bcd[0] & 0x3F);
    soft_min  = bcd_to_bin(time_bcd[1] & 0x7F);
    soft_sec  = bcd_to_bin(time_bcd[2] & 0x7F);
    last_sec_tick = ms_ticks;
}
