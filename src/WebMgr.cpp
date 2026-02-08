#include "WebMgr.h"

WebMgr::WebMgr() : server(80) {}

void WebMgr::begin() {
  setupRoutes();
  server.begin();
  Serial.println("WebMgr: HTTP Server started");
}

void WebMgr::loop() {
  if (_rebootPending && millis() > _rebootTime) {
    Serial.println("WebMgr: Rebooting...");
    ESP.restart();
  }
}

void WebMgr::setupRoutes() {
  // Serve Static Files
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // API: Validates if we are reachable
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"status\":\"ok\"}");
  });

  // API: Get Config
  server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleConfigGet(request);
  });

  // API: Save Config (Body handler)
  server.on(
      "/api/config", HTTP_POST,
      [this](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"status\":\"ok\"}");
        _rebootPending = true;
        _rebootTime = millis() + 1000; // Reboot dopo 1 secondo
      }, NULL,
      [this](AsyncWebServerRequest *request, uint8_t *data, size_t len,
             size_t index, size_t total) {
        this->handleConfigPost(request, data, len, index, total);
      });

  // CORS (Optional, good for dev)
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods",
                                       "GET, POST, PUT, DELETE, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers",
                                       "Content-Type");
}

void WebMgr::handleConfigGet(AsyncWebServerRequest *request) {
  if (LittleFS.exists("/config.json")) {
    request->send(LittleFS, "/config.json", "application/json");
  } else {
    request->send(500, "application/json", "{\"error\":\"No config file\"}");
  }
}

void WebMgr::handleConfigPost(AsyncWebServerRequest *request, uint8_t *data,
                              size_t len, size_t index, size_t total) {
  // Accumulate body chunks into buffer
  if (index == 0) {
    _bodyBuffer = "";
    _bodyBuffer.reserve(total);
  }
  _bodyBuffer += String((char *)data, len);

  // Wait until all chunks have been received
  if (index + len < total) {
    return;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, _bodyBuffer);
  _bodyBuffer = "";

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  ConfigMgr &config = ConfigMgr::getInstance();

  // Wifi
  if (doc["wifi"].is<JsonObject>()) {
    config.wifi.ssid = doc["wifi"]["ssid"] | config.wifi.ssid;
    config.wifi.password = doc["wifi"]["password"] | config.wifi.password;
    config.wifi.hostname = doc["wifi"]["hostname"] | config.wifi.hostname;
    config.wifi.apMode = doc["wifi"]["ap_mode"] | config.wifi.apMode;
  }

  // System
  if (doc["system"].is<JsonObject>()) {
    config.system.timezoneOffset =
        doc["system"]["tz_offset"] | config.system.timezoneOffset;
    config.system.ntpServer =
        doc["system"]["ntp_server"] | config.system.ntpServer;
    config.system.weatherEnabled =
        doc["system"]["weather_en"] | config.system.weatherEnabled;
    config.system.openWeatherMapApiKey =
        doc["system"]["weather_key"] | config.system.openWeatherMapApiKey;
    config.system.city = doc["system"]["city"] | config.system.city;
    config.system.alarmTimeout =
        doc["system"]["alm_timeout"] | config.system.alarmTimeout;
  }

  // Display
  if (doc["display"].is<JsonObject>()) {
    config.display.brightness =
        doc["display"]["brightness"] | config.display.brightness;
    config.display.dimmingStartHour =
        doc["display"]["dim_start"] | config.display.dimmingStartHour;
    config.display.dimmingEndHour =
        doc["display"]["dim_end"] | config.display.dimmingEndHour;
    config.display.dimmingBrightness =
        doc["display"]["dim_val"] | config.display.dimmingBrightness;
    config.display.showSeconds =
        doc["display"]["show_sec"] | config.display.showSeconds;
    config.display.flipDisplay =
        doc["display"]["flip"] | config.display.flipDisplay;
    config.display.scrollSpeed =
        doc["display"]["speed"] | config.display.scrollSpeed;
  }

  // Alarms (Rebuild vector)
  if (doc["alarms"].is<JsonArray>()) {
    config.alarms.clear();
    JsonArray arr = doc["alarms"];
    for (JsonObject v : arr) {
      AlarmEntry a;
      a.enabled = v["en"];
      a.hour = v["h"];
      a.minute = v["m"];
      a.days = v["d"];
      config.alarms.push_back(a);
    }
  }

  if (config.saveConfig()) {
    Serial.println("WebMgr: Config Updated via Web");
  }
}
