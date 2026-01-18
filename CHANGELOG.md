# Changelog

Todos los cambios notables en este proyecto serán documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
y este proyecto adhiere a [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Añadido

#### 🎫 Soporte BornHack 2024 Badge
- Entorno de compilación `bornhack2024` en `platformio.ini`
- Soporte para ESP32-C3 con 16 LEDs WS2812
- Módulo `accelerometer.{h,cpp}` para acelerómetro LIS3DH
- Detección automática de movimiento para activar POV
- Calibración automática del acelerómetro al inicio
- Módulo `button.{h,cpp}` para manejo de botón SELECT
- Sistema de debouncing para botón
- Detección de pulsación larga (long press)
- 4 efectos cambiables con botón: OFF, Rainbow, Chase, POV
- Modo POV activado automáticamente con movimiento detectado
- Indicador visual de número de efecto (LEDs azules)
- Ahorro de batería con efecto 0 (LEDs apagados)
- Compilación condicional con flags `BORNHACK_BADGE`, `HAS_ACCELEROMETER`, `BUTTON_PIN`
- Documentación completa en `BORNHACK_BADGE.md`
- Instrucciones para identificar pines desde schematic
- Guías de troubleshooting específicas para badge
- Dependencias: Adafruit LIS3DH y Adafruit BusIO

#### Soporte Multi-LED (WS281x y APA102)
- Soporte configurable para tiras LED WS2811/WS2812/WS2812B (1 pin)
- Soporte configurable para tiras LED APA102/DotStar (2 pines)
- Configuración de tipo de LED via interfaz web
- Configuración de número de LEDs (1-300) via web
- Enum `LEDStripType` en `config.h` con tipos soportados
- Función `ledController.setLEDType()` para cambiar tipo en runtime
- Función `ledController.getLEDType()` para obtener tipo actual
- Endpoints `/api/settings` actualizados con parámetros `ledType` y `numLeds`
- `/api/status` incluye información de `ledType` y `numLeds`
- Guardado automático de configuración LED en `config.json`
- Detección automática de hardware en inicialización
- Archivo `diagram-ws2811.json` para simulación Wokwi con WS2811
- Archivo `diagram-apa102.json` para simulación Wokwi con APA102
- Documentación completa en `LED_CONFIGURATION.md`

#### Simulación Wokwi
- Soporte completo para Wokwi Simulator
- Archivo `diagram.json` con configuración de hardware virtual
- Archivo `wokwi.toml` con configuración de proyecto y port forwarding
- Archivo `partitions.csv` con tabla de particiones personalizada para LittleFS
- Test simplificado en `test/wokwi_test.cpp` para validación rápida
- Documentación completa de simulación en `docs/WOKWI_SIMULATION.md`
- Guías de uso, troubleshooting y mejores prácticas para Wokwi
- README en `test/` con documentación del sketch de prueba

#### Beneficios de la Simulación
- Desarrollo sin hardware físico
- Visualización en tiempo real de efectos LED
- Testing de servidor web y API REST
- Validación de lógica antes de deployment a hardware
- Acceso a interfaz web via http://localhost
- Soporte completo de WiFi, LittleFS y AsyncWebServer

---

## [1.0.0] - 2026-01-18

### Añadido

#### Core Features
- Sistema completo de control de tira LED APA102 via FastLED
- Motor POV (Persistence of Vision) con streaming de imágenes
- Soporte para orientación vertical y horizontal
- Efectos decorativos: Rainbow, Solid Color, Color Chase, Fade
- Gestión de imágenes en LittleFS
- Parser de imágenes BMP (24-bit) y RGB565

#### Conectividad
- WiFi Manager con modos AP y Station
- Servidor web asíncrono con interfaz HTML/CSS/JS
- API REST completa para control del sistema
- Integración con Home Assistant via MQTT con autodiscovery
- Portal web para configuración y control

#### Multiplataforma
- Soporte para ESP32 clásico
- Soporte para ESP32-C3
- Soporte para ESP32-S3
- Configuración de pines automática según plataforma

#### Interfaz Web
- Diseño responsive con dark theme
- Control de reproducción POV (play/pause/stop)
- Ajuste de velocidad y brillo en tiempo real
- Upload de imágenes con drag & drop
- Galería de imágenes con información
- Activación de efectos decorativos
- Configuración de WiFi y MQTT
- Instrucciones de uso y limitaciones

#### Configuración
- Sistema de configuración persistente en JSON
- Gestión de credenciales WiFi
- Configuración MQTT para Home Assistant
- Ajustes de LEDs (número, brillo)
- Parámetros POV (velocidad, loop, orientación)

#### Documentación
- README completo con instrucciones
- Documentación de arquitectura del sistema
- Referencia completa de API (REST y MQTT)
- Guía de desarrollo para programadores
- Ejemplos de código extensivos
- Comentarios en código fuente

### Características Técnicas

#### Optimizaciones
- Streaming de columnas de imagen para minimizar uso de RAM
- Actualización non-blocking de todos los componentes
- Servidor web asíncrono (no bloquea loop principal)
- Acceso directo a buffer de LEDs para rendimiento
- Gestión eficiente de memoria dinámica

#### Seguridad y Validación
- Validación de tamaño y formato de imágenes
- Límites de parámetros en API
- Verificación de espacio disponible antes de uploads
- Sanitización de nombres de archivo
- Manejo robusto de errores

#### Compilación
- Build exitoso en PlatformIO
- Uso de RAM: 14.5% (47,528 bytes)
- Uso de Flash: 71.0% (931,017 bytes)
- Sin warnings críticos de compilación

### Detalles de Implementación

**Módulos Creados:**
- `config.h` - Configuración y constantes
- `led_controller.{h,cpp}` - Control de LEDs
- `pov_engine.{h,cpp}` - Motor de animación POV
- `effects.{h,cpp}` - Efectos decorativos
- `image_parser.{h,cpp}` - Parser de formatos de imagen
- `image_manager.{h,cpp}` - Gestión de archivos
- `wifi_manager.{h,cpp}` - Gestión WiFi
- `web_server.{h,cpp}` - Servidor HTTP y API
- `ha_integration.{h,cpp}` - Integración Home Assistant
- `main.cpp` - Programa principal

**Archivos Web:**
- `data/index.html` - Interfaz de usuario
- `data/style.css` - Estilos CSS
- `data/app.js` - Lógica JavaScript

**Documentación:**
- `docs/ARCHITECTURE.md` - Arquitectura del sistema
- `docs/API_REFERENCE.md` - Referencia de API
- `docs/DEVELOPMENT_GUIDE.md` - Guía de desarrollo
- `docs/EXAMPLES.md` - Ejemplos de código
- `docs/README.md` - Índice de documentación

**Dependencias:**
- FastLED ^3.7.0
- ESPAsyncWebServer (from GitHub)
- AsyncTCP (from GitHub)
- ArduinoJson ^7.2.0
- PubSubClient ^2.8.0

### Configuración por Defecto

```
Device Name: POV-Line
LEDs: 144
Brightness: 128
POV Speed: 30 FPS
Loop Mode: Enabled
Orientation: Vertical
WiFi AP: POV-Line-Setup / povline123
MQTT Port: 1883
Max Image Size: 100 KB
Max Image Width: 128 px
```

### Limitaciones Conocidas

- Altura de imagen debe coincidir con número de LEDs configurados
- Ancho máximo de imagen: 128 píxeles
- Tamaño máximo de archivo: 100 KB
- Solo BMP 24-bit sin compresión y RGB565 raw soportados
- WiFi solo en banda 2.4 GHz (limitación de ESP32)
- MQTT sin TLS/SSL en esta versión
- Passwords WiFi/MQTT almacenados sin encriptar en LittleFS

### Notas de Desarrollo

**Decisiones de Diseño:**
- Instancias globales para módulos de hardware (simplifica acceso)
- Non-blocking pattern para todos los componentes (evita watchdog reset)
- Streaming de imágenes (optimiza uso de RAM limitada)
- Servidor asíncrono (permite múltiples clientes simultáneos)
- JSON para configuración (facilita debugging y edición manual)

**Testing:**
- Compilación exitosa en ESP32, ESP32-C3 y ESP32-S3
- Validación de API endpoints con cURL
- Verificación de formatos de imagen BMP y RGB565
- Prueba de efectos LED
- Test de conectividad WiFi (AP y STA)

### Roadmap Futuro

#### v1.1.0 (Planned)
- [ ] Soporte para GIF animados
- [ ] Soporte para PNG
- [ ] OTA (Over-The-Air) updates
- [ ] Sensor de proximidad para activación automática
- [ ] Más efectos LED (sparkle, fire, matrix)
- [ ] Scheduling de efectos por tiempo
- [ ] Control por Alexa/Google Home

#### v2.0.0 (Planned)
- [ ] Soporte para múltiples tiras LED
- [ ] Sincronización entre múltiples dispositivos
- [ ] Visualizador de audio
- [ ] Control por gestos (sensor)
- [ ] App móvil nativa
- [ ] Encriptación de configuración
- [ ] MQTT con TLS

---

## Formato de Versiones

**MAJOR.MINOR.PATCH**

- **MAJOR**: Cambios incompatibles en API
- **MINOR**: Nueva funcionalidad compatible con versión anterior
- **PATCH**: Bug fixes y mejoras menores compatibles

---

## Tipos de Cambios

- **Añadido**: Para nuevas funcionalidades
- **Cambiado**: Para cambios en funcionalidad existente
- **Deprecado**: Para funcionalidad que será removida
- **Removido**: Para funcionalidad removida
- **Corregido**: Para bug fixes
- **Seguridad**: Para vulnerabilidades de seguridad

---

**[Unreleased]**: Cambios en desarrollo no lanzados aún
**[1.0.0]**: Primera versión estable - 2026-01-18
