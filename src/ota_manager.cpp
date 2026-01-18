#include "ota_manager.h"

OTAManager::OTAManager() : started(false) {
}

void OTAManager::begin(const char* hostname) {
  if (started) return;

  ArduinoOTA.setHostname(hostname);

  ArduinoOTA.onStart([]() {
    Serial.println("OTA: inicio de actualización");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA: fin, reiniciando...");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned int lastPct = 0;
    unsigned int pct = (progress * 100) / total;
    if (pct != lastPct) {
      lastPct = pct;
      Serial.printf("OTA: %u%%\n", pct);
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]\n", error);
  });

  ArduinoOTA.begin();
  started = true;
  Serial.printf("OTA listo en '%s.local' (ArduinoOTA)\n", hostname);
}

void OTAManager::loop() {
  if (!started) return;
  ArduinoOTA.handle();
}

bool OTAManager::isStarted() const {
  return started;
}

OTAManager otaManager;
