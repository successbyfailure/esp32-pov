#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include "config.h"

class WiFiManager {
private:
  bool apMode;
  bool connected;
  unsigned long lastConnectionAttempt;
  String currentSSID;

public:
  WiFiManager();

  void init();
  void loop();

  bool startAP(const char* ssid = AP_SSID, const char* password = AP_PASSWORD);
  bool connectWiFi(const char* ssid, const char* password);
  void disconnect();

  bool isConnected();
  bool isAPMode();
  String getIP();
  String getSSID();
  int getRSSI();

private:
  void checkConnection();
};

extern WiFiManager wifiManager;

#endif
