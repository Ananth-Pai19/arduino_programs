/*
 * wifi_control.h — phone control for the pick-and-place arm (Induino / UNO R4 WiFi)
 *
 * The board makes a WiFi hotspot "ColourBot" (password colourbot123).
 * Phone: join it, open http://192.168.4.1 in the browser.
 *
 * The phone page replaces the Serial Monitor input. Your sketch:
 *   - calls wifiSetup() at the end of setup()
 *   - calls wifiLoop() at the start of loop()
 *   - asks wifiGetMission(pick, drop) when it is waiting for colours
 *   - asks wifiStopRequested() to see if STOP was pressed
 *   - calls wifiSetStatus(...) so the phone shows what the arm is doing
 */
#pragma once
#include <Arduino.h>

void wifiSetup();
void wifiLoop();

// Returns true ONCE when the phone presses "Start". pick/drop are "RED" / "GREEN" / "BLUE".
bool wifiGetMission(String& pick, String& drop);

// Returns true ONCE when the phone presses STOP.
bool wifiStopRequested();

// state: "idle", "sweep_pick", "pick", "sweep_drop", "drop" or "home"
void wifiSetStatus(const char* state, const String& msg, bool magnetOn);
