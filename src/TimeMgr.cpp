#include "TimeMgr.h"

TimeMgr::TimeMgr() {}

void TimeMgr::begin() {
  ConfigMgr &config = ConfigMgr::getInstance();

  // configTime expects: gmtOffset_sec, daylightOffset_sec, ntpServer
  // We'll trust the offset includes daylight for simplicity or set daylight to
  // 0 and manage valid TZ strings later. Ideally we'd use POSIX TZ string, but
  // let's stick to simple offset for now as per ConfigMgr struct. If ConfigMgr
  // used a TZ string (e.g. "CET-1CEST,M3.5.0,M10.5.0/3"), we'd use setenv("TZ",
  // ...)

  configTime(config.system.timezoneOffset, 0, config.system.ntpServer.c_str());
  Serial.println("TimeMgr: NTP Init");
}

void TimeMgr::loop() {
  if (!_isTimeSet) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      _isTimeSet = true;
      Serial.println("TimeMgr: Time synchronized!");
    }
  }
}

int TimeMgr::getHour() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return 0;
  }
  return timeinfo.tm_hour;
}

int TimeMgr::getMinute() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return 0;
  }
  return timeinfo.tm_min;
}

int TimeMgr::getSecond() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return 0;
  }
  return timeinfo.tm_sec;
}

int TimeMgr::getDay() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return 0;
  }
  return timeinfo.tm_wday;
}

String TimeMgr::getFormattedTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "--:--";
  }
  char timeStringBuff[10];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M", &timeinfo);
  return String(timeStringBuff);
}

String TimeMgr::getFormattedDate() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "--/--";
  }
  char dateStringBuff[20];
  strftime(dateStringBuff, sizeof(dateStringBuff), "%d/%m/%Y", &timeinfo);
  return String(dateStringBuff);
}

bool TimeMgr::isTimeSet() { return _isTimeSet; }
