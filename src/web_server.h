#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "config.h"
#include "led_controller.h"
#include "pov_engine.h"
#include "effects.h"
#include "image_manager.h"
#include "wifi_manager.h"

class WebServer {
private:
  AsyncWebServer* server;
  File uploadFile;

public:
  WebServer();
  ~WebServer();

  void init();
  void setupRoutes();

private:
  // Handlers para servir archivos estáticos
  void handleRoot(AsyncWebServerRequest *request);
  void handleNotFound(AsyncWebServerRequest *request);

  // API handlers
  void handleStatus(AsyncWebServerRequest *request);
  void handleImages(AsyncWebServerRequest *request);
  void handlePlay(AsyncWebServerRequest *request);
  void handlePause(AsyncWebServerRequest *request);
  void handleStop(AsyncWebServerRequest *request);
  void handleSettings(AsyncWebServerRequest *request);
  void handleEffects(AsyncWebServerRequest *request);
  void handleEffect(AsyncWebServerRequest *request);
  void handleDeleteImage(AsyncWebServerRequest *request);
  void handleConfig(AsyncWebServerRequest *request);
  void handleConfigSave(AsyncWebServerRequest *request);

  // Upload handlers
  void handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);

  // Utilidades
  String getStatusJSON();
  String getImagesJSON();
  String getEffectsJSON();
};

extern WebServer webServer;

#endif
