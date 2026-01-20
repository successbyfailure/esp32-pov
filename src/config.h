#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Versión del firmware
#define FIRMWARE_VERSION "1.0.0"

// Pines GPIO para LEDs (definidos en platformio.ini según el board)
#ifndef LED_DATA_PIN
  #define LED_DATA_PIN 23  // Default para ESP32 clásico
#endif

#ifndef LED_CLOCK_PIN
  #define LED_CLOCK_PIN 18  // Default para ESP32 clásico (solo APA102)
#endif

// Tipos de tira LED soportados
enum LEDStripType {
  LED_TYPE_WS2811,   // WS2811/WS2812/WS2812B (1 pin - DATA)
  LED_TYPE_WS2812,   // Alias de WS2811
  LED_TYPE_WS2812B,  // Alias de WS2811
  LED_TYPE_APA102    // APA102/SK9822 (2 pines - DATA + CLOCK)
};

// Configuración de LEDs
#ifdef BORNHACK_BADGE
  #define DEFAULT_LED_TYPE LED_TYPE_WS2812
  #define DEFAULT_NUM_LEDS 16
  #define MAX_LEDS 16
#else
  #define DEFAULT_LED_TYPE LED_TYPE_WS2811
  #define DEFAULT_NUM_LEDS 144
  #define MAX_LEDS 300
#endif

#define MIN_LEDS 1
#define DEFAULT_BRIGHTNESS 51  // ~20% para setups sin badge
#define MAX_BRIGHTNESS 255

// Configuración de imágenes
#define MAX_IMAGE_WIDTH 128
#define MAX_IMAGE_HEIGHT MAX_LEDS
#define MAX_IMAGE_SIZE (100 * 1024)  // 100KB máximo por imagen
#define IMAGE_BUFFER_SIZE 1024

// Configuración POV
#define DEFAULT_POV_SPEED 30  // FPS de columnas
#define MIN_POV_SPEED 1
#define MAX_POV_SPEED 120
#define DEFAULT_LOOP_MODE true

// Orientación POV
enum POVOrientation {
  POV_VERTICAL,    // Columnas verticales (default)
  POV_HORIZONTAL   // Filas horizontales
};
#define DEFAULT_POV_ORIENTATION POV_VERTICAL

// WiFi
#define AP_SSID "POV-Line-Setup"
#define AP_PASSWORD "povline123"
#define WIFI_CONNECT_TIMEOUT 20000  // 20 segundos
#define WIFI_RETRY_INTERVAL 30000   // 30 segundos entre reintentos

// Web Server
#define WEB_SERVER_PORT 80
#define UPLOAD_BUFFER_SIZE 2048

// MQTT
#define MQTT_PORT 1883
#define MQTT_KEEPALIVE 60
#define MQTT_RECONNECT_DELAY 5000
#define MQTT_BASE_TOPIC "pov_line"
#define HA_DISCOVERY_PREFIX "homeassistant"

// Sistema de archivos
#define CONFIG_FILE "/config.json"
#define IMAGES_DIR "/images"

// Estructura de información de imagen
struct ImageInfo {
  char filename[32];
  uint16_t width;
  uint16_t height;
  uint32_t fileSize;
  uint8_t format;  // 0=BMP, 1=RGB565
  bool valid;

  ImageInfo() : width(0), height(0), fileSize(0), format(0), valid(false) {
    filename[0] = '\0';
  }
};

// Estructura de configuración global
struct Config {
  // WiFi
  char wifiSSID[32];
  char wifiPassword[64];
  bool wifiEnabled;

  // MQTT
  bool mqttEnabled;
  char mqttBroker[64];
  uint16_t mqttPort;
  char mqttUser[32];
  char mqttPassword[64];

  // LED
  LEDStripType ledType;
  uint16_t numLeds;
  uint8_t brightness;

  // POV
  uint16_t povSpeed;
  bool loopMode;
  POVOrientation povOrientation;
  char activeImage[32];

  // Sistema
  char deviceName[32];

  // Constructor con valores por defecto
  Config() {
    strcpy(wifiSSID, "");
    strcpy(wifiPassword, "");
    wifiEnabled = false;

    mqttEnabled = false;
    strcpy(mqttBroker, "");
    mqttPort = MQTT_PORT;
    strcpy(mqttUser, "");
    strcpy(mqttPassword, "");

    ledType = DEFAULT_LED_TYPE;
    numLeds = DEFAULT_NUM_LEDS;
    brightness = DEFAULT_BRIGHTNESS;

    povSpeed = DEFAULT_POV_SPEED;
    loopMode = DEFAULT_LOOP_MODE;
    povOrientation = DEFAULT_POV_ORIENTATION;
    strcpy(activeImage, "");

    strcpy(deviceName, "POV-Line");
  }
};

// Enumeración de estados del sistema
enum SystemState {
  STATE_IDLE,
  STATE_POV_PLAYING,
  STATE_POV_PAUSED,
  STATE_EFFECT_RUNNING,
  STATE_ERROR
};

// Enumeración de efectos
enum EffectType {
  EFFECT_NONE,
  EFFECT_RAINBOW,
  EFFECT_SOLID_COLOR,
  EFFECT_COLOR_CHASE,
  EFFECT_FADE
};

// Funciones de configuración global (definidas en main.cpp)
extern bool saveConfig();
extern bool loadConfig();

#endif
