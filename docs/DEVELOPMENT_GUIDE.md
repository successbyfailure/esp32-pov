# Guía de Desarrollo - POV-Line

## Para Agentes de Programación y Desarrolladores

Esta guía proporciona información detallada sobre cómo trabajar con el código base, implementar nuevas funcionalidades y debugging del sistema.

---

## Tabla de Contenidos

1. [Configuración del Entorno](#configuración-del-entorno)
2. [Estructura del Código](#estructura-del-código)
3. [Patrones de Código](#patrones-de-código)
4. [Cómo Añadir Funcionalidades](#cómo-añadir-funcionalidades)
5. [Testing y Debugging](#testing-y-debugging)
6. [Optimización](#optimización)
7. [Troubleshooting](#troubleshooting)

---

## Configuración del Entorno

### Requisitos

- **PlatformIO Core** o **PlatformIO IDE** (VSCode extension)
- **Python 3.x** (para PlatformIO)
- **Git** (para control de versiones)

### Instalación de PlatformIO

```bash
# Via pip
pip install platformio

# O usar el binario local ya instalado
~/.platformio/penv/bin/pio --version
```

### Clonar y Configurar Proyecto

```bash
cd /ruta/a/proyectos
git init pov-line
cd pov-line

# Inicializar PlatformIO (si es nuevo)
pio project init --board esp32dev

# Instalar dependencias
pio lib install
```

### Estructura de Directorios

```
pov-line/
├── src/                    # Código fuente C++
├── include/                # Headers públicos (vacío en este proyecto)
├── lib/                    # Librerías locales (vacío)
├── data/                   # Archivos para LittleFS
│   ├── index.html
│   ├── style.css
│   └── app.js
├── docs/                   # Documentación
├── test/                   # Tests unitarios
├── .pio/                   # Build artifacts (ignorar en git)
├── platformio.ini          # Configuración del proyecto
└── README.md
```

---

## Estructura del Código

### Headers (.h files)

**Ubicación:** `src/*.h`

**Patrón:**
```cpp
#ifndef NOMBRE_H
#define NOMBRE_H

#include <Arduino.h>
#include "dependencies.h"

class MiClase {
private:
  // Variables privadas
  int variable;

public:
  // Constructor
  MiClase();

  // Métodos públicos
  void metodo();
};

// Instancia global (si aplica)
extern MiClase instanciaGlobal;

#endif
```

### Implementation (.cpp files)

**Ubicación:** `src/*.cpp`

**Patrón:**
```cpp
#include "mi_clase.h"

MiClase::MiClase() : variable(0) {
  // Inicialización
}

void MiClase::metodo() {
  // Implementación
}

// Instancia global
MiClase instanciaGlobal;
```

### Variables Globales

**Ubicación:** Declaradas como `extern` en headers, definidas en .cpp

**Razón:** Permite acceso desde múltiples módulos sin duplicación.

**Instancias Globales Actuales:**
```cpp
extern LEDController ledController;
extern POVEngine povEngine;
extern Effects effects;
extern ImageManager imageManager;
extern ImageParser imageParser;
extern WiFiManager wifiManager;
extern WebServer webServer;
extern HAIntegration haIntegration;
extern Config config;  // en main.cpp
```

---

## Patrones de Código

### 1. Singleton Pattern (Instancias Globales)

**Por qué:** Hardware único, estado compartido necesario.

**Ejemplo:**
```cpp
// led_controller.h
class LEDController {
  // ...
};
extern LEDController ledController;

// led_controller.cpp
LEDController ledController;  // Una sola instancia
```

**Uso:**
```cpp
// Desde cualquier parte del código
ledController.setBrightness(128);
```

---

### 2. Non-Blocking Update Pattern

**Por qué:** ESP32 single-threaded, watchdog timer requiere yields.

**Ejemplo:**
```cpp
class MiClase {
private:
  unsigned long lastUpdate;
  unsigned long updateInterval;

public:
  void update() {
    unsigned long currentTime = millis();

    if (currentTime - lastUpdate < updateInterval) {
      return;  // No es tiempo de actualizar
    }

    // Realizar actualización
    doWork();

    lastUpdate = currentTime;
  }
};

// En loop()
void loop() {
  miClase.update();
  delay(1);  // Yield para watchdog
}
```

**Ventajas:**
- No bloquea otras tareas
- Permite múltiples componentes actualizándose
- Evita watchdog reset

---

### 3. State Machine Pattern

**Por qué:** Gestión clara de estados del sistema.

**Ejemplo:**
```cpp
enum SystemState {
  STATE_IDLE,
  STATE_PLAYING,
  STATE_PAUSED
};

class StateMachine {
private:
  SystemState currentState;

public:
  void setState(SystemState newState) {
    // Transición de estado
    exitState(currentState);
    currentState = newState;
    enterState(newState);
  }

  void update() {
    switch (currentState) {
      case STATE_IDLE:
        updateIdle();
        break;
      case STATE_PLAYING:
        updatePlaying();
        break;
      // ...
    }
  }
};
```

---

### 4. Error Handling Pattern

**Estrategia:** Return false + Serial logging.

**Ejemplo:**
```cpp
bool operationThatMightFail() {
  if (!precondition) {
    Serial.println("Error: Precondición no cumplida");
    return false;
  }

  if (!doWork()) {
    Serial.println("Error: Trabajo falló");
    return false;
  }

  return true;
}

// Uso
if (!operationThatMightFail()) {
  // Manejar error
  fallbackBehavior();
}
```

**No usamos excepciones** porque:
- Overhead en memoria
- Arduino framework no las usa
- Return codes más simples en embedded

---

### 5. Resource Management Pattern

**Para archivos:**
```cpp
bool processFile(const char* filename) {
  File file = LittleFS.open(filename, "r");

  if (!file) {
    return false;
  }

  // Procesar archivo
  bool result = processData(file);

  file.close();  // SIEMPRE cerrar
  return result;
}
```

**Para memoria dinámica:**
```cpp
class MiClase {
private:
  uint8_t* buffer;

public:
  MiClase() : buffer(nullptr) {}

  ~MiClase() {
    if (buffer != nullptr) {
      delete[] buffer;
    }
  }

  bool init(size_t size) {
    buffer = new uint8_t[size];
    return (buffer != nullptr);
  }
};
```

---

## Cómo Añadir Funcionalidades

### Añadir Nuevo Efecto LED

**Paso 1:** Definir enum en `config.h`
```cpp
enum EffectType {
  EFFECT_NONE,
  EFFECT_RAINBOW,
  EFFECT_SOLID_COLOR,
  EFFECT_COLOR_CHASE,
  EFFECT_FADE,
  EFFECT_SPARKLE  // <- Nuevo efecto
};
```

**Paso 2:** Declarar métodos en `effects.h`
```cpp
class Effects {
public:
  // ...
  void sparkle(CRGB color, uint8_t density);

private:
  void updateSparkle();
};
```

**Paso 3:** Implementar en `effects.cpp`
```cpp
void Effects::sparkle(CRGB color, uint8_t density) {
  currentEffect = EFFECT_SPARKLE;
  effectColor = color;
  effectSpeed = density;
  running = true;
}

void Effects::updateSparkle() {
  unsigned long currentTime = millis();
  if (currentTime - lastUpdate < 50) return;

  // Apagar todos
  ledController.clear();

  // Encender LEDs aleatorios
  uint16_t numLeds = ledController.getNumLeds();
  for (int i = 0; i < effectSpeed; i++) {
    uint16_t pos = random(0, numLeds);
    ledController.setPixel(pos, effectColor);
  }

  ledController.show();
  lastUpdate = currentTime;
}
```

**Paso 4:** Añadir case en `update()`
```cpp
void Effects::update() {
  // ...
  switch (currentEffect) {
    case EFFECT_SPARKLE:
      updateSparkle();
      break;
    // ...
  }
}
```

**Paso 5:** Añadir endpoint en `web_server.cpp`
```cpp
void WebServer::handleEffect(AsyncWebServerRequest *request) {
  String effectName = request->getParam("effect", true)->value();

  // ...
  else if (effectName == "sparkle") {
    uint8_t r = request->hasParam("r", true) ? request->getParam("r", true)->value().toInt() : 255;
    uint8_t g = request->hasParam("g", true) ? request->getParam("g", true)->value().toInt() : 255;
    uint8_t b = request->hasParam("b", true) ? request->getParam("b", true)->value().toInt() : 255;
    uint8_t density = request->hasParam("density", true) ? request->getParam("density", true)->value().toInt() : 10;
    effects.sparkle(CRGB(r, g, b), density);
  }
  // ...
}
```

**Paso 6:** Actualizar lista de efectos
```cpp
String WebServer::getEffectsJSON() {
  JsonDocument doc;
  JsonArray effectsArray = doc["effects"].to<JsonArray>();

  effectsArray.add("rainbow");
  effectsArray.add("solid");
  effectsArray.add("chase");
  effectsArray.add("sparkle");  // <- Añadir
  effectsArray.add("off");

  String json;
  serializeJson(doc, json);
  return json;
}
```

**Paso 7:** Actualizar UI en `app.js`
```javascript
// En la función setEffect()
function setSparkle() {
    const color = document.getElementById('color-picker').value;
    const r = parseInt(color.substr(1, 2), 16);
    const g = parseInt(color.substr(3, 2), 16);
    const b = parseInt(color.substr(5, 2), 16);
    const density = 10;

    const formData = new FormData();
    formData.append('effect', 'sparkle');
    formData.append('r', r);
    formData.append('g', g);
    formData.append('b', b);
    formData.append('density', density);

    fetch('/api/effect', {
        method: 'POST',
        body: formData
    });
}
```

**Paso 8:** Añadir botón en `index.html`
```html
<button class="btn btn-effect" onclick="setEffect('sparkle')">✨ Sparkle</button>
```

---

### Añadir Nuevo Formato de Imagen

**Paso 1:** Definir header en `image_parser.h`
```cpp
#pragma pack(push, 1)
struct JPEGHeader {
  uint8_t marker[2];      // FF D8
  // ... resto de header
};
#pragma pack(pop)
```

**Paso 2:** Implementar parser en `image_parser.cpp`
```cpp
bool ImageParser::parseJPEG(File& file, ImageInfo& info) {
  JPEGHeader header;

  file.seek(0);
  if (file.read((uint8_t*)&header, sizeof(JPEGHeader)) != sizeof(JPEGHeader)) {
    return false;
  }

  // Validar marker
  if (header.marker[0] != 0xFF || header.marker[1] != 0xD8) {
    return false;
  }

  // Extraer dimensiones
  // ... decodificar JPEG
  info.width = extractedWidth;
  info.height = extractedHeight;
  info.valid = true;

  return true;
}
```

**Paso 3:** Implementar lectura de columnas
```cpp
bool ImageParser::getColumnJPEG(File& file, const ImageInfo& info,
                                 uint16_t columnIndex, CRGB* buffer,
                                 uint16_t bufferSize) {
  // Decodificar JPEG y extraer columna
  // Esto es complejo - considerar usar librería TJpgDec

  return true;
}
```

**Paso 4:** Integrar en `parseImageInfo()`
```cpp
bool ImageParser::parseImageInfo(const char* filename, ImageInfo& info) {
  String fn = String(filename);
  fn.toLowerCase();

  if (fn.endsWith(".bmp")) {
    result = parseBMP(file, info);
    info.format = 0;
  } else if (fn.endsWith(".rgb") || fn.endsWith(".565")) {
    result = parseRGB565(file, info);
    info.format = 1;
  } else if (fn.endsWith(".jpg") || fn.endsWith(".jpeg")) {
    result = parseJPEG(file, info);
    info.format = 2;  // Nuevo formato
  }
  // ...
}
```

**Paso 5:** Actualizar `getColumn()`
```cpp
bool ImageParser::getColumn(const char* filename, uint16_t columnIndex,
                             CRGB* buffer, uint16_t bufferSize) {
  // ...
  if (info.format == 0) {
    result = getColumnBMP(file, info, columnIndex, buffer, bufferSize);
  } else if (info.format == 1) {
    result = getColumnRGB565(file, info, columnIndex, buffer, bufferSize);
  } else if (info.format == 2) {
    result = getColumnJPEG(file, info, columnIndex, buffer, bufferSize);
  }
  // ...
}
```

**Paso 6:** Actualizar validación en `image_manager.cpp`
```cpp
bool ImageManager::isImageFile(const char* filename) {
  String fn = String(filename);
  fn.toLowerCase();
  return fn.endsWith(".bmp") ||
         fn.endsWith(".rgb") ||
         fn.endsWith(".565") ||
         fn.endsWith(".jpg") ||  // <- Añadir
         fn.endsWith(".jpeg");   // <- Añadir
}
```

**Paso 7:** Actualizar UI y documentación

---

### Añadir Nuevo Endpoint API

**Paso 1:** Declarar handler en `web_server.h`
```cpp
class WebServer {
private:
  void handleNewEndpoint(AsyncWebServerRequest *request);
};
```

**Paso 2:** Implementar handler en `web_server.cpp`
```cpp
void WebServer::handleNewEndpoint(AsyncWebServerRequest *request) {
  // Validar parámetros
  if (!request->hasParam("param", true)) {
    request->send(400, "application/json",
                  "{\"error\":\"Missing parameter\"}");
    return;
  }

  String param = request->getParam("param", true)->value();

  // Procesar
  bool success = doSomething(param);

  // Responder
  if (success) {
    request->send(200, "application/json", "{\"success\":true}");
  } else {
    request->send(500, "application/json",
                  "{\"error\":\"Operation failed\"}");
  }
}
```

**Paso 3:** Registrar ruta en `setupRoutes()`
```cpp
void WebServer::setupRoutes() {
  // ...

  server->on("/api/newendpoint", HTTP_POST,
    [this](AsyncWebServerRequest *request) {
      this->handleNewEndpoint(request);
    }
  );

  // ...
}
```

**Paso 4:** Documentar en API_REFERENCE.md

---

## Testing y Debugging

### Serial Debugging

**Habilitar:**
```cpp
void setup() {
  Serial.begin(115200);
  delay(1000);  // Esperar a que Serial esté listo
}
```

**Usar:**
```cpp
Serial.println("Mensaje simple");
Serial.printf("Valor: %d, String: %s\n", numero, cadena);

// Con prefijo de componente
Serial.println("[POV] Imagen cargada correctamente");
```

**Ver output:**
```bash
~/.platformio/penv/bin/pio device monitor
```

---

### Memory Debugging

**Verificar heap libre:**
```cpp
void printMemoryStats() {
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Heap size: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Min free heap: %d bytes\n", ESP.getMinFreeHeap());
}
```

**Verificar LittleFS:**
```cpp
void printFSStats() {
  Serial.printf("FS Total: %d bytes\n", LittleFS.totalBytes());
  Serial.printf("FS Used: %d bytes\n", LittleFS.usedBytes());
  Serial.printf("FS Free: %d bytes\n",
                LittleFS.totalBytes() - LittleFS.usedBytes());
}
```

**Llamar periódicamente:**
```cpp
void loop() {
  static unsigned long lastMemCheck = 0;

  if (millis() - lastMemCheck > 10000) {  // Cada 10s
    printMemoryStats();
    lastMemCheck = millis();
  }

  // ... resto del loop
}
```

---

### Testing con cURL

**Test de status:**
```bash
curl http://192.168.1.100/api/status | jq
```

**Test de upload:**
```bash
curl -F "file=@test.bmp" http://192.168.1.100/api/upload
```

**Test de settings:**
```bash
curl -X POST \
  -d "speed=60&brightness=200&loop=true" \
  http://192.168.1.100/api/settings
```

**Test de efecto:**
```bash
curl -X POST \
  -d "effect=rainbow&speed=20" \
  http://192.168.1.100/api/effect
```

---

### Unit Testing

**Framework:** Unity (incluido en PlatformIO)

**Crear test:** `test/test_image_parser.cpp`
```cpp
#include <unity.h>
#include "image_parser.h"

void setUp(void) {
  // Configuración antes de cada test
}

void tearDown(void) {
  // Limpieza después de cada test
}

void test_parse_bmp_header() {
  ImageInfo info;
  // ... setup test file

  TEST_ASSERT_TRUE(imageParser.parseBMP(file, info));
  TEST_ASSERT_EQUAL(128, info.width);
  TEST_ASSERT_EQUAL(144, info.height);
}

void test_invalid_bmp() {
  ImageInfo info;
  // ... setup invalid file

  TEST_ASSERT_FALSE(imageParser.parseBMP(file, info));
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_parse_bmp_header);
  RUN_TEST(test_invalid_bmp);
  UNITY_END();
}

void loop() {
  // Empty
}
```

**Ejecutar:**
```bash
~/.platformio/penv/bin/pio test
```

---

## Optimización

### 1. Optimizar Memoria

**Usar PROGMEM para constantes:**
```cpp
const char htmlPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
...
</html>
)=====";

// Leer con
String page = FPSTR(htmlPage);
```

**Reducir buffers:**
```cpp
// En lugar de
char buffer[1024];

// Usar tamaño mínimo necesario
char buffer[256];
```

**Compartir buffers:**
```cpp
// Global shared buffer
uint8_t sharedBuffer[1024];

void function1() {
  // Usar sharedBuffer
}

void function2() {
  // Reusar sharedBuffer (asegurar que function1 terminó)
}
```

---

### 2. Optimizar CPU

**Evitar división en loops:**
```cpp
// Lento
for (int i = 0; i < 1000; i++) {
  int result = value / 3;
}

// Rápido
int divider = value / 3;
for (int i = 0; i < 1000; i++) {
  int result = divider;
}
```

**Usar bit shifts en lugar de multiplicación/división por potencias de 2:**
```cpp
// En lugar de
int result = value * 8;

// Usar
int result = value << 3;

// En lugar de
int result = value / 4;

// Usar
int result = value >> 2;
```

**Cache valores de funciones costosas:**
```cpp
// En lugar de
for (int i = 0; i < ledController.getNumLeds(); i++) {
  // ...
}

// Usar
uint16_t numLeds = ledController.getNumLeds();
for (int i = 0; i < numLeds; i++) {
  // ...
}
```

---

### 3. Optimizar SPI/LEDs

**Batch updates:**
```cpp
// Lento
for (int i = 0; i < 144; i++) {
  ledController.setPixel(i, color);
  ledController.show();  // ¡NO hacer esto!
}

// Rápido
for (int i = 0; i < 144; i++) {
  ledController.setPixel(i, color);
}
ledController.show();  // Una sola llamada
```

**Acceso directo al buffer:**
```cpp
// En lugar de
for (int i = 0; i < 144; i++) {
  ledController.setPixel(i, color);
}

// Usar
CRGB* pixels = ledController.getPixels();
for (int i = 0; i < 144; i++) {
  pixels[i] = color;
}
ledController.show();
```

---

## Troubleshooting

### Problema: No compila

**Error: Library not found**
```bash
# Solución
~/.platformio/penv/bin/pio lib install
```

**Error: Outdated framework**
```bash
# Solución
~/.platformio/penv/bin/pio platform update espressif32
```

---

### Problema: No se conecta a WiFi

**Debug:**
```cpp
WiFi.setDebugOutput(true);  // En setup()
```

**Verificar:**
- SSID correcto
- Password correcto
- Red 2.4GHz (ESP32 no soporta 5GHz)
- Señal suficiente

---

### Problema: Watchdog Reset

**Síntoma:** ESP32 se reinicia constantemente

**Causa:** Loop bloqueante

**Solución:**
```cpp
void loop() {
  // Evitar delay() largos
  // Usar millis() para timing
  // Añadir yield() o delay(1) al final

  povEngine.update();
  effects.update();
  delay(1);  // IMPORTANTE: yield para watchdog
}
```

---

### Problema: LittleFS lleno

**Verificar:**
```cpp
Serial.printf("FS Free: %d\n", imageManager.getFreeSpace());
```

**Solución:**
```bash
# Reformatear filesystem
# En setup():
LittleFS.format();
```

---

### Problema: Imágenes corruptas

**Debug:**
```cpp
ImageInfo info;
if (!imageParser.parseImageInfo(filename, info)) {
  Serial.println("Failed to parse");
} else {
  Serial.printf("Image: %dx%d, size=%d, format=%d\n",
                info.width, info.height, info.fileSize, info.format);
}
```

**Verificar:**
- Formato correcto (BMP 24-bit sin comprimir)
- Dimensiones válidas
- No corrupto durante upload

---

### Problema: LEDs no funcionan

**Verificar hardware:**
- Pines correctos (DATA y CLOCK)
- Alimentación suficiente para LEDs
- Conexiones firmes

**Verificar software:**
```cpp
// En setup(), test básico
ledController.init(144);
ledController.fill(CRGB::Red);
ledController.show();
delay(1000);
ledController.fill(CRGB::Green);
ledController.show();
delay(1000);
ledController.fill(CRGB::Blue);
ledController.show();
```

---

## Mejores Prácticas

### 1. Código Limpio

- Nombres descriptivos de variables y funciones
- Comentarios solo cuando el código no es obvio
- Funciones pequeñas (< 50 líneas idealmente)
- Una responsabilidad por función/clase

### 2. Git Workflow

```bash
# Feature branch
git checkout -b feature/new-effect

# Commits pequeños y descriptivos
git commit -m "Add sparkle effect implementation"
git commit -m "Add sparkle effect to web API"
git commit -m "Update UI for sparkle effect"

# Merge a main
git checkout main
git merge feature/new-effect
```

### 3. Versionado

Actualizar en `config.h`:
```cpp
#define FIRMWARE_VERSION "1.1.0"
```

### 4. Documentación

- Actualizar README.md
- Actualizar docs/API_REFERENCE.md
- Comentar código complejo
- Ejemplos de uso en headers

---

## Recursos Adicionales

### Documentación Oficial

- [PlatformIO Docs](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [FastLED Wiki](https://github.com/FastLED/FastLED/wiki)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson](https://arduinojson.org/)

### Herramientas Útiles

- **Serial Plotter**: Para gráficas de datos
- **ESP32 Exception Decoder**: Para decodificar stack traces
- **OTA Updates**: Para actualizaciones remotas

---

**Última actualización:** 2026-01-18
**Versión de esta guía:** 1.0.0
