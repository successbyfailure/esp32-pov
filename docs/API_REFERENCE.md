# API Reference - POV-Line

## Tabla de Contenidos

1. [REST API Endpoints](#rest-api-endpoints)
2. [MQTT Topics](#mqtt-topics)
3. [Estructuras de Datos](#estructuras-de-datos)
4. [C++ API Interna](#c-api-interna)

---

## REST API Endpoints

Base URL: `http://<ESP32_IP>/`

### GET /api/status

Obtiene el estado actual del sistema.

**Request:**
```http
GET /api/status HTTP/1.1
Host: 192.168.1.100
```

**Response:**
```json
{
  "state": "playing",              // "idle" | "playing" | "paused"
  "image": "/images/test.bmp",     // Path de imagen activa
  "column": 45,                    // Columna actual (0-based)
  "totalColumns": 128,             // Total de columnas
  "speed": 30,                     // FPS actual
  "brightness": 128,               // Brillo (0-255)
  "loopMode": true,                // Loop habilitado
  "orientation": "vertical",       // "vertical" | "horizontal"
  "effectRunning": false,          // Efecto activo
  "effectType": 0,                 // Tipo de efecto (enum)
  "wifiConnected": true,           // Estado WiFi
  "wifiSSID": "MiWiFi",           // SSID conectado
  "wifiIP": "192.168.1.100",      // IP asignada
  "freeSpace": 245760              // Espacio libre en bytes
}
```

**Status Codes:**
- `200 OK`: Success

---

### GET /api/images

Lista todas las imágenes almacenadas.

**Request:**
```http
GET /api/images HTTP/1.1
Host: 192.168.1.100
```

**Response:**
```json
{
  "images": [
    {
      "name": "test.bmp",
      "width": 128,
      "height": 144,
      "size": 55296,              // Bytes
      "format": "BMP"             // "BMP" | "RGB565"
    },
    {
      "name": "logo.rgb",
      "width": 100,
      "height": 144,
      "size": 28800,
      "format": "RGB565"
    }
  ],
  "freeSpace": 245760,            // Bytes disponibles
  "totalSpace": 1048576           // Bytes totales
}
```

**Status Codes:**
- `200 OK`: Success

---

### POST /api/upload

Sube una nueva imagen al sistema.

**Request:**
```http
POST /api/upload HTTP/1.1
Host: 192.168.1.100
Content-Type: multipart/form-data; boundary=----WebKitFormBoundary

------WebKitFormBoundary
Content-Disposition: form-data; name="file"; filename="test.bmp"
Content-Type: application/octet-stream

<binary data>
------WebKitFormBoundary--
```

**Response:**
```json
{
  "success": true
}
```

**Error Response:**
```json
{
  "success": false,
  "error": "Not enough space"
}
```

**Status Codes:**
- `200 OK`: Upload successful
- `500 Internal Server Error`: Upload failed

**Validaciones:**
- Tamaño máximo: 100 KB
- Extensiones válidas: .bmp, .rgb, .565
- Espacio disponible suficiente

---

### POST /api/play

Inicia la reproducción POV de una imagen.

**Request:**
```http
POST /api/play HTTP/1.1
Host: 192.168.1.100
Content-Type: application/x-www-form-urlencoded

image=test.bmp
```

**Parameters:**
- `image` (required): Nombre del archivo de imagen

**Response:**
```json
{
  "success": true
}
```

**Error Response:**
```json
{
  "error": "Failed to load image"
}
```

**Status Codes:**
- `200 OK`: Playback started
- `400 Bad Request`: Missing image parameter
- `500 Internal Server Error`: Failed to load image

---

### POST /api/pause

Pausa la reproducción POV actual.

**Request:**
```http
POST /api/pause HTTP/1.1
Host: 192.168.1.100
```

**Response:**
```json
{
  "success": true
}
```

**Status Codes:**
- `200 OK`: Paused successfully

---

### POST /api/stop

Detiene completamente la reproducción POV y efectos.

**Request:**
```http
POST /api/stop HTTP/1.1
Host: 192.168.1.100
```

**Response:**
```json
{
  "success": true
}
```

**Status Codes:**
- `200 OK`: Stopped successfully

---

### POST /api/settings

Actualiza configuración del sistema.

**Request:**
```http
POST /api/settings HTTP/1.1
Host: 192.168.1.100
Content-Type: application/x-www-form-urlencoded

speed=60&brightness=200&loop=true&orientation=horizontal
```

**Parameters** (todos opcionales):
- `speed`: Velocidad POV en FPS (1-120)
- `brightness`: Brillo global (0-255)
- `loop`: Modo loop ("true" | "false")
- `orientation`: Orientación ("vertical" | "horizontal")

**Response:**
```json
{
  "success": true
}
```

**Error Response:**
```json
{
  "error": "No parameters provided"
}
```

**Status Codes:**
- `200 OK`: Settings updated
- `400 Bad Request`: No valid parameters

---

### GET /api/effects

Lista efectos disponibles.

**Request:**
```http
GET /api/effects HTTP/1.1
Host: 192.168.1.100
```

**Response:**
```json
{
  "effects": [
    "rainbow",
    "solid",
    "chase",
    "off"
  ]
}
```

**Status Codes:**
- `200 OK`: Success

---

### POST /api/effect

Activa un efecto decorativo.

**Request:**
```http
POST /api/effect HTTP/1.1
Host: 192.168.1.100
Content-Type: application/x-www-form-urlencoded

effect=rainbow&speed=15
```

**Parameters:**

**Para "rainbow":**
- `effect`: "rainbow" (required)
- `speed`: Velocidad (1-255, default: 10)

**Para "solid":**
- `effect`: "solid" (required)
- `r`: Rojo (0-255, default: 255)
- `g`: Verde (0-255, default: 255)
- `b`: Azul (0-255, default: 255)

**Para "chase":**
- `effect`: "chase" (required)
- `r`, `g`, `b`: Color RGB (0-255)
- `speed`: Velocidad (1-255, default: 50)

**Para "off":**
- `effect`: "off" (required)

**Response:**
```json
{
  "success": true
}
```

**Error Response:**
```json
{
  "error": "Unknown effect"
}
```

**Status Codes:**
- `200 OK`: Effect activated
- `400 Bad Request`: Invalid effect or parameters

---

### POST /api/image/delete

Elimina una imagen.

**Request:**
```http
POST /api/image/delete HTTP/1.1
Host: 192.168.1.100
Content-Type: application/x-www-form-urlencoded

image=test.bmp
```

**Parameters:**
- `image` (required): Nombre del archivo a eliminar

**Response:**
```json
{
  "success": true
}
```

**Error Response:**
```json
{
  "error": "Failed to delete image"
}
```

**Status Codes:**
- `200 OK`: Deleted successfully
- `400 Bad Request`: Missing image parameter
- `500 Internal Server Error`: Delete failed

---

### GET /api/config

Obtiene la configuración completa del sistema.

**Request:**
```http
GET /api/config HTTP/1.1
Host: 192.168.1.100
```

**Response:**
```json
{
  "deviceName": "POV-Line",
  "numLeds": 144,
  "brightness": 128,
  "povSpeed": 30,
  "loopMode": true,
  "povOrientation": "vertical",
  "wifiSSID": "MiWiFi",
  "mqttEnabled": true,
  "mqttBroker": "192.168.1.10",
  "mqttPort": 1883
}
```

**Status Codes:**
- `200 OK`: Success

---

### POST /api/config

Guarda configuración completa del sistema.

**Request:**
```http
POST /api/config HTTP/1.1
Host: 192.168.1.100
Content-Type: application/x-www-form-urlencoded

deviceName=MyPOV&numLeds=144&wifiSSID=NewWiFi&wifiPassword=pass123&mqttEnabled=true&mqttBroker=192.168.1.10&mqttPort=1883
```

**Parameters:**
- `deviceName`: Nombre del dispositivo
- `numLeds`: Número de LEDs (1-300)
- `wifiSSID`: SSID WiFi
- `wifiPassword`: Password WiFi
- `mqttEnabled`: MQTT habilitado ("true" | "false")
- `mqttBroker`: IP del broker MQTT
- `mqttPort`: Puerto MQTT (default: 1883)

**Response:**
```json
{
  "success": true
}
```

**Status Codes:**
- `200 OK`: Config saved (device will reboot)

**Note:** El dispositivo se reiniciará después de guardar la configuración.

---

## MQTT Topics

### State Topics (Published by Device)

#### pov_line/state
Estado principal ON/OFF.

**Payload:**
```
ON
```
o
```
OFF
```

**Retained:** Yes

---

#### pov_line/brightness/state
Nivel de brillo actual.

**Payload:**
```
128
```
(0-255)

**Retained:** Yes

---

#### pov_line/effect/state
Efecto activo actualmente.

**Payload:**
```
POV
```
o
```
Rainbow
```
o
```
Solid Color
```
o
```
Chase
```
o
```
None
```

**Retained:** Yes

---

### Command Topics (Subscribed by Device)

#### pov_line/command
Encender o apagar el dispositivo.

**Payload:**
```
ON
```
→ Activa efecto rainbow por defecto

```
OFF
```
→ Apaga todos los LEDs

---

#### pov_line/brightness/set
Ajustar brillo.

**Payload:**
```
200
```
(0-255)

---

#### pov_line/effect/set
Cambiar efecto activo.

**Payload:**
```
pov
```
→ Inicia POV (si hay imagen cargada)

```
rainbow
```
→ Efecto rainbow

```
solid color
```
→ Color sólido blanco

```
chase
```
→ Efecto chase rojo

---

### Discovery Topic

#### homeassistant/light/pov_line_XXXXXXXX/config

Publicado al conectar a MQTT para autodiscovery en Home Assistant.

**Payload:**
```json
{
  "name": "POV Line",
  "unique_id": "pov_line_12345678",
  "state_topic": "pov_line/state",
  "command_topic": "pov_line/command",
  "brightness_state_topic": "pov_line/brightness/state",
  "brightness_command_topic": "pov_line/brightness/set",
  "brightness_scale": 255,
  "effect_state_topic": "pov_line/effect/state",
  "effect_command_topic": "pov_line/effect/set",
  "effect_list": ["POV", "Rainbow", "Solid Color", "Chase"],
  "device": {
    "identifiers": ["pov_line_12345678"],
    "name": "POV Line",
    "model": "POV Line",
    "manufacturer": "Custom",
    "sw_version": "1.0.0"
  }
}
```

**Retained:** Yes

---

## Estructuras de Datos

### ImageInfo (C++ struct)

```cpp
struct ImageInfo {
  char filename[32];      // Nombre del archivo
  uint16_t width;         // Ancho en píxeles
  uint16_t height;        // Alto en píxeles
  uint32_t fileSize;      // Tamaño en bytes
  uint8_t format;         // 0=BMP, 1=RGB565
  bool valid;             // Datos válidos

  ImageInfo();            // Constructor con valores por defecto
};
```

---

### Config (C++ struct)

```cpp
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
  uint16_t numLeds;
  uint8_t brightness;

  // POV
  uint16_t povSpeed;
  bool loopMode;
  POVOrientation povOrientation;  // POV_VERTICAL | POV_HORIZONTAL
  char activeImage[32];

  // Sistema
  char deviceName[32];

  Config();  // Constructor con valores por defecto
};
```

---

### POVOrientation (C++ enum)

```cpp
enum POVOrientation {
  POV_VERTICAL,     // Columnas verticales (default)
  POV_HORIZONTAL    // Filas horizontales
};
```

---

### SystemState (C++ enum)

```cpp
enum SystemState {
  STATE_IDLE,           // Inactivo
  STATE_POV_PLAYING,    // Reproduciendo POV
  STATE_POV_PAUSED,     // POV pausado
  STATE_EFFECT_RUNNING, // Ejecutando efecto
  STATE_ERROR           // Estado de error
};
```

---

### EffectType (C++ enum)

```cpp
enum EffectType {
  EFFECT_NONE,          // Sin efecto
  EFFECT_RAINBOW,       // Arco iris
  EFFECT_SOLID_COLOR,   // Color sólido
  EFFECT_COLOR_CHASE,   // Persecución de color
  EFFECT_FADE           // Desvanecimiento
};
```

---

## C++ API Interna

### LEDController

```cpp
class LEDController {
public:
  bool init(uint16_t numLeds = DEFAULT_NUM_LEDS);
  void setPixel(uint16_t index, CRGB color);
  void setPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
  void setBrightness(uint8_t value);
  uint8_t getBrightness();
  void clear();
  void fill(CRGB color);
  void show();
  CRGB* getPixels();
  uint16_t getNumLeds();
  bool isInitialized();
};

extern LEDController ledController;
```

**Uso:**
```cpp
ledController.init(144);
ledController.setBrightness(128);
ledController.setPixel(0, CRGB::Red);
ledController.setPixel(1, 255, 0, 0);
ledController.show();
```

---

### POVEngine

```cpp
class POVEngine {
public:
  bool loadImage(const char* filename);
  void unloadImage();
  bool isImageLoaded();

  void play();
  void pause();
  void stop();
  void resume();

  bool isPlaying();
  bool isPaused();

  void setSpeed(uint16_t fps);
  uint16_t getSpeed();

  void setLoopMode(bool loop);
  bool getLoopMode();

  void setOrientation(POVOrientation orient);
  POVOrientation getOrientation();

  void update();

  const char* getCurrentImageName();
  uint16_t getCurrentColumn();
  uint16_t getTotalColumns();
};

extern POVEngine povEngine;
```

**Uso:**
```cpp
if (povEngine.loadImage("test.bmp")) {
  povEngine.setSpeed(30);
  povEngine.setLoopMode(true);
  povEngine.setOrientation(POV_VERTICAL);
  povEngine.play();
}

void loop() {
  povEngine.update();  // Llamar en cada iteración
}
```

---

### ImageParser

```cpp
class ImageParser {
public:
  bool parseImageInfo(const char* filename, ImageInfo& info);
  bool parseBMP(File& file, ImageInfo& info);
  bool parseRGB565(File& file, ImageInfo& info);

  bool getColumn(const char* filename, uint16_t columnIndex,
                 CRGB* buffer, uint16_t bufferSize);
  bool getColumnBMP(File& file, const ImageInfo& info,
                    uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize);
  bool getColumnRGB565(File& file, const ImageInfo& info,
                       uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize);
};

extern ImageParser imageParser;
```

**Uso:**
```cpp
ImageInfo info;
if (imageParser.parseImageInfo("/images/test.bmp", info)) {
  Serial.printf("Image: %dx%d, %s\n",
                info.width, info.height,
                info.format == 0 ? "BMP" : "RGB565");

  CRGB columnBuffer[144];
  if (imageParser.getColumn("/images/test.bmp", 0, columnBuffer, 144)) {
    // Procesar primera columna
  }
}
```

---

### ImageManager

```cpp
class ImageManager {
public:
  bool init();
  std::vector<ImageInfo> listImages();
  bool deleteImage(const char* filename);
  bool getImageInfo(const char* filename, ImageInfo& info);
  size_t getFreeSpace();
  size_t getTotalSpace();
  size_t getUsedSpace();
  bool imageExists(const char* filename);
  void refreshList();
};

extern ImageManager imageManager;
```

**Uso:**
```cpp
imageManager.init();

auto images = imageManager.listImages();
for (const auto& img : images) {
  Serial.printf("Image: %s (%dx%d)\n",
                img.filename, img.width, img.height);
}

Serial.printf("Free space: %d bytes\n", imageManager.getFreeSpace());
```

---

### Effects

```cpp
class Effects {
public:
  void update();
  void stop();
  bool isRunning();
  EffectType getCurrentEffect();

  void rainbow(uint8_t speed = 10);
  void solidColor(CRGB color);
  void solidColor(uint8_t r, uint8_t g, uint8_t b);
  void colorChase(CRGB color, uint8_t speed = 50);
  void fade(CRGB fromColor, CRGB toColor, uint16_t duration = 2000);
};

extern Effects effects;
```

**Uso:**
```cpp
effects.rainbow(15);

void loop() {
  effects.update();  // Llamar en cada iteración
}

// Cambiar efecto
effects.stop();
effects.solidColor(255, 0, 0);  // Rojo
```

---

### WiFiManager

```cpp
class WiFiManager {
public:
  void init();
  void loop();

  bool startAP(const char* ssid = AP_SSID,
               const char* password = AP_PASSWORD);
  bool connectWiFi(const char* ssid, const char* password);
  void disconnect();

  bool isConnected();
  bool isAPMode();
  String getIP();
  String getSSID();
  int getRSSI();
};

extern WiFiManager wifiManager;
```

**Uso:**
```cpp
wifiManager.init();

if (wifiManager.connectWiFi("MyWiFi", "password123")) {
  Serial.println("Connected!");
  Serial.println(wifiManager.getIP());
} else {
  wifiManager.startAP();
  Serial.println("AP Mode started");
}
```

---

### WebServer

```cpp
class WebServer {
public:
  void init();
  void setupRoutes();

private:
  void handleStatus(AsyncWebServerRequest *request);
  void handleImages(AsyncWebServerRequest *request);
  void handlePlay(AsyncWebServerRequest *request);
  // ... más handlers
};

extern WebServer webServer;
```

**Uso:**
```cpp
webServer.init();
// El servidor corre automáticamente en background
```

---

### HAIntegration

```cpp
class HAIntegration {
public:
  void init(const char* broker, uint16_t port,
            const char* user = "", const char* password = "");
  void loop();

  bool isConnected();
  void publishState();
  void publishDiscovery();
};

extern HAIntegration haIntegration;
```

**Uso:**
```cpp
haIntegration.init("192.168.1.10", 1883);

void loop() {
  haIntegration.loop();  // Procesar MQTT
}

// Publicar estado cuando cambia
haIntegration.publishState();
```

---

## Códigos de Error

### HTTP Status Codes

- `200 OK`: Operación exitosa
- `400 Bad Request`: Parámetros inválidos o faltantes
- `404 Not Found`: Recurso no encontrado
- `500 Internal Server Error`: Error interno del servidor

### Serial Debug Messages

**Formato:** `[COMPONENTE] Mensaje`

**Ejemplos:**
```
[LED] LEDs inicializados: 144 LEDs en pines DATA=23, CLOCK=18
[POV] Imagen cargada: test.bmp (128x144)
[WiFi] Conectado a WiFi. IP: 192.168.1.100
[MQTT] Conectado a MQTT
[ERROR] No se pudo leer columna 45
```

---

## Rate Limits y Timeouts

### HTTP
- No hay rate limiting implementado
- Timeout de conexión: Default de AsyncTCP

### MQTT
- Reconnect delay: 5 segundos
- Keepalive: 60 segundos

### WiFi
- Connect timeout: 20 segundos
- Retry interval: 30 segundos

---

## Versioning

**Formato:** MAJOR.MINOR.PATCH

**Version actual:** 1.0.0

**Cambios que incrementan cada número:**
- **MAJOR**: Cambios incompatibles en API
- **MINOR**: Nueva funcionalidad compatible
- **PATCH**: Bug fixes compatibles
