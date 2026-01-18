#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>

class OTAManager {
private:
  bool started;

public:
  OTAManager();
  void begin(const char* hostname);
  void loop();
  bool isStarted() const;
};

extern OTAManager otaManager;

#endif
