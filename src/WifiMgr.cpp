#include "WifiMgr.h"

WifiMgr::WifiMgr() {}

void WifiMgr::begin() {
  ConfigMgr &config = ConfigMgr::getInstance();
  WiFi.mode(WIFI_STA); // Default to station
  WiFi.setHostname(config.wifi.hostname.c_str());

  // DisplayMgr::getInstance().showMessage("Connecting...");  // Rimosso: mostra sempre orologio
  connectToStation();
}

// --- Non-blocking Wifi Logic ---

void WifiMgr::connectToStation() {
  ConfigMgr &config = ConfigMgr::getInstance();
  DisplayMgr &display = DisplayMgr::getInstance();

  Serial.printf("WifiMgr: Connecting to %s\n", config.wifi.ssid.c_str());

  WiFi.mode(WIFI_STA);
  WiFi.begin(config.wifi.ssid.c_str(), config.wifi.password.c_str());

  _state = WIFI_STATE_CONNECTING;
  _stateEntryTime = millis();
}

void WifiMgr::startAP() {
  ConfigMgr &config = ConfigMgr::getInstance();

  WiFi.mode(WIFI_AP);
  WiFi.softAP("Wakie-Setup", "12345678");

  _state = WIFI_STATE_AP_MODE;
  Serial.print("WifiMgr: AP Started. IP: ");
  Serial.println(WiFi.softAPIP());

  // DisplayMgr::getInstance().showMessage("AP Mode");  // Rimosso: mostra sempre orologio
}

void WifiMgr::loop() {
  unsigned long now = millis();

  switch (_state) {
  case WIFI_STATE_CONNECTING:
    if (WiFi.status() == WL_CONNECTED) {
      _state = WIFI_STATE_CONNECTED;
      Serial.println("WifiMgr: Connected!");
      Serial.print("WifiMgr: IP: ");
      Serial.println(WiFi.localIP());

      // DisplayMgr::getInstance().showMessage("WiFi OK");  // Rimosso: mostra sempre orologio

      ConfigMgr &config = ConfigMgr::getInstance();
      if (MDNS.begin(config.wifi.hostname.c_str())) {
        Serial.println("WifiMgr: mDNS responder started");
      }
    } else if (now - _stateEntryTime > 10000) { // 10s Timeout
      Serial.println("WifiMgr: Connection timed out. Switching to AP.");
      startAP();
    }
    break;

  case WIFI_STATE_CONNECTED:
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WifiMgr: Lost connection. Reconnecting...");
      WiFi.disconnect();
      connectToStation();
    }
    break;

  case WIFI_STATE_AP_MODE:
    // In AP mode, we might want to try reconnecting every few minutes?
    // For now, stay in AP mode until reboot or user config change.
    break;

  case WIFI_STATE_DISCONNECTED:
    // Do nothing, wait for begin()
    break;
  }
}

bool WifiMgr::isConnected() { return _state == WIFI_STATE_CONNECTED; }

String WifiMgr::getIpAddress() {
  if (_state == WIFI_STATE_AP_MODE) {
    return WiFi.softAPIP().toString();
  }
  return WiFi.localIP().toString();
}
