#ifndef TIMEMGR_H
#define TIMEMGR_H

#include "ConfigMgr.h"
#include <Arduino.h>
#include <time.h>

class TimeMgr {
public:
  static TimeMgr &getInstance() {
    static TimeMgr instance;
    return instance;
  }

  void begin();
  void loop();

  int getHour();
  int getMinute();
  int getSecond();
  int getDay();
  String getFormattedTime();
  String getFormattedDate();
  bool isTimeSet();

private:
  TimeMgr();
  bool _isTimeSet = false;
};

#endif
