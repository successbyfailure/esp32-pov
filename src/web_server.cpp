#include "web_server.h"

extern Config config;

WebServer::WebServer() : server(nullptr) {
}

WebServer::~WebServer() {
  if (server != nullptr) {
    delete server;
  }
}

void WebServer::init() {
  server = new AsyncWebServer(WEB_SERVER_PORT);

  if (server == nullptr) {
    Serial.println("Error: No se pudo crear servidor web");
    return;
  }

  setupRoutes();
  server->begin();

  Serial.printf("Servidor web iniciado en puerto %d\n", WEB_SERVER_PORT);
}

void WebServer::setupRoutes() {
  // Servir archivos estáticos desde LittleFS
  server->serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  // API endpoints
  server->on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleStatus(request);
  });

  server->on("/api/images", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleImages(request);
  });

  server->on("/api/play", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handlePlay(request);
  });

  server->on("/api/pause", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handlePause(request);
  });

  server->on("/api/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleStop(request);
  });

  server->on("/api/settings", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleSettings(request);
  });

  server->on("/api/effects", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleEffects(request);
  });

  server->on("/api/effect", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleEffect(request);
  });

  server->on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
    this->handleConfig(request);
  });

  server->on("/api/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleConfigSave(request);
  });

  // Upload endpoint
  server->on("/api/upload", HTTP_POST,
    [](AsyncWebServerRequest *request) {
      request->send(200, "application/json", "{\"success\":true}");
    },
    [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      this->handleUpload(request, filename, index, data, len, final);
    }
  );

  // Delete image endpoint
  server->on("/api/image/delete", HTTP_POST, [this](AsyncWebServerRequest *request) {
    this->handleDeleteImage(request);
  });

  // 404 handler
  server->onNotFound([this](AsyncWebServerRequest *request) {
    this->handleNotFound(request);
  });
}

void WebServer::handleStatus(AsyncWebServerRequest *request) {
  String json = getStatusJSON();
  request->send(200, "application/json", json);
}

void WebServer::handleImages(AsyncWebServerRequest *request) {
  String json = getImagesJSON();
  request->send(200, "application/json", json);
}

void WebServer::handlePlay(AsyncWebServerRequest *request) {
  if (!request->hasParam("image", true)) {
    request->send(400, "application/json", "{\"error\":\"Missing image parameter\"}");
    return;
  }

  String imageName = request->getParam("image", true)->value();

  if (povEngine.loadImage(imageName.c_str())) {
    povEngine.play();
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json", "{\"error\":\"Failed to load image\"}");
  }
}

void WebServer::handlePause(AsyncWebServerRequest *request) {
  povEngine.pause();
  request->send(200, "application/json", "{\"success\":true}");
}

void WebServer::handleStop(AsyncWebServerRequest *request) {
  povEngine.stop();
  effects.stop();
  request->send(200, "application/json", "{\"success\":true}");
}

void WebServer::handleSettings(AsyncWebServerRequest *request) {
  bool updated = false;

  if (request->hasParam("speed", true)) {
    uint16_t speed = request->getParam("speed", true)->value().toInt();
    povEngine.setSpeed(speed);
    config.povSpeed = speed;
    updated = true;
  }

  if (request->hasParam("brightness", true)) {
    uint8_t brightness = request->getParam("brightness", true)->value().toInt();
    ledController.setBrightness(brightness);
    config.brightness = brightness;
    updated = true;
  }

  if (request->hasParam("loop", true)) {
    bool loop = request->getParam("loop", true)->value() == "true";
    povEngine.setLoopMode(loop);
    config.loopMode = loop;
    updated = true;
  }

  if (request->hasParam("orientation", true)) {
    String orient = request->getParam("orientation", true)->value();
    POVOrientation orientation = (orient == "horizontal") ? POV_HORIZONTAL : POV_VERTICAL;
    povEngine.setOrientation(orientation);
    config.povOrientation = orientation;
    updated = true;
  }

  if (request->hasParam("ledType", true)) {
    String ledTypeStr = request->getParam("ledType", true)->value();
    LEDStripType newType = LED_TYPE_WS2811;

    if (ledTypeStr == "WS2811" || ledTypeStr == "WS2812" || ledTypeStr == "WS2812B") {
      newType = LED_TYPE_WS2811;
    } else if (ledTypeStr == "APA102") {
      newType = LED_TYPE_APA102;
    }

    if (newType != config.ledType) {
      config.ledType = newType;
      ledController.setLEDType(newType);
      updated = true;
    }
  }

  if (request->hasParam("numLeds", true)) {
    uint16_t numLeds = request->getParam("numLeds", true)->value().toInt();
    if (numLeds >= MIN_LEDS && numLeds <= MAX_LEDS && numLeds != config.numLeds) {
      config.numLeds = numLeds;
      ledController.setNumLeds(numLeds);
      updated = true;
    }
  }

  if (updated) {
    saveConfig();  // Guardar cambios en archivo
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(400, "application/json", "{\"error\":\"No parameters provided\"}");
  }
}

void WebServer::handleEffects(AsyncWebServerRequest *request) {
  String json = getEffectsJSON();
  request->send(200, "application/json", json);
}

void WebServer::handleEffect(AsyncWebServerRequest *request) {
  if (!request->hasParam("effect", true)) {
    request->send(400, "application/json", "{\"error\":\"Missing effect parameter\"}");
    return;
  }

  String effectName = request->getParam("effect", true)->value();

  // Detener POV si está activo
  povEngine.stop();

  if (effectName == "rainbow") {
    uint8_t speed = request->hasParam("speed", true) ? request->getParam("speed", true)->value().toInt() : 10;
    effects.rainbow(speed);
  } else if (effectName == "solid") {
    uint8_t r = request->hasParam("r", true) ? request->getParam("r", true)->value().toInt() : 255;
    uint8_t g = request->hasParam("g", true) ? request->getParam("g", true)->value().toInt() : 255;
    uint8_t b = request->hasParam("b", true) ? request->getParam("b", true)->value().toInt() : 255;
    effects.solidColor(r, g, b);
  } else if (effectName == "chase") {
    uint8_t r = request->hasParam("r", true) ? request->getParam("r", true)->value().toInt() : 255;
    uint8_t g = request->hasParam("g", true) ? request->getParam("g", true)->value().toInt() : 0;
    uint8_t b = request->hasParam("b", true) ? request->getParam("b", true)->value().toInt() : 0;
    uint8_t speed = request->hasParam("speed", true) ? request->getParam("speed", true)->value().toInt() : 50;
    effects.colorChase(CRGB(r, g, b), speed);
  } else if (effectName == "accel") {
    uint8_t interval = request->hasParam("speed", true) ? request->getParam("speed", true)->value().toInt() : 20;
    effects.accelDirection(interval);
  } else if (effectName == "off") {
    effects.stop();
  } else {
    request->send(400, "application/json", "{\"error\":\"Unknown effect\"}");
    return;
  }

  request->send(200, "application/json", "{\"success\":true}");
}

void WebServer::handleDeleteImage(AsyncWebServerRequest *request) {
  if (!request->hasParam("image", true)) {
    request->send(400, "application/json", "{\"error\":\"Missing image parameter\"}");
    return;
  }

  String imageName = request->getParam("image", true)->value();

  if (imageManager.deleteImage(imageName.c_str())) {
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json", "{\"error\":\"Failed to delete image\"}");
  }
}

void WebServer::handleConfig(AsyncWebServerRequest *request) {
  JsonDocument doc;

  doc["deviceName"] = config.deviceName;
  doc["numLeds"] = config.numLeds;
  doc["brightness"] = config.brightness;
  doc["povSpeed"] = config.povSpeed;
  doc["loopMode"] = config.loopMode;
  doc["povOrientation"] = (config.povOrientation == POV_VERTICAL) ? "vertical" : "horizontal";
  doc["wifiSSID"] = config.wifiSSID;
  doc["wifiEnabled"] = config.wifiEnabled;
  doc["mqttEnabled"] = config.mqttEnabled;
  doc["mqttBroker"] = config.mqttBroker;
  doc["mqttPort"] = config.mqttPort;

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void WebServer::handleConfigSave(AsyncWebServerRequest *request) {
  bool updated = false;

  if (request->hasParam("deviceName", true)) {
    String name = request->getParam("deviceName", true)->value();
    strlcpy(config.deviceName, name.c_str(), sizeof(config.deviceName));
    updated = true;
  }

  if (request->hasParam("numLeds", true)) {
    uint16_t num = request->getParam("numLeds", true)->value().toInt();
    if (num >= MIN_LEDS && num <= MAX_LEDS) {
      config.numLeds = num;
      ledController.setNumLeds(num);
      updated = true;
    }
  }

  if (request->hasParam("brightness", true)) {
    uint8_t brightness = request->getParam("brightness", true)->value().toInt();
    config.brightness = constrain(brightness, 0, 255);
    ledController.setBrightness(config.brightness);
    updated = true;
  }

  if (request->hasParam("povSpeed", true)) {
    uint16_t speed = request->getParam("povSpeed", true)->value().toInt();
    config.povSpeed = constrain(speed, MIN_POV_SPEED, MAX_POV_SPEED);
    povEngine.setSpeed(config.povSpeed);
    updated = true;
  }

  if (request->hasParam("loopMode", true)) {
    String loop = request->getParam("loopMode", true)->value();
    config.loopMode = (loop == "true" || loop == "1");
    povEngine.setLoopMode(config.loopMode);
    updated = true;
  }

  if (request->hasParam("povOrientation", true)) {
    String orient = request->getParam("povOrientation", true)->value();
    config.povOrientation = (orient == "horizontal") ? POV_HORIZONTAL : POV_VERTICAL;
    povEngine.setOrientation(config.povOrientation);
    updated = true;
  }

  if (request->hasParam("wifiSSID", true)) {
    String ssid = request->getParam("wifiSSID", true)->value();
    strlcpy(config.wifiSSID, ssid.c_str(), sizeof(config.wifiSSID));
    config.wifiEnabled = ssid.length() > 0;
    updated = true;
  }

  if (request->hasParam("wifiPassword", true)) {
    String pass = request->getParam("wifiPassword", true)->value();
    strlcpy(config.wifiPassword, pass.c_str(), sizeof(config.wifiPassword));
    updated = true;
  }

  if (request->hasParam("mqttEnabled", true)) {
    String enabled = request->getParam("mqttEnabled", true)->value();
    config.mqttEnabled = (enabled == "true" || enabled == "1");
    updated = true;
  }

  if (request->hasParam("mqttBroker", true)) {
    String broker = request->getParam("mqttBroker", true)->value();
    strlcpy(config.mqttBroker, broker.c_str(), sizeof(config.mqttBroker));
    updated = true;
  }

  if (request->hasParam("mqttPort", true)) {
    uint16_t port = request->getParam("mqttPort", true)->value().toInt();
    if (port > 0) {
      config.mqttPort = port;
      updated = true;
    }
  }

  if (updated) {
    saveConfig();
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(400, "application/json", "{\"error\":\"No parameters provided\"}");
  }
}

void WebServer::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Serial.printf("Upload iniciado: %s\n", filename.c_str());

    // Validar espacio disponible
    if (imageManager.getFreeSpace() < MAX_IMAGE_SIZE) {
      Serial.println("Error: No hay espacio suficiente");
      return;
    }

    // Crear archivo en directorio de imágenes
    String filepath = String(IMAGES_DIR) + "/" + filename;
    uploadFile = LittleFS.open(filepath, "w");

    if (!uploadFile) {
      Serial.println("Error: No se pudo crear archivo");
      return;
    }
  }

  if (uploadFile && len) {
    uploadFile.write(data, len);
  }

  if (final) {
    if (uploadFile) {
      uploadFile.close();
      Serial.printf("Upload completado: %s (%d bytes)\n", filename.c_str(), index + len);

      // Refrescar lista de imágenes
      imageManager.refreshList();
    }
  }
}

void WebServer::handleNotFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String WebServer::getStatusJSON() {
  JsonDocument doc;

  doc["state"] = povEngine.isPlaying() ? "playing" : (povEngine.isPaused() ? "paused" : "idle");
  doc["image"] = povEngine.getCurrentImageName();
  doc["column"] = povEngine.getCurrentColumn();
  doc["totalColumns"] = povEngine.getTotalColumns();
  doc["speed"] = povEngine.getSpeed();
  doc["measuredFps"] = povEngine.getMeasuredFps();
  doc["brightness"] = ledController.getBrightness();
  doc["loopMode"] = povEngine.getLoopMode();
  doc["orientation"] = (povEngine.getOrientation() == POV_VERTICAL) ? "vertical" : "horizontal";
  doc["direction"] = povEngine.isReverse() ? "right_to_left" : "left_to_right";

  // LED configuration
  String ledTypeStr = "WS2811";
  switch (ledController.getLEDType()) {
    case LED_TYPE_WS2811:
    case LED_TYPE_WS2812:
    case LED_TYPE_WS2812B:
      ledTypeStr = "WS2811";
      break;
    case LED_TYPE_APA102:
      ledTypeStr = "APA102";
      break;
  }
  doc["ledType"] = ledTypeStr;
  doc["numLeds"] = ledController.getNumLeds();

  doc["effectRunning"] = effects.isRunning();
  doc["effectType"] = effects.getCurrentEffect();
  doc["wifiConnected"] = wifiManager.isConnected();
  doc["wifiSSID"] = wifiManager.getSSID();
  doc["wifiIP"] = wifiManager.getIP();
  doc["freeSpace"] = imageManager.getFreeSpace();

  String json;
  serializeJson(doc, json);
  return json;
}

String WebServer::getImagesJSON() {
  JsonDocument doc;
  JsonArray images = doc["images"].to<JsonArray>();

  std::vector<ImageInfo> imageList = imageManager.listImages();

  for (const auto& img : imageList) {
    JsonObject imgObj = images.add<JsonObject>();
    imgObj["name"] = img.filename;
    imgObj["width"] = img.width;
    imgObj["height"] = img.height;
    imgObj["size"] = img.fileSize;
    imgObj["format"] = (img.format == 0) ? "BMP" : "RGB565";
  }

  doc["freeSpace"] = imageManager.getFreeSpace();
  doc["totalSpace"] = imageManager.getTotalSpace();

  String json;
  serializeJson(doc, json);
  return json;
}

String WebServer::getEffectsJSON() {
  JsonDocument doc;
  JsonArray effectsArray = doc["effects"].to<JsonArray>();

  effectsArray.add("rainbow");
  effectsArray.add("solid");
  effectsArray.add("chase");
  effectsArray.add("accel");
  effectsArray.add("off");

  String json;
  serializeJson(doc, json);
  return json;
}

// Instancia global
WebServer webServer;
