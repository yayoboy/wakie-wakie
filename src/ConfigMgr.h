#ifndef CONFIGMGR_H
#define CONFIGMGR_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <vector>

struct WifiConfig {
  String ssid;
  String password;
  String hostname;
  bool apMode;
};

struct SystemConfig {
  long timezoneOffset;
  String ntpServer;
  bool weatherEnabled;
  String openWeatherMapApiKey;
  String city;
  int alarmTimeout; // Minutes
};

struct DisplayConfig {
  int brightness;
  int dimmingStartHour;
  int dimmingEndHour;
  int dimmingBrightness;
  bool showSeconds;
  bool flipDisplay;
  int scrollSpeed;
};

struct AlarmEntry {
  bool enabled;
  int hour;
  int minute;
  uint8_t days; // Bitmask: Bit 0=Sun, 1=Mon, ..., 6=Sat
  // ID could be useful for frontend but array index suffices for now if we
  // rewrite whole list
};

class ConfigMgr {
public:
  static ConfigMgr &getInstance() {
    static ConfigMgr instance;
    return instance;
  }

  bool begin();
  bool loadConfig();
  bool saveConfig();
  void resetToFactory();

  WifiConfig wifi;
  SystemConfig system;
  DisplayConfig display;
  std::vector<AlarmEntry> alarms; // Dynamic list

private:
  ConfigMgr();
  String _configFile = "/config.json";
  bool _loaded = false;
};

#endif
