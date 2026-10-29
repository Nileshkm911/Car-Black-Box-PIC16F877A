/* 
 * File:   dashboard.h
 * Purpose: Dashboard display, speed tracking, and gear state control
 */

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "common.h"

// Run the main dashboard screen: processes key presses for gear shift/collision,
// reads potentiometer speed, reads RTC time, and displays them.
void run_dashboard(unsigned char key);

#endif /* DASHBOARD_H */
