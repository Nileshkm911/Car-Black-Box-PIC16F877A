/* 
 * File:   password.h
 * Purpose: Security login screen, passcode entry, and lockout management
 */

#ifndef PASSWORD_H
#define PASSWORD_H

#include "common.h"

// --- EEPROM Password Address Config ---
#define EE_PASS_START_ADDR          0x00  // Password starts at 0x00 (4 bytes)
#define EE_PASS_INIT_FLAG_ADDR      0x04  // Flag byte to check if initialized (0x55)

// Initialize password in EEPROM if not already done
void init_password(void);

// Run the Login Screen logic
void run_login_screen(unsigned char key, unsigned char *operation);

// Run the Lockout Screen logic
void run_lockout_screen(unsigned char *operation);

// Check if the system is currently locked out
unsigned char is_locked_out(void);

#endif /* PASSWORD_H */
