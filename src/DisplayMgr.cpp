#include "DisplayMgr.h"
#include "TimeMgr.h"

DisplayMgr::DisplayMgr() {}

void DisplayMgr::begin() {
  Serial.println("DisplayMgr: Starting Parola...");

  // ESATTAMENTE come ESPTimeCast - minimalista!
  P.begin();
  P.setCharSpacing(1);  // Spacing minimo tra caratteri
  P.setIntensity(1);
  P.setSpeed(200);  // Test: forse funziona al contrario? (alto = veloce?)
  P.displayClear();  // Clear eventuali LED accesi

  Serial.println("DisplayMgr: Init complete");

  // Apply configured brightness, flip and scroll speed
  updateConfig();

  // Mostra subito l'ora (00:00 fino alla sincronizzazione NTP)
  P.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
  P.setTextAlignment(PA_CENTER);
  P.print("00:00");
}

void DisplayMgr::updateConfig() {
  ConfigMgr &config = ConfigMgr::getInstance();

  P.setIntensity(config.display.brightness);
  P.setInvert(false); // Can add to config if needed

  if (config.display.flipDisplay) {
    P.setZoneEffect(0, true, PA_FLIP_UD);
    P.setZoneEffect(0, true, PA_FLIP_LR);
  } else {
    P.setZoneEffect(0, false, PA_FLIP_UD);
    P.setZoneEffect(0, false, PA_FLIP_LR);
  }

  P.setSpeed(config.display.scrollSpeed);
}

void DisplayMgr::loop() {
  // Dimming Logic
  static unsigned long lastDimCheck = 0;
  if (millis() - lastDimCheck > 5000) { // Check every 5s
    lastDimCheck = millis();
    ConfigMgr &config = ConfigMgr::getInstance();
    TimeMgr &tm = TimeMgr::getInstance();

    if (tm.isTimeSet()) {
      int h = tm.getHour();
      int start = config.display.dimmingStartHour;
      int end = config.display.dimmingEndHour;
      bool dim = false;

      if (start == end) {
        // Same hour: dimming disabled
        dim = false;
      } else if (start < end) {
        // e.g. 22 to 23
        if (h >= start && h < end)
          dim = true;
      } else {
        // e.g. 22 to 7 (crossover midnight)
        if (h >= start || h < end)
          dim = true;
      }

      if (dim) {
        P.setIntensity(config.display.dimmingBrightness);
      } else {
        P.setIntensity(config.display.brightness);
      }
    }
  }

  if (P.displayAnimate()) {
    // Animation finished
    if (_showingMessage) {
      _showingMessage = false;
      P.displayReset();
    }
  }
}

void DisplayMgr::showMessage(String msg) {
  _showingMessage = true;
  static char buf[64];
  msg.toCharArray(buf, 64);
  P.displayText(buf, PA_CENTER, 85, 2000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);  // 85 = velocità ESPTimeCast
}

void DisplayMgr::showTime(int hour, int minute, int second, bool showSeconds) {
  if (_showingMessage)
    return; // Don't interrupt important messages

  static char timeBuf[12];

  // Formato HH:MM con i due punti lampeggianti per indicare i secondi
  // I due punti appaiono/scompaiono ogni secondo (secondi pari = visibili, dispari = nascosti)
  if (showSeconds && (second % 2 == 0)) {
    // Secondi pari: mostra "HH:MM" con i due punti
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
  } else if (showSeconds) {
    // Secondi dispari: mostra "HH MM" senza i due punti (effetto lampeggio)
    snprintf(timeBuf, sizeof(timeBuf), "%02d %02d", hour, minute);
  } else {
    // Senza secondi: mostra sempre "HH:MM"
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour, minute);
  }

  // Use PRINT to just update text without animation if fitting
  P.setTextEffect(0, PA_PRINT, PA_NO_EFFECT);
  P.setTextAlignment(PA_CENTER);
  P.print(timeBuf);
}

