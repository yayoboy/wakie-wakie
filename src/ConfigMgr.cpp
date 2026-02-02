#include "ConfigMgr.h"

ConfigMgr::ConfigMgr() {}

bool ConfigMgr::begin() {
  if (!LittleFS.begin()) {
    Serial.println("ConfigMgr: LittleFS Mount Failed");
    return false;
  }
  return loadConfig();
}

void ConfigMgr::resetToFactory() {
  wifi.ssid = "Wakie-Clock";
  wifi.password = "12345678";
  wifi.hostname = "wakie-wakie";
  wifi.apMode = true;

  system.timezoneOffset = 3600;
  system.ntpServer = "pool.ntp.org";
  system.weatherEnabled = false;
  system.openWeatherMapApiKey = "";
  system.city = "Rome,IT";
  system.alarmTimeout = 5; // Default 5 mins

  display.brightness = 5;
  display.dimmingStartHour = 22;
  display.dimmingEndHour = 7;
  display.dimmingBrightness = 0;
  display.showSeconds = true;  // Mostra sempre i secondi
  display.flipDisplay = false;
  display.scrollSpeed = 10;  // 10ms delay = veloce (basso = veloce in setSpeed!)

  alarms.clear();
  // Default alarm for Mon-Fri 07:00
  AlarmEntry a1;
  a1.enabled = false;
  a1.hour = 7;
  a1.minute = 0;
  a1.days = 0b01111110; // Mon(1)-Fri(5). Sun=0, Sat=6.
  // Bit 0=Sun, 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat
  // 01111110 = 0x7E. (Sat=64, Fri=32, Thu=16, Wed=8, Tue=4, Mon=2, Sun=1) ->
  // Wait Sun=Bit0(1), Mon=Bit1(2), Tue=Bit2(4), Wed=Bit3(8), Thu=Bit4(16),
  // Fri=Bit5(32), Sat=Bit6(64) Mon+Tue+Wed+Thu+Fri = 2+4+8+16+32 = 62 (0x3E).
  a1.days = 62;
  alarms.push_back(a1);

  saveConfig();
}

bool ConfigMgr::loadConfig() {
  if (!LittleFS.exists(_configFile)) {
    Serial.println("ConfigMgr: No config file found, creating default.");
    resetToFactory();
    return true;
  }

  File file = LittleFS.open(_configFile, "r");
  if (!file) {
    Serial.println("ConfigMgr: Failed to open config file");
    return false;
  }

  // Increased doc size for array
  StaticJsonDocument<4096> doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.println("ConfigMgr: Failed to deserialize config");
    return false;
  }

  // Load Wifi
  wifi.ssid = doc["wifi"]["ssid"] | "Wakie-Clock";
  wifi.password = doc["wifi"]["password"] | "12345678";
  wifi.hostname = doc["wifi"]["hostname"] | "wakie-wakie";
  wifi.apMode = doc["wifi"]["ap_mode"] | true;

  // Load System
  system.timezoneOffset = doc["system"]["tz_offset"] | 3600;
  system.ntpServer = doc["system"]["ntp_server"] | "pool.ntp.org";
  system.weatherEnabled = doc["system"]["weather_en"] | false;
  system.openWeatherMapApiKey = doc["system"]["weather_key"] | "";
  system.city = doc["system"]["city"] | "Rome,IT";
  system.alarmTimeout = doc["system"]["alm_timeout"] | 5;

  // Load Display
  display.brightness = doc["display"]["brightness"] | 5;
  display.dimmingStartHour = doc["display"]["dim_start"] | 22;
  display.dimmingEndHour = doc["display"]["dim_end"] | 7;
  display.dimmingBrightness = doc["display"]["dim_val"] | 0;
  display.showSeconds = doc["display"]["show_sec"] | true;  // Default: mostra i secondi
  display.flipDisplay = doc["display"]["flip"] | false;
  display.scrollSpeed = doc["display"]["speed"] | 40;

  // Load Alarms (Array)
  alarms.clear();
  JsonArray arr = doc["alarms"];
  for (JsonObject v : arr) {
    AlarmEntry a;
    a.enabled = v["en"];
    a.hour = v["h"];
    a.minute = v["m"];
    a.days = v["d"];
    alarms.push_back(a);
  }

  _loaded = true;
  Serial.println("ConfigMgr: Configuration loaded");
  return true;
}

bool ConfigMgr::saveConfig() {
  StaticJsonDocument<4096> doc;

  doc["wifi"]["ssid"] = wifi.ssid;
  doc["wifi"]["password"] = wifi.password;
  doc["wifi"]["hostname"] = wifi.hostname;
  doc["wifi"]["ap_mode"] = wifi.apMode;

  doc["system"]["tz_offset"] = system.timezoneOffset;
  doc["system"]["ntp_server"] = system.ntpServer;
  doc["system"]["weather_en"] = system.weatherEnabled;
  doc["system"]["weather_key"] = system.openWeatherMapApiKey;
  doc["system"]["city"] = system.city;
  doc["system"]["alm_timeout"] = system.alarmTimeout;

  doc["display"]["brightness"] = display.brightness;
  doc["display"]["dim_start"] = display.dimmingStartHour;
  doc["display"]["dim_end"] = display.dimmingEndHour;
  doc["display"]["dim_val"] = display.dimmingBrightness;
  doc["display"]["show_sec"] = display.showSeconds;
  doc["display"]["flip"] = display.flipDisplay;
  doc["display"]["speed"] = display.scrollSpeed;

  JsonArray arr = doc.createNestedArray("alarms");
  for (const auto &a : alarms) {
    JsonObject obj = arr.createNestedObject();
    obj["en"] = a.enabled;
    obj["h"] = a.hour;
    obj["m"] = a.minute;
    obj["d"] = a.days;
  }

  File file = LittleFS.open(_configFile, "w");
  if (!file) {
    Serial.println("ConfigMgr: Failed to write config file");
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("ConfigMgr: Failed to serialize config");
    file.close();
    return false;
  }

  file.close();
  Serial.println("ConfigMgr: Configuration saved");
  return true;
}
