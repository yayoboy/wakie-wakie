#ifndef ALARMMGR_H
#define ALARMMGR_H

#include "ConfigMgr.h"
#include "DisplayMgr.h"
#include "TimeMgr.h"
#include <Arduino.h>

#define BUZZER_PIN 33 // PWM-capable pin for Wemos S2 Mini
#define BUTTON_PIN 0  // Boot button on Wemos S2 Mini (built-in with pull-up)

class AlarmMgr {
public:
  static AlarmMgr &getInstance() {
    static AlarmMgr instance;
    return instance;
  }

  void begin();
  void loop();
  void stopAlarm();
  void snoozeAlarm();
  bool isRinging();

private:
  AlarmMgr();
  void checkAlarms();
  void playTone();

  bool _isRinging = false;
  unsigned long _ringStartTime = 0;
  unsigned long _lastToneChange = 0;
  bool _toneHigh = false;

  // Simple state to prevent re-triggering same minute
  int _lastTriggeredHour = -1;
  int _lastTriggeredMinute = -1;
  int _lastTriggeredDay = -1;
};

#endif
