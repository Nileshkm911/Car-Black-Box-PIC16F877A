/* 
 * File:   event_log.c
 * Purpose: Event logging circular buffer implementation
 */

#include "event_log.h"
#include "uart.h"

// Define the global mapping of event indices to strings
const char *event_names[] = {"ON", "GN", "G1", "G2", "G3", "G4", "GR", "C "};

void log_event(unsigned char event_idx, unsigned char speed, const unsigned char *time_bcd) {
    unsigned char head = eeprom_read(EE_LOG_HEAD_ADDR);
    unsigned char count = eeprom_read(EE_LOG_COUNT_ADDR);
    
    // Safety check for uninitialized EEPROM values (0xFF)
    if (head >= MAX_LOGS) {
        head = 0;
    }
    if (count > MAX_LOGS) {
        count = 0;
    }
    
    // Calculate the start address of the head slot in EEPROM
    unsigned char addr = (unsigned char)(EE_LOG_START_ADDR + (head * LOG_ENTRY_SIZE));
    
    // Write the log data
    eeprom_write(addr + 0, time_bcd[0]); // Hour BCD
    eeprom_write(addr + 1, time_bcd[1]); // Minute BCD
    eeprom_write(addr + 2, time_bcd[2]); // Second BCD
    eeprom_write(addr + 3, event_idx);   // Event index (0-7)
    eeprom_write(addr + 4, speed);       // Speed (0-99)
    
    // Increment head pointer (wrap around if it reaches MAX_LOGS)
    head = (unsigned char)((head + 1) % MAX_LOGS);
    eeprom_write(EE_LOG_HEAD_ADDR, head);
    
    // Increment log count if it hasn't reached MAX_LOGS
    if (count < MAX_LOGS) {
        count++;
        eeprom_write(EE_LOG_COUNT_ADDR, count);
    }
}

unsigned char get_log_count(void) {
    unsigned char count = eeprom_read(EE_LOG_COUNT_ADDR);
    if (count == 0xFF) {
        return 0; // Empty EEPROM case
    }
    if (count > MAX_LOGS) {
        return MAX_LOGS; // Guard against corruption
    }
    return count;
}

void get_log(unsigned char logical_idx, unsigned char *log_entry) {
    unsigned char count = get_log_count();
    unsigned char head = eeprom_read(EE_LOG_HEAD_ADDR);
    unsigned char physical_idx;
    
    if (head >= MAX_LOGS) {
        head = 0;
    }
    
    if (count < MAX_LOGS) {
        // If buffer is not full, logical index corresponds directly to physical index
        physical_idx = logical_idx;
    } else {
        // If buffer is full, the oldest log is at the head pointer, and we wrap around
        physical_idx = (unsigned char)((head + logical_idx) % MAX_LOGS);
    }
    
    unsigned char addr = (unsigned char)(EE_LOG_START_ADDR + (physical_idx * LOG_ENTRY_SIZE));
    
    // Read the 5 bytes of the log entry
    log_entry[0] = eeprom_read(addr + 0); // Hour BCD
    log_entry[1] = eeprom_read(addr + 1); // Minute BCD
    log_entry[2] = eeprom_read(addr + 2); // Second BCD
    log_entry[3] = eeprom_read(addr + 3); // Event Index
    log_entry[4] = eeprom_read(addr + 4); // Speed
}

void clear_logs(void) {
    eeprom_write(EE_LOG_COUNT_ADDR, 0);
    eeprom_write(EE_LOG_HEAD_ADDR, 0);
}

void download_logs(void) {
    unsigned char count = get_log_count();
    unsigned char log_entry[5];
    
    if (count == 0) {
        uart_print("No logs found\r\n");
        return;
    }
    
    // Print Table Header
    uart_print("#  Time      E  SP\r\n");
    
    for (unsigned char i = 0; i < count; i++) {
        get_log(i, log_entry);
        
        // Extract BCD elements for Time formatting
        char h1 = (char)(((log_entry[0] >> 4) & 0x03) + '0');
        char h0 = (char)((log_entry[0] & 0x0F) + '0');
        char m1 = (char)(((log_entry[1] >> 4) & 0x07) + '0');
        char m0 = (char)((log_entry[1] & 0x0F) + '0');
        char s1 = (char)(((log_entry[2] >> 4) & 0x07) + '0');
        char s0 = (char)((log_entry[2] & 0x0F) + '0');
        
        // Extract Speed
        char sp1 = (char)((log_entry[4] / 10) + '0');
        char sp0 = (char)((log_entry[4] % 10) + '0');
        
        // Retrieve Event String
        const char *ev_name = "??";
        if (log_entry[3] < 8) {
            ev_name = event_names[log_entry[3]];
        }
        
        // Format: "index time_str event_str speed\r\n"
        // Example: "0  19:14:41  ON  67\r\n"
        uart_putch((char)(i + '0'));
        uart_print("  ");
        uart_putch(h1);
        uart_putch(h0);
        uart_putch(':');
        uart_putch(m1);
        uart_putch(m0);
        uart_putch(':');
        uart_putch(s1);
        uart_putch(s0);
        uart_print("  ");
        uart_print(ev_name);
        uart_print("  ");
        uart_putch(sp1);
        uart_putch(sp0);
        uart_print("\r\n");
    }
}
