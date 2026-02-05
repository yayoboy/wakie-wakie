#ifndef DISPLAYMGR_H
#define DISPLAYMGR_H

#include "ConfigMgr.h"
#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>

// Hardware Definitions - Wemos S2 Mini + 4x MAX7219 8x8 modules
// IMPORTANTE: Se vedi quadrati/pattern invece di testo, cambia il tipo:
//   - PAROLA_HW (più comune, moduli cinesi - PROVA QUESTO PRIMA!)
//   - FC16_HW (moduli FC-16 originali)
//   - GENERIC_HW (universale ma lento)
//   - ICSTATION_HW (moduli ICSTATION)
//   - DR0CR0RR0_HW (layout speciale)

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW  // ESPTimeCast
#define MAX_DEVICES 4  // 4 moduli

// ESP32-S2 Pins da ESPTimeCast (testati e funzionanti con HW-692!)
#define CLK_PIN 7    // SCK
#define DATA_PIN 12  // MOSI/DIN
#define CS_PIN 11    // CS

class DisplayMgr {
public:
  static DisplayMgr &getInstance() {
    static DisplayMgr instance;
    return instance;
  }

  void begin();
  void loop();
  void updateConfig(); // Apply brightness/flip from ConfigMgr

  void showTime(int hour, int minute, int second, bool showSeconds);
  void showMessage(String msg);

private:
  DisplayMgr();
  // Software SPI esattamente come ESPTimeCast
  MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

  bool _showingMessage = false;
};

#endif
