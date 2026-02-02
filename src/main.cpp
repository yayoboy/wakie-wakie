#include <Arduino.h>
#include <LittleFS.h>
// Managers
#include "AlarmMgr.h"
#include "ConfigMgr.h"
#include "DisplayMgr.h"
#include "TimeMgr.h"
#include "WebMgr.h"
#include "WifiMgr.h"

void setup() {
  Serial.begin(115200);
  // Wait for serial
  delay(1000);
  Serial.println("\n\n--- Wakie-Wakie ---");

  // 1. Config (Filesystem)
  if (!ConfigMgr::getInstance().begin()) {
    Serial.println("Major Error: Config Init Failed");
  }

  // 2. Display (Immediate visual feedback)
  DisplayMgr::getInstance().begin();

  // 3. WiFi (Connect or AP)
  WifiMgr::getInstance().begin();

  // 4. Time
  TimeMgr::getInstance().begin();

  // 5. Web Server
  WebMgr::getInstance().begin();

  // 6. Alarms
  AlarmMgr::getInstance().begin();
}

void loop() {
  WifiMgr::getInstance().loop();
  DisplayMgr::getInstance().loop();
  TimeMgr::getInstance().loop();
  WebMgr::getInstance().loop(); // Actually empty for AsyncWebServer usually,
                                // but good practice if we add stuff
  AlarmMgr::getInstance().loop();

  // Main Coordination Logic
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    // Update Clock on Display - mostra sempre l'ora (anche prima della sincronizzazione NTP)
    TimeMgr &tm = TimeMgr::getInstance();
    DisplayMgr::getInstance().showTime(
        tm.getHour(), tm.getMinute(), tm.getSecond(),
        ConfigMgr::getInstance().display.showSeconds);
  }
}
