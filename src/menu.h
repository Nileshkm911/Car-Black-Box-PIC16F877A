/* 
 * File:   menu.h
 * Purpose: Main menu display and submenu operations
 */

#ifndef MENU_H
#define MENU_H

#include "common.h"

// Run the main menu and submenu logic
// Reads key presses and updates system operation state
void run_menu(unsigned char key, unsigned char *operation);

#endif /* MENU_H */
