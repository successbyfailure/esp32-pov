#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include "config.h"
#include "led_controller.h"
#include "pov_engine.h"
#include "effects.h"
#include "image_manager.h"
#include "image_parser.h"
#include "wifi_manager.h"
#include "web_server.h"
#include "ha_integration.h"
#include "ota_manager.h"

#ifdef HAS_ACCELEROMETER
#include "accelerometer.h"
#endif

#ifdef BUTTON_PIN
#include "button.h"
#endif

// Configuración global
Config config;

// Variables para BornHack Badge
#ifdef BORNHACK_BADGE
int currentEffectIndex = 0;
const int NUM_EFFECTS = 4;
unsigned long lastMotionCheck = 0;
bool motionDetected = false;
int8_t lastDirectionSign = 1;  // 1: izquierda->derecha (default), -1: derecha->izquierda
#endif

// Funciones de configuración
bool loadConfig();
bool saveConfig();
void printConfig();

// mDNS
static bool mdnsStarted = false;
static String sanitizeHostname(const String& raw) {
  String out;
  out.reserve(raw.length());
  bool lastDash = false;
  for (size_t i = 0; i < raw.length(); i++) {
    char c = raw.charAt(i);
    if (isalnum(static_cast<unsigned char>(c))) {
      out += (char)tolower(c);
      lastDash = false;
    } else if (c == ' ' || c == '_' || c == '-' ) {
      if (!lastDash && out.length() > 0) {
        out += '-';
        lastDash = true;
      }
    }
  }
  // Quitar guion final si quedó
  while (out.endsWith("-")) {
    out.remove(out.length() - 1);
  }
  if (out.length() == 0) {
    out = "pov-line";
  }
  return out;
}

static void startMDNS(const String& deviceName) {
  String host = sanitizeHostname(deviceName);
  if (mdnsStarted) {
    MDNS.end();
    mdnsStarted = false;
  }
  if (MDNS.begin(host.c_str())) {
    MDNS.addService("http", "tcp", WEB_SERVER_PORT);
    mdnsStarted = true;
    Serial.printf("mDNS activo: http://%s.local\n", host.c_str());
  } else {
    Serial.println("AVISO: No se pudo iniciar mDNS");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("POV Line - Firmware v" + String(FIRMWARE_VERSION));
  Serial.println("========================================\n");

#ifdef BORNHACK_MINIMAL
  Serial.println("*** MODO MINIMAL ACTIVADO ***");
  Serial.println("Solo Serial habilitado - periféricos desactivados");
  Serial.println("Esto es para diagnosticar problemas de GPIO");
  Serial.println("\nESP32-C3 arrancado correctamente!");
  Serial.println("\nInfo del chip:");
  Serial.printf("  Chip: ESP32-C3 Rev %d\n", ESP.getChipRevision());
  Serial.printf("  Cores: %d\n", ESP.getChipCores());
  Serial.printf("  Frecuencia: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("  Flash: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("  RAM libre: %d bytes\n", ESP.getFreeHeap());
  Serial.println("\nEsperando comandos...\n");
  return;  // Salir temprano, no inicializar nada más
#endif

  // 1. Inicializar LittleFS
  Serial.println("[1/7] Inicializando sistema de archivos...");
  if (!LittleFS.begin(true)) {
    Serial.println("ERROR: No se pudo inicializar LittleFS");
    Serial.println("Sistema detenido");
    while (1) {
      delay(1000);
    }
  }
  Serial.printf("LittleFS OK - Total: %d KB, Usado: %d KB, Libre: %d KB\n",
                LittleFS.totalBytes() / 1024,
                LittleFS.usedBytes() / 1024,
                (LittleFS.totalBytes() - LittleFS.usedBytes()) / 1024);

  // 2. Cargar configuración
  Serial.println("\n[2/7] Cargando configuración...");
  if (loadConfig()) {
    Serial.println("Configuración cargada desde archivo");
    printConfig();
  } else {
    Serial.println("Usando configuración por defecto");
    saveConfig();
  }

  // 3. Inicializar LEDs
  Serial.println("\n[3/7] Inicializando controlador de LEDs...");
  if (!ledController.init(config.numLeds, config.ledType)) {
    Serial.println("ERROR: No se pudo inicializar LEDs");
  } else {
    ledController.setBrightness(config.brightness);
    Serial.println("LEDs inicializados correctamente");

    // Efecto de inicio
    ledController.fill(CRGB::Blue);
    ledController.show();
    delay(500);
    ledController.clear();
    ledController.show();
  }

#ifdef HAS_ACCELEROMETER
  // 3b. Inicializar acelerómetro (BornHack Badge)
  Serial.println("\n[3b] Inicializando acelerómetro...");
  if (!accelerometer.init()) {
    Serial.println("AVISO: Acelerómetro no disponible");
  } else {
    Serial.println("Acelerómetro listo para detección de movimiento POV");
  }
#endif

#ifdef BUTTON_PIN
  // 3c. Inicializar botón (BornHack Badge)
  Serial.println("\n[3c] Inicializando botón...");
  button.init();
  Serial.println("Botón listo - presionar para cambiar efectos");
#endif

  // 4. Inicializar gestor de imágenes
  Serial.println("\n[4/7] Inicializando gestor de imágenes...");
  if (!imageManager.init()) {
    Serial.println("ERROR: No se pudo inicializar gestor de imágenes");
  } else {
    Serial.println("Gestor de imágenes inicializado");
    // Si no hay imagen activa, seleccionar la primera disponible
    if (strlen(config.activeImage) == 0) {
      auto imgs = imageManager.listImages();
      if (!imgs.empty()) {
        strlcpy(config.activeImage, imgs[0].filename, sizeof(config.activeImage));
        Serial.printf("Imagen por defecto seleccionada: %s\n", config.activeImage);
        povEngine.loadImage(config.activeImage);
        saveConfig();
      } else {
        Serial.println("No hay imágenes en /images; sube una para POV");
      }
    }
  }

  // 5. Inicializar WiFi
  Serial.println("\n[5/7] Inicializando WiFi...");
  wifiManager.init();

  // Intentar conectar a WiFi guardado
  if (config.wifiEnabled && strlen(config.wifiSSID) > 0) {
    Serial.printf("Intentando conectar a: %s\n", config.wifiSSID);
    if (wifiManager.connectWiFi(config.wifiSSID, config.wifiPassword)) {
      Serial.printf("Conectado! IP: %s\n", wifiManager.getIP().c_str());
      startMDNS(config.deviceName);
      otaManager.begin(config.deviceName);
    } else {
      Serial.println("No se pudo conectar. Iniciando modo AP...");
      wifiManager.startAP();
      Serial.printf("Modo AP activo. IP: %s\n", wifiManager.getIP().c_str());
      startMDNS("pov-line");
    }
  } else {
    Serial.println("WiFi no configurado. Iniciando modo AP...");
    wifiManager.startAP();
    Serial.printf("Modo AP activo. IP: %s\n", wifiManager.getIP().c_str());
    startMDNS("pov-line");
  }

  // 6. Inicializar servidor web
  Serial.println("\n[6/7] Inicializando servidor web...");
  webServer.init();
  Serial.printf("Servidor web activo en http://%s\n", wifiManager.getIP().c_str());

  // 7. Inicializar Home Assistant (si está habilitado)
  Serial.println("\n[7/7] Configurando Home Assistant...");
  if (config.mqttEnabled && wifiManager.isConnected()) {
    haIntegration.init(config.mqttBroker, config.mqttPort, config.mqttUser, config.mqttPassword);
    Serial.printf("MQTT configurado para: %s:%d\n", config.mqttBroker, config.mqttPort);
  } else {
    Serial.println("Home Assistant deshabilitado o WiFi no conectado");
  }

  // Configurar motor POV con valores guardados
  povEngine.setSpeed(config.povSpeed);
  povEngine.setLoopMode(config.loopMode);
  povEngine.setOrientation(config.povOrientation);

  // Cargar imagen activa si existe
  if (strlen(config.activeImage) > 0) {
    Serial.printf("\nCargando imagen activa: %s\n", config.activeImage);
    if (povEngine.loadImage(config.activeImage)) {
      Serial.println("Imagen activa cargada correctamente");
    }
  }

  Serial.println("\n========================================");
  Serial.println("Sistema iniciado correctamente");
  Serial.println("========================================\n");

  // Efecto inicial: Rainbow
  effects.rainbow(15);
}

void loop() {
#ifdef BORNHACK_MINIMAL
  // Modo minimal - solo heartbeat
  static unsigned long lastHeart = 0;
  if (millis() - lastHeart > 5000) {
    lastHeart = millis();
    Serial.printf("[%lu] Heartbeat - RAM libre: %d bytes\n", millis() / 1000, ESP.getFreeHeap());
  }
  delay(100);
  return;
#endif

#ifdef BORNHACK_BADGE
  // ==== MODO BORNHACK BADGE ====

  // Actualizar acelerómetro
#ifdef HAS_ACCELEROMETER
  accelerometer.update();

  // Detectar movimiento cada 100ms
  if (millis() - lastMotionCheck > 100) {
    lastMotionCheck = millis();
    motionDetected = accelerometer.detectMotion();
  }
#endif

  // Actualizar botón
#ifdef BUTTON_PIN
  button.update();

  // Cambiar efecto con el botón
  if (button.wasPressed()) {
    currentEffectIndex = (currentEffectIndex + 1) % NUM_EFFECTS;
    Serial.printf("Efecto cambiado a: %d\n", currentEffectIndex);

    // Cambiar al efecto seleccionado
    effects.stop();
    povEngine.stop();

    switch (currentEffectIndex) {
      case 0:  // LEDs apagados (ahorro de batería)
        ledController.clear();
        ledController.show();
        Serial.println("Efecto 0: LEDs apagados");
        break;
      case 1:  // Rainbow
        effects.rainbow(20);
        Serial.println("Efecto 1: Rainbow");
        break;
      case 2:  // Chase
        effects.colorChase(CRGB::Red, 50);
        Serial.println("Efecto 2: Chase rojo");
        break;
      case 3:  // POV (solo si hay movimiento)
        Serial.println("Efecto 3: POV mode");
        // Se activa automáticamente con movimiento más abajo
        break;
    }

    // Indicador visual: mostrar número de efecto con LEDs encendidos
    ledController.clear();
    for (int i = 0; i < currentEffectIndex && i < ledController.getNumLeds(); i++) {
      ledController.setPixel(i, CRGB::Blue);
    }
    ledController.show();
    delay(500);
    // Limpiar indicador tras mostrarlo
    ledController.clear();
    ledController.show();
  }
#endif

  // Si está en modo POV (efecto 3) y hay movimiento, activar POV
  if (currentEffectIndex == 3) {
#ifdef HAS_ACCELEROMETER
    // Detectar dirección del movimiento para ajustar el orden de columnas
    int8_t dir = accelerometer.getSweepDirection();
    if (dir != 0) {
      bool reverse = (dir < 0);  // dir negativo: invertir barrido
      povEngine.setReverseDirection(reverse);
      lastDirectionSign = dir;
    }

    if (motionDetected && !povEngine.isPlaying()) {
      // Iniciar POV si hay una imagen cargada
      if (strlen(config.activeImage) > 0) {
        povEngine.play();
        Serial.println("POV activado por movimiento");
      }
    } else if (!motionDetected) {
      if (povEngine.isPlaying()) {
        // Pausar POV si no hay movimiento
        povEngine.pause();
        Serial.println("POV pausado - sin movimiento");
      }
      // Asegurar LEDs apagados cuando está quieto
      ledController.clear();
      ledController.show();
    }
#endif
  }

  // Actualizar POV engine
  povEngine.update();

  // Actualizar efectos
  effects.update();

#else
  // ==== MODO NORMAL ====

  // Actualizar POV engine
  povEngine.update();

  // Actualizar efectos
  effects.update();

  // Actualizar MQTT
  if (config.mqttEnabled) {
    haIntegration.loop();
  }

  // Actualizar WiFi Manager
  wifiManager.loop();

  // Iniciar OTA si hay WiFi STA y aún no se inició
  if (wifiManager.isConnected() && !wifiManager.isAPMode() && !otaManager.isStarted()) {
    otaManager.begin(config.deviceName);
  }

  // Manejar OTA
  if (otaManager.isStarted()) {
    otaManager.loop();
  }
#endif

  // Delay mínimo para no saturar el loop
  // Reducir delay cuando POV está reproduciendo para maximizar FPS
  delay(povEngine.isPlaying() ? 0 : 1);
}

// Funciones de configuración
bool loadConfig() {
  if (!LittleFS.exists(CONFIG_FILE)) {
    return false;
  }

  File file = LittleFS.open(CONFIG_FILE, "r");
  if (!file) {
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error) {
    Serial.printf("Error parseando config JSON: %s\n", error.c_str());
    return false;
  }

  // Cargar valores
  if (doc.containsKey("wifiSSID"))
    strlcpy(config.wifiSSID, doc["wifiSSID"] | "", sizeof(config.wifiSSID));

  if (doc.containsKey("wifiPassword"))
    strlcpy(config.wifiPassword, doc["wifiPassword"] | "", sizeof(config.wifiPassword));

  config.wifiEnabled = doc["wifiEnabled"] | false;

  config.mqttEnabled = doc["mqttEnabled"] | false;

  if (doc.containsKey("mqttBroker"))
    strlcpy(config.mqttBroker, doc["mqttBroker"] | "", sizeof(config.mqttBroker));

  config.mqttPort = doc["mqttPort"] | MQTT_PORT;

  if (doc.containsKey("mqttUser"))
    strlcpy(config.mqttUser, doc["mqttUser"] | "", sizeof(config.mqttUser));

  if (doc.containsKey("mqttPassword"))
    strlcpy(config.mqttPassword, doc["mqttPassword"] | "", sizeof(config.mqttPassword));

  // LED configuration
  String ledTypeStr = doc["ledType"] | "WS2811";
  if (ledTypeStr == "WS2811" || ledTypeStr == "WS2812" || ledTypeStr == "WS2812B") {
    config.ledType = LED_TYPE_WS2811;
  } else if (ledTypeStr == "APA102") {
    config.ledType = LED_TYPE_APA102;
  } else {
    config.ledType = DEFAULT_LED_TYPE;
  }

  config.numLeds = doc["numLeds"] | DEFAULT_NUM_LEDS;
  config.brightness = doc["brightness"] | DEFAULT_BRIGHTNESS;

  config.povSpeed = doc["povSpeed"] | DEFAULT_POV_SPEED;
  config.loopMode = doc["loopMode"] | DEFAULT_LOOP_MODE;

  String orientStr = doc["povOrientation"] | "vertical";
  config.povOrientation = (orientStr == "horizontal") ? POV_HORIZONTAL : POV_VERTICAL;

  if (doc.containsKey("activeImage"))
    strlcpy(config.activeImage, doc["activeImage"] | "", sizeof(config.activeImage));

  if (doc.containsKey("deviceName"))
    strlcpy(config.deviceName, doc["deviceName"] | "POV-Line", sizeof(config.deviceName));

  return true;
}

bool saveConfig() {
  JsonDocument doc;

  doc["wifiSSID"] = config.wifiSSID;
  doc["wifiPassword"] = config.wifiPassword;
  doc["wifiEnabled"] = config.wifiEnabled;

  doc["mqttEnabled"] = config.mqttEnabled;
  doc["mqttBroker"] = config.mqttBroker;
  doc["mqttPort"] = config.mqttPort;
  doc["mqttUser"] = config.mqttUser;
  doc["mqttPassword"] = config.mqttPassword;

  // LED configuration
  String ledTypeStr = "WS2811";
  switch (config.ledType) {
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
  doc["numLeds"] = config.numLeds;
  doc["brightness"] = config.brightness;

  doc["povSpeed"] = config.povSpeed;
  doc["loopMode"] = config.loopMode;
  doc["povOrientation"] = (config.povOrientation == POV_VERTICAL) ? "vertical" : "horizontal";
  doc["activeImage"] = config.activeImage;

  doc["deviceName"] = config.deviceName;

  File file = LittleFS.open(CONFIG_FILE, "w");
  if (!file) {
    Serial.println("Error abriendo archivo de configuración para escritura");
    return false;
  }

  if (serializeJson(doc, file) == 0) {
    Serial.println("Error escribiendo configuración");
    file.close();
    return false;
  }

  file.close();
  Serial.println("Configuración guardada correctamente");
  return true;
}

void printConfig() {
  Serial.println("Configuración actual:");
  Serial.printf("  Device: %s\n", config.deviceName);
  Serial.printf("  LEDs: %d\n", config.numLeds);
  Serial.printf("  Brightness: %d\n", config.brightness);
  Serial.printf("  POV Speed: %d FPS\n", config.povSpeed);
  Serial.printf("  Loop Mode: %s\n", config.loopMode ? "ON" : "OFF");
  Serial.printf("  Orientation: %s\n", config.povOrientation == POV_VERTICAL ? "Vertical" : "Horizontal");
  Serial.printf("  WiFi: %s\n", config.wifiEnabled ? config.wifiSSID : "Disabled");
  Serial.printf("  MQTT: %s\n", config.mqttEnabled ? "Enabled" : "Disabled");
}
