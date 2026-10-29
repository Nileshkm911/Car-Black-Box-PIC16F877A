/* 
 * File:   event_log.h
 * Purpose: Event logging and circular buffer EEPROM driver
 */

#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include "common.h"
#include "eeprom.h"

// --- EEPROM Address Map for Log Storage ---
#define EE_LOG_COUNT_ADDR           0x05  // Address storing current log count (0 to 10)
#define EE_LOG_HEAD_ADDR            0x06  // Address storing head pointer (0 to 9)
#define EE_LOG_START_ADDR           0x10  // Starting address of log entries (10 * 5 bytes = 50 bytes)
#define LOG_ENTRY_SIZE              5     // Bytes per log: [HH_BCD, MM_BCD, SS_BCD, EventIdx, Speed]

// Function Declarations

// Log a new event to the circular buffer in internal EEPROM
void log_event(unsigned char event_idx, unsigned char speed, const unsigned char *time_bcd);

// Get the current number of active logs stored
unsigned char get_log_count(void);

// Read a log entry by logical index (0 = oldest, (count-1) = newest)
// Outputs to a 5-byte array log_entry
void get_log(unsigned char logical_idx, unsigned char *log_entry);

// Clear all logs by resetting count and head pointers in EEPROM
void clear_logs(void);

// Download all logs over UART in a Tera Term compatible format
void download_logs(void);

#endif /* EVENT_LOG_H */
