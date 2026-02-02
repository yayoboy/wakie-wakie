#ifndef WEBMGR_H
#define WEBMGR_H

#include "ConfigMgr.h"
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

class WebMgr {
public:
  static WebMgr &getInstance() {
    static WebMgr instance;
    return instance;
  }

  void begin();
  void loop();

private:
  WebMgr();
  AsyncWebServer server;
  bool _rebootPending = false;
  unsigned long _rebootTime = 0;

  void setupRoutes();
  void handleConfigGet(AsyncWebServerRequest *request);
  void handleConfigPost(AsyncWebServerRequest *request, uint8_t *data,
                        size_t len, size_t index, size_t total);
};

#endif
