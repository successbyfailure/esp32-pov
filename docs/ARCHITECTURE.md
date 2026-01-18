# Arquitectura del Sistema POV-Line

## Visión General

POV-Line es un sistema embebido para ESP32 que controla tiras LED APA102 para crear efectos de persistencia de visión (POV). El sistema está diseñado con una arquitectura modular que separa responsabilidades en componentes independientes.

## Diagrama de Componentes

```
┌─────────────────────────────────────────────────────────┐
│                      ESP32 Main Loop                     │
└─────────────────────────────────────────────────────────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
        ▼                  ▼                  ▼
┌──────────────┐   ┌──────────────┐   ┌──────────────┐
│ POV Engine   │   │  Effects     │   │ HA Integration│
│              │   │              │   │  (MQTT)       │
└──────────────┘   └──────────────┘   └──────────────┘
        │                  │
        ▼                  ▼
┌──────────────────────────────────┐
│      LED Controller (FastLED)     │
└──────────────────────────────────┘
        │
        ▼
┌──────────────────────────────────┐
│      APA102 LED Strip (SPI)       │
└──────────────────────────────────┘

┌─────────────────────────────────────────────────────────┐
│                    Web Server (Async)                    │
└─────────────────────────────────────────────────────────┘
        │
        ├──► Image Manager ──► LittleFS
        │
        └──► Image Parser ──► BMP/RGB565 Files

┌─────────────────────────────────────────────────────────┐
│              WiFi Manager (AP/STA Modes)                 │
└─────────────────────────────────────────────────────────┘
```

## Módulos del Sistema

### 1. LED Controller
**Archivo**: `src/led_controller.{h,cpp}`

**Responsabilidad**: Abstracción de bajo nivel para el control de la tira LED APA102.

**Dependencias**:
- FastLED library
- config.h

**Funciones Principales**:
```cpp
bool init(uint16_t numLeds)          // Inicializa tira LED
void setPixel(uint16_t index, CRGB)  // Establece color de un LED
void setBrightness(uint8_t value)    // Ajusta brillo global
void show()                          // Actualiza LEDs (envía datos por SPI)
CRGB* getPixels()                    // Acceso directo al buffer
```

**Implementación**:
- Utiliza FastLED con driver APA102
- Buffer dinámico de LEDs en heap
- SPI hardware para comunicación rápida
- Soporte para hasta MAX_LEDS (300)

**Optimizaciones**:
- Buffer único compartido para evitar duplicación
- Acceso directo al buffer para operaciones rápidas
- Sin double buffering (no necesario para APA102)

---

### 2. POV Engine
**Archivo**: `src/pov_engine.{h,cpp}`

**Responsabilidad**: Motor de animación POV que lee imágenes y las muestra columna por columna o fila por fila.

**Dependencias**:
- image_parser
- led_controller
- LittleFS

**Estado Interno**:
```cpp
char currentImageFile[64]      // Path de imagen actual
ImageInfo currentImage          // Metadata de imagen
uint16_t currentColumn          // Columna/fila actual
uint16_t speed                  // FPS de reproducción
bool loopMode                   // Reproducción en bucle
POVOrientation orientation      // Vertical u horizontal
bool playing, paused            // Estado de reproducción
CRGB* columnBuffer             // Buffer para columna actual
```

**Funciones Principales**:
```cpp
bool loadImage(const char* filename)     // Cargar imagen desde LittleFS
void play()                              // Iniciar reproducción POV
void pause() / resume() / stop()         // Control de reproducción
void setSpeed(uint16_t fps)              // Ajustar velocidad (1-120 FPS)
void setOrientation(POVOrientation)      // Vertical u horizontal
void update()                            // Actualizar frame (llamar en loop)
```

**Algoritmo de Reproducción**:
1. Verificar timing con millis() (non-blocking)
2. Leer columna/fila actual del archivo
3. Escribir píxeles al LED Controller
4. Incrementar índice de columna/fila
5. Loop si está habilitado, o stop al final

**Streaming de Datos**:
- No carga imagen completa en RAM
- Lee columna por columna desde LittleFS
- Reduce uso de memoria significativamente
- Permite imágenes más grandes

**Orientaciones**:
- **Vertical**: Lee columnas (X) de la imagen, muestra en altura de LEDs (Y)
- **Horizontal**: Lee filas (Y) de la imagen, muestra en ancho de LEDs (X)

---

### 3. Image Parser
**Archivo**: `src/image_parser.{h,cpp}`

**Responsabilidad**: Parseo y lectura de archivos de imagen BMP y RGB565.

**Formatos Soportados**:

#### BMP (24-bit sin comprimir):
```cpp
struct BMPHeader {
    uint16_t signature;      // "BM"
    uint32_t fileSize;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;     // Offset a datos de píxeles
};

struct BMPInfoHeader {
    uint32_t headerSize;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bitsPerPixel;   // Debe ser 24
    uint32_t compression;    // Debe ser 0 (sin comprimir)
    // ...
};
```

#### RGB565 Raw:
```cpp
struct RGB565Header {
    char magic[4];           // "R565"
    uint16_t width;
    uint16_t height;
};
// Seguido de datos raw: width * height * 2 bytes
```

**Funciones Principales**:
```cpp
bool parseImageInfo(const char* filename, ImageInfo& info)
bool getColumn(const char* filename, uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize)
```

**Lectura de Columnas**:
- BMP: Se almacena bottom-up, requiere inversión de Y
- BMP: Padding de filas a 4 bytes
- RGB565: Conversión a RGB888 para FastLED
- Acceso directo con seek() para columnas individuales

**Conversión RGB565 a RGB888**:
```cpp
R8 = (RGB565 >> 11) & 0x1F * 255 / 31
G8 = (RGB565 >> 5)  & 0x3F * 255 / 63
B8 = RGB565         & 0x1F * 255 / 31
```

---

### 4. Image Manager
**Archivo**: `src/image_manager.{h,cpp}`

**Responsabilidad**: Gestión de archivos de imagen en LittleFS.

**Funciones Principales**:
```cpp
bool init()                                    // Inicializar LittleFS
std::vector<ImageInfo> listImages()            // Listar todas las imágenes
bool deleteImage(const char* filename)         // Eliminar imagen
bool getImageInfo(const char* filename, ImageInfo& info)
size_t getFreeSpace()                          // Espacio disponible
void refreshList()                             // Recargar lista de imágenes
```

**Estructura de Directorios**:
```
/
├── config.json           # Configuración del sistema
└── images/              # Directorio de imágenes
    ├── test.bmp
    ├── logo.bmp
    └── animation.rgb
```

**Límites**:
- Tamaño máximo por imagen: 100 KB (MAX_IMAGE_SIZE)
- Número de imágenes: Limitado por espacio LittleFS
- Extensiones válidas: .bmp, .rgb, .565

---

### 5. Effects
**Archivo**: `src/effects.{h,cpp}`

**Responsabilidad**: Efectos decorativos para cuando no está en modo POV.

**Efectos Implementados**:

#### Rainbow:
```cpp
void rainbow(uint8_t speed)
```
- Genera arco iris usando HSV
- Animación con incremento de hue
- Speed controla velocidad de rotación

#### Solid Color:
```cpp
void solidColor(CRGB color)
```
- Color sólido en toda la tira
- Sin animación

#### Color Chase:
```cpp
void colorChase(CRGB color, uint8_t speed)
```
- 3 LEDs del color especificado persiguiéndose
- Speed controla velocidad de movimiento

**Patrón de Actualización**:
```cpp
void update() {
    if (!running) return;

    unsigned long currentTime = millis();
    if (currentTime - lastUpdate < delay) return;

    // Actualizar efecto
    switch (currentEffect) {
        case EFFECT_RAINBOW: updateRainbow(); break;
        // ...
    }

    lastUpdate = currentTime;
}
```

---

### 6. WiFi Manager
**Archivo**: `src/wifi_manager.{h,cpp}`

**Responsabilidad**: Gestión de conectividad WiFi con modos AP y STA.

**Modos de Operación**:

#### Access Point (AP):
- SSID: "POV-Line-Setup"
- Password: "povline123"
- IP: 192.168.4.1
- Para configuración inicial

#### Station (STA):
- Conecta a red WiFi guardada
- Reintentos automáticos cada 30s
- Timeout de 20s por intento

**Flujo de Conexión**:
```
┌─────────────┐
│   Inicio    │
└─────┬───────┘
      │
      ▼
┌────────────────────┐
│ ¿Credenciales      │
│ guardadas?         │
└────┬──────────┬────┘
     │ No       │ Yes
     │          ▼
     │    ┌─────────────┐
     │    │ Intentar    │
     │    │ conectar    │
     │    └─────┬───┬───┘
     │          │   │
     │   ┌──────┘   └──────┐
     │   │ Éxito       Fallo│
     │   ▼                  │
     │ ┌─────────┐          │
     │ │ Modo STA│          │
     │ └─────────┘          │
     │                      │
     └──────────────────────┘
              │
              ▼
        ┌──────────┐
        │ Modo AP  │
        └──────────┘
```

---

### 7. Web Server
**Archivo**: `src/web_server.{h,cpp}`

**Responsabilidad**: Servidor HTTP asíncrono y API REST.

**Dependencias**:
- ESPAsyncWebServer
- AsyncTCP
- ArduinoJson

**Endpoints API**:

| Método | Ruta | Descripción |
|--------|------|-------------|
| GET | /api/status | Estado del sistema |
| GET | /api/images | Lista de imágenes |
| POST | /api/play | Iniciar POV |
| POST | /api/pause | Pausar POV |
| POST | /api/stop | Detener POV |
| POST | /api/settings | Actualizar configuración |
| GET | /api/effects | Lista de efectos |
| POST | /api/effect | Activar efecto |
| POST | /api/upload | Subir imagen |
| POST | /api/image/delete | Eliminar imagen |
| GET | /api/config | Obtener configuración |
| POST | /api/config | Guardar configuración |

**Ejemplo de Respuesta JSON** (/api/status):
```json
{
  "state": "playing",
  "image": "/images/test.bmp",
  "column": 45,
  "totalColumns": 128,
  "speed": 30,
  "brightness": 128,
  "loopMode": true,
  "orientation": "vertical",
  "effectRunning": false,
  "effectType": 0,
  "wifiConnected": true,
  "wifiSSID": "MiWiFi",
  "wifiIP": "192.168.1.100",
  "freeSpace": 245760
}
```

**Upload de Imágenes**:
- Multipart form-data
- Streaming a LittleFS
- Validación de tamaño
- Progress tracking

---

### 8. Home Assistant Integration
**Archivo**: `src/ha_integration.{h,cpp}`

**Responsabilidad**: Integración con Home Assistant via MQTT.

**MQTT Topics**:

```
pov_line/state                        # ON/OFF
pov_line/command                      # ON/OFF commands
pov_line/brightness/state             # 0-255
pov_line/brightness/set               # Set brightness
pov_line/effect/state                 # Current effect name
pov_line/effect/set                   # Set effect

homeassistant/light/pov_line_XXXXX/config  # Discovery
```

**Discovery Payload**:
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

**Reconexión Automática**:
- Intenta reconectar cada 5 segundos
- Solo si WiFi está conectado
- Publica discovery y estado al conectar

---

### 9. Main Application
**Archivo**: `src/main.cpp`

**Responsabilidad**: Inicialización del sistema y loop principal.

**Secuencia de Inicialización**:
```
1. Serial.begin(115200)
2. LittleFS.begin(true)
3. loadConfig() from /config.json
4. ledController.init(config.numLeds)
5. imageManager.init()
6. wifiManager.init()
   ├─► connectWiFi() si hay credenciales
   └─► startAP() si falla conexión
7. webServer.init()
8. haIntegration.init() si MQTT habilitado
9. povEngine.setSpeed/setLoopMode/setOrientation
10. effects.rainbow(15) # Efecto inicial
```

**Loop Principal**:
```cpp
void loop() {
    povEngine.update();      // Actualizar POV
    effects.update();        // Actualizar efectos
    haIntegration.loop();    // Procesar MQTT
    wifiManager.loop();      // Mantener WiFi
    delay(1);                // Yield para watchdog
}
```

**Gestión de Configuración**:
- Archivo: `/config.json` en LittleFS
- Formato: JSON con ArduinoJson v7
- Auto-guardado al cambiar valores críticos
- Valores por defecto si no existe archivo

---

## Flujo de Datos

### Reproducción POV

```
Usuario selecciona imagen en Web UI
        ▼
Web Server recibe POST /api/play
        ▼
povEngine.loadImage(filename)
        ├─► imageParser.parseImageInfo()
        │   └─► Valida formato y dimensiones
        └─► Reserva memoria para columnBuffer
        ▼
povEngine.play()
        └─► playing = true, currentColumn = 0
        ▼
loop() llama povEngine.update()
        ▼
Verificar timing con millis()
        ▼
imageParser.getColumn(currentColumn, buffer)
        ├─► Abre archivo
        ├─► Seek a posición de columna
        └─► Lee píxeles y convierte a CRGB
        ▼
ledController.setPixel() para cada LED
        ▼
ledController.show()
        └─► FastLED envía datos por SPI a APA102
        ▼
Incrementar currentColumn
        ▼
¿Fin de imagen?
    ├─► Si, y loopMode: currentColumn = 0
    └─► Si, y !loopMode: stop()
```

### Upload de Imagen

```
Usuario arrastra BMP a Web UI
        ▼
app.js valida tamaño y extensión
        ▼
Envía POST /api/upload con FormData
        ▼
WebServer.handleUpload()
        ├─► Primera llamada (index=0):
        │   ├─► Verifica espacio disponible
        │   └─► Abre /images/filename en write
        ├─► Llamadas intermedias:
        │   └─► Escribe chunks de datos
        └─► Última llamada (final=true):
            ├─► Cierra archivo
            └─► imageManager.refreshList()
        ▼
Respuesta JSON {success: true}
        ▼
app.js actualiza galería
```

---

## Gestión de Memoria

### Heap Allocation

**Dinámico** (heap):
- LED buffer: `numLeds * sizeof(CRGB)` = numLeds * 3 bytes
- POV column buffer: `MAX_LEDS * sizeof(CRGB)` = 900 bytes
- Image list: `std::vector<ImageInfo>` variable
- JSON documents: ArduinoJson dynamic allocation

**Estático** (stack/data):
- Config struct: ~200 bytes
- Buffers temporales en funciones
- String constants

### Ejemplo de Uso (144 LEDs):
```
LED buffer:           144 * 3 = 432 bytes
POV column buffer:    300 * 3 = 900 bytes
Image list (10 imgs): 10 * 80 = 800 bytes
Config:                        200 bytes
JSON docs (temp):              ~2KB
Otros:                         ~5KB
--------------------------------
Total aprox:                   ~9.3 KB

RAM disponible ESP32:          320 KB
Uso actual:                    ~47 KB (14.5%)
Margen:                        ~273 KB
```

### Optimizaciones de Memoria

1. **Streaming de imágenes**: No cargar imagen completa
2. **Compartir buffers**: LED buffer accesible directamente
3. **Lazy loading**: Cargar image list solo cuando se necesita
4. **JSON documents**: Tamaño mínimo necesario

---

## Consideraciones de Rendimiento

### Timing Crítico

**APA102 SPI**:
- Frecuencia: 10 MHz (configurable hasta 20 MHz)
- Latencia muy baja, no requiere timing preciso
- show() toma ~0.5ms para 144 LEDs

**POV Update Rate**:
- Configurable: 1-120 FPS
- A 30 FPS: ~33ms por frame
- Suficiente para effects.update() y otros procesos

**Web Server**:
- Asíncrono, no bloquea loop
- Handlers ejecutan rápido
- File uploads en streaming

### CPU Usage

**Loop Timing** (ESP32 @ 240 MHz):
- povEngine.update(): <1ms
- effects.update(): <1ms
- MQTT loop: <0.5ms
- WiFi loop: <0.1ms
- Total: <3ms por iteración
- Yield cada iteración para watchdog

---

## Seguridad y Validación

### Input Validation

**Imágenes**:
- Tamaño máximo: 100 KB
- Extensiones permitidas: .bmp, .rgb, .565
- Validación de headers
- Verificación de dimensiones

**API Requests**:
- Validación de parámetros
- Límites de valores (brightness 0-255, speed 1-120)
- Sanitización de nombres de archivo

**WiFi Credentials**:
- Longitud máxima: 32 chars SSID, 64 chars password
- Almacenamiento en LittleFS (no encriptado)

### Error Handling

**Patrones**:
- Return false en caso de error
- Mensajes de error a Serial
- Estados de error en API responses
- Fallback a valores por defecto

---

## Extensibilidad

### Añadir Nuevo Efecto

1. Agregar enum en `config.h`:
```cpp
enum EffectType {
    // ...
    EFFECT_NEW_EFFECT
};
```

2. Implementar en `effects.cpp`:
```cpp
void Effects::newEffect(uint8_t param) {
    currentEffect = EFFECT_NEW_EFFECT;
    // ... setup
    running = true;
}

void Effects::updateNewEffect() {
    // ... animación
}
```

3. Añadir case en `update()`:
```cpp
case EFFECT_NEW_EFFECT:
    updateNewEffect();
    break;
```

4. Agregar endpoint en `web_server.cpp`
5. Actualizar UI en `app.js`

### Añadir Nuevo Formato de Imagen

1. Definir struct en `image_parser.h`
2. Implementar `parseFormatX()` en `image_parser.cpp`
3. Implementar `getColumnFormatX()`
4. Añadir detección en `parseImageInfo()`
5. Actualizar `isImageFile()` en `image_manager.cpp`
6. Documentar en README y Web UI

---

## Testing y Debugging

### Serial Output

Habilitar con: `Serial.begin(115200)`

**Mensajes Importantes**:
- Inicialización de componentes
- Errores de carga de archivos
- Estado de conexión WiFi/MQTT
- Info de imágenes cargadas

### Web API Testing

```bash
# Status
curl http://192.168.1.100/api/status

# Upload image
curl -F "file=@test.bmp" http://192.168.1.100/api/upload

# Play POV
curl -X POST -d "image=test.bmp" http://192.168.1.100/api/play

# Settings
curl -X POST -d "speed=60&brightness=200" http://192.168.1.100/api/settings
```

### Memory Monitoring

```cpp
// En main.cpp o donde sea necesario
Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
Serial.printf("LittleFS - Total: %d, Used: %d, Free: %d\n",
              LittleFS.totalBytes(),
              LittleFS.usedBytes(),
              LittleFS.totalBytes() - LittleFS.usedBytes());
```

---

## Build System

### PlatformIO Environments

```ini
[env:esp32dev]       # ESP32 clásico
[env:esp32-c3-devkitm-1]  # ESP32-C3
[env:esp32-s3-devkitc-1]  # ESP32-S3
```

### Compile Flags

```
-DESP32_CLASSIC / ESP32_C3 / ESP32_S3
-DLED_DATA_PIN=xx
-DLED_CLOCK_PIN=xx
```

### Dependencies

Todas las dependencias se descargan automáticamente:
- FastLED (control LEDs)
- ESPAsyncWebServer (servidor web)
- AsyncTCP (networking asíncrono)
- ArduinoJson (parsing JSON)
- PubSubClient (cliente MQTT)

---

## Conclusión

El sistema está diseñado con separación de responsabilidades clara, permitiendo:
- **Mantenibilidad**: Cada módulo tiene una función específica
- **Testabilidad**: Componentes pueden probarse independientemente
- **Extensibilidad**: Fácil añadir nuevos efectos, formatos, features
- **Eficiencia**: Optimizado para memoria y CPU limitados
- **Robustez**: Manejo de errores y validación en todos los niveles
