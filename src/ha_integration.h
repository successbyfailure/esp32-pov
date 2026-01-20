#ifndef HA_INTEGRATION_H
#define HA_INTEGRATION_H

#include <Arduino.h>
#include <PubSubClient.h>
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
  #include <ESP8266WiFi.h>
#else
  #include <WiFi.h>
#endif
#include <ArduinoJson.h>
#include "config.h"

class HAIntegration {
private:
  WiFiClient wifiClient;
  PubSubClient* mqttClient;
  bool initialized;
  bool connected;
  unsigned long lastReconnectAttempt;
  char mqttClientId[32];

public:
  HAIntegration();
  ~HAIntegration();

  void init(const char* broker, uint16_t port, const char* user = "", const char* password = "");
  void loop();

  bool isConnected();
  void publishState();
  void publishDiscovery();

private:
  void connect();
  void callback(char* topic, byte* payload, unsigned int length);
  void handleCommand(const char* payload);
  void handleBrightnessCommand(const char* payload);
  void handleEffectCommand(const char* payload);

  String getStateTopic();
  String getCommandTopic();
  String getBrightnessStateTopic();
  String getBrightnessCommandTopic();
  String getEffectStateTopic();
  String getEffectCommandTopic();
  String getDiscoveryTopic();
};

extern HAIntegration haIntegration;

#endif
