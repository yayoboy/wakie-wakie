#include "AlarmMgr.h"

AlarmMgr::AlarmMgr() {}

void AlarmMgr::begin() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Assuming active low button
}

void AlarmMgr::loop() {
  // Check Hardware Button
  static unsigned long lastButtonPress = 0;
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (millis() - lastButtonPress > 500) {
      lastButtonPress = millis();
      if (_isRinging) {
        Serial.println("AlarmMgr: Button pressed, stopping alarm");
        stopAlarm();
      }
    }
  }

  // Check Alarms
  checkAlarms();

  // Handle Ringing Audio
  if (_isRinging) {
    // Auto-stop after configured timeout (seconds -> millis)
    unsigned long timeout =
        (unsigned long)ConfigMgr::getInstance().system.alarmTimeout * 1000UL;
    if (timeout == 0)
      timeout = 60000; // Safety fallback 60 seconds

    unsigned long elapsed = millis() - _ringStartTime;

    // Debug: stampa ogni 10 secondi quanto tempo è passato
    static unsigned long lastDebug = 0;
    if (millis() - lastDebug > 10000) {
      lastDebug = millis();
      Serial.printf("AlarmMgr: Ringing for %lu ms (timeout: %lu ms)\n", elapsed, timeout);
    }

    if (elapsed > timeout) {
      Serial.println("AlarmMgr: Timeout reached, stopping alarm");
      stopAlarm();
      return;
    }

    // BEEP - BEEP logic
    // Simple non-blocking beep
    unsigned long now = millis();
    if (now - _lastToneChange > 200) { // 200ms on/off
      _lastToneChange = now;
      _toneHigh = !_toneHigh;
      if (_toneHigh) {
        tone(BUZZER_PIN, 2000); // 2kHz
      } else {
        noTone(BUZZER_PIN);
      }
    }
  }
}

void AlarmMgr::checkAlarms() {
  TimeMgr &tm = TimeMgr::getInstance();
  ConfigMgr &config = ConfigMgr::getInstance();

  if (!tm.isTimeSet())
    return;

  int currentH = tm.getHour();
  int currentM = tm.getMinute();
  int currentDay = tm.getDay(); // 0-6

  // Reset _lastTriggered* quando cambia il minuto
  if (currentM != _lastTriggeredMinute) {
    _lastTriggeredHour = -1;
    _lastTriggeredMinute = -1;
    _lastTriggeredDay = -1;
  }

  // Avoid re-triggering endlessly within same minute
  if (currentH == _lastTriggeredHour && currentM == _lastTriggeredMinute &&
      currentDay == _lastTriggeredDay)
    return;

  // Check all alarms in vector
  for (const auto &alm : config.alarms) {
    if (alm.enabled) {
      // Check day bitmask. Bit N is set if Day N is active.
      // (alm.days >> currentDay) & 1
      bool dayActive = (alm.days >> currentDay) & 1;

      if (dayActive) {
        if (currentH == alm.hour && currentM == alm.minute) {
          Serial.printf("AlarmMgr: ALARM TRIGGERED! (Time: %02d:%02d)\n", currentH, currentM);
          _isRinging = true;
          _ringStartTime = millis();
          _lastTriggeredHour = currentH;
          _lastTriggeredMinute = currentM;
          _lastTriggeredDay = currentDay;
          // DisplayMgr::getInstance().showMessage("WAKE UP!");  // Rimosso: mostra sempre orologio
          break; // Trigger only one if multiple collide (rare)
        }
      }
    }
  }
}

void AlarmMgr::stopAlarm() {
  if (_isRinging) {
    Serial.println("AlarmMgr: Alarm stopped");
    _isRinging = false;
    noTone(BUZZER_PIN);
    digitalWrite(BUZZER_PIN, LOW);  // Assicura che il pin sia LOW
    // DisplayMgr::getInstance().showMessage("Good Morning");  // Rimosso: mostra sempre orologio
  }
}

void AlarmMgr::snoozeAlarm() { stopAlarm(); }

bool AlarmMgr::isRinging() { return _isRinging; }
