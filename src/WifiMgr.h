#ifndef WIFIMGR_H
#define WIFIMGR_H

#include "ConfigMgr.h"
#include "DisplayMgr.h"
#include <ESPmDNS.h>
#include <WiFi.h>

class WifiMgr {
public:
  static WifiMgr &getInstance() {
    static WifiMgr instance;
    return instance;
  }

  void begin();
  void loop();
  bool isConnected();
  String getIpAddress();

private:
  WifiMgr();
  void connectToStation();
  void startAP();

  enum WifiState {
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_AP_MODE
  };

  unsigned long _lastWifiCheck = 0;
  WifiState _state = WIFI_STATE_DISCONNECTED;
  unsigned long _stateEntryTime = 0;
};

#endif
