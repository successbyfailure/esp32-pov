# ESP32 POV-Line

Sistema de persistencia de visión (POV) "inverso" con ESP32 y tira LED. La tira está fija y muestra columnas de imagen secuencialmente para crear efecto visual cuando las personas pasan al lado.

## 🎫 BornHack 2024 Badge Support

**¡NUEVO!** Soporte completo para el badge oficial de BornHack 2024 con:
- Detección automática de movimiento (acelerómetro LIS3DH)
- Control por botón SELECT para cambiar efectos
- 16 LEDs WS2812 optimizados para POV
- Calibración automática del acelerómetro

📖 **Ver guía completa**: [BORNHACK_BADGE.md](BORNHACK_BADGE.md)

## Características

- **Soporte Multi-ESP32**: Compatible con ESP32 clásico, ESP32-C3 y ESP32-S3
- **🎫 BornHack 2024 Badge**: Configuración especializada con acelerómetro y botón
- **Tiras LED Soportadas**: WS2811/WS2812/WS2812B y APA102 (configurable)
- **LEDs Configurables**: 1-300 LEDs (configurable via web), 16 LEDs para badge
- **Detección de Movimiento**: Acelerómetro LIS3DH para POV automático (badge)
- **Control por Botón**: Cambio de efectos con botón físico (badge)
- **Control Web**: Interfaz web completa para configuración y control
- **Home Assistant**: Integración nativa via MQTT con autodiscovery
- **Orientación Configurable**: POV vertical u horizontal
- **Efectos Decorativos**: Rainbow, color sólido, chase, etc.
- **Formatos de Imagen**: BMP 24-bit y RGB565 raw
- **WiFi Manager**: Modo AP para configuración inicial

## Hardware Requerido

- ESP32 (clásico, C3 o S3)
- Tira LED: **WS2811/WS2812/WS2812B** (1 pin) o **APA102** (2 pines)
- Cantidad: 1-300 LEDs (recomendado 61-144 LEDs)
- Fuente de alimentación adecuada para los LEDs

## Pines GPIO

Los pines están predefinidos según el modelo de ESP32:

### WS2811/WS2812/WS2812B (1 pin)
- **ESP32 Clásico**: GPIO23 (DATA)
- **ESP32-C3**: GPIO7 (DATA)
- **ESP32-S3**: GPIO11 (DATA)

### APA102 (2 pines)
- **ESP32 Clásico**: GPIO23 (DATA), GPIO18 (CLOCK)
- **ESP32-C3**: GPIO7 (DATA), GPIO6 (CLOCK)
- **ESP32-S3**: GPIO11 (DATA), GPIO12 (CLOCK)

**Nota**: Para WS281x solo se usa el pin DATA. El pin CLOCK solo se usa con APA102.

## Simulación con Wokwi

**¡Nuevo!** Puedes probar el proyecto completamente en [Wokwi Simulator](https://wokwi.com) sin necesidad de hardware físico.

### Características Soportadas en Wokwi
- ✅ Control completo de LEDs APA102 (visualización en tiempo real)
- ✅ Servidor web con interfaz HTML
- ✅ Todos los efectos (Rainbow, Solid, Chase, POV Test)
- ✅ WiFi simulado con acceso a internet
- ✅ Sistema de archivos LittleFS
- ✅ API REST completa

### Inicio Rápido con Wokwi

1. **Instalar Wokwi for VS Code**:
   ```bash
   code --install-extension wokwi.wokwi-vscode
   ```

2. **Compilar el proyecto**:
   ```bash
   ~/.platformio/penv/bin/pio run -e esp32dev
   ```

3. **Iniciar simulación**: Presionar `F1` → "Wokwi: Start Simulator"

4. **Acceder a interfaz web**: Abrir http://localhost en tu navegador

### Test Simplificado

Para validación rápida, usa el sketch de prueba:

```bash
# Ver test/wokwi_test.cpp para código standalone
# Incluye: control LEDs, efectos, servidor web
```

📖 **Guía rápida**: Ver [WOKWI_QUICKSTART.md](WOKWI_QUICKSTART.md) (inicio en 5 minutos)

📖 **Documentación completa**: Ver [docs/WOKWI_SIMULATION.md](docs/WOKWI_SIMULATION.md)

---

## Compilación y Carga

### Compilar para ESP32 clásico (default):
```bash
~/.platformio/penv/bin/pio run
```

### Compilar para ESP32-C3:
```bash
~/.platformio/penv/bin/pio run -e esp32-c3-devkitm-1
```

### Compilar para ESP32-S3:
```bash
~/.platformio/penv/bin/pio run -e esp32-s3-devkitc-1
```

### 🎫 Compilar para BornHack 2024 Badge:
```bash
~/.platformio/penv/bin/pio run -e bornhack2024
```

📖 **Ver guía completa**: [BORNHACK_BADGE.md](BORNHACK_BADGE.md)

### Cargar firmware:
```bash
~/.platformio/penv/bin/pio run --target upload
```

### Cargar sistema de archivos (interfaz web):
```bash
~/.platformio/penv/bin/pio run --target uploadfs
```

## Configuración Inicial

1. Al encender por primera vez, el ESP32 creará un Access Point llamado "POV-Line-Setup"
2. Contraseña: "povline123"
3. Conectarse al AP y abrir http://192.168.4.1
4. Configurar WiFi en la sección de Configuración
5. El dispositivo se reiniciará y se conectará a tu WiFi

## Uso de la Interfaz Web

### Control POV
1. Subir una imagen (BMP o RGB565)
2. Seleccionar la imagen de la galería
3. Ajustar velocidad y brillo
4. Click en "Play"

### Formatos de Imagen Soportados

**BMP (24-bit sin comprimir)**:
- Altura: Igual al número de LEDs configurados
- Ancho máximo: 128 píxeles
- Crear con GIMP, Photoshop, Paint.NET, etc.

**RGB565 Raw**:
- Header personalizado con magic "R565"
- Formato: [4 bytes magic][2 bytes width][2 bytes height][datos RGB565]

### Limitaciones
- Tamaño máximo de archivo: 100 KB
- Recomendación: Usar imágenes con buen contraste
- Evitar detalles muy finos

## Integración Home Assistant

1. Configurar MQTT en la interfaz web:
   - Broker IP
   - Puerto (default: 1883)
   - Usuario y contraseña (opcional)

2. Habilitar MQTT y guardar configuración

3. El dispositivo aparecerá automáticamente en Home Assistant como "POV Line"

4. Control desde Home Assistant:
   - Encender/Apagar
   - Ajustar brillo
   - Cambiar efectos (POV, Rainbow, Solid Color, Chase)

## Efectos Disponibles

- **POV**: Reproduce imagen con efecto de persistencia de visión
- **Rainbow**: Arco iris animado
- **Solid Color**: Color sólido seleccionable
- **Chase**: Efecto de persecución con color configurable

## Orientación POV

### Vertical (Default)
La tira muestra columnas de la imagen de arriba hacia abajo. Ideal para:
- Tiras LED verticales
- Instalaciones de pared

### Horizontal
La tira muestra filas de la imagen de izquierda a derecha. Ideal para:
- Tiras LED horizontales
- Instalaciones en techo o suelo

## Estructura del Proyecto

```
pov-line/
├── src/
│   ├── main.cpp                 # Programa principal
│   ├── config.h                 # Configuración y constantes
│   ├── led_controller.{h,cpp}   # Control de tira APA102
│   ├── pov_engine.{h,cpp}       # Motor de animación POV
│   ├── image_parser.{h,cpp}     # Parser de imágenes BMP/RGB565
│   ├── image_manager.{h,cpp}    # Gestión de archivos
│   ├── web_server.{h,cpp}       # Servidor web y API
│   ├── wifi_manager.{h,cpp}     # Gestión WiFi
│   ├── effects.{h,cpp}          # Efectos decorativos
│   └── ha_integration.{h,cpp}   # Integración Home Assistant
├── data/
│   ├── index.html               # Interfaz web
│   ├── style.css                # Estilos
│   └── app.js                   # JavaScript cliente
└── platformio.ini               # Configuración PlatformIO
```

## API REST

### Estado
```
GET /api/status
```

### Listar Imágenes
```
GET /api/images
```

### Subir Imagen
```
POST /api/upload
Content-Type: multipart/form-data
```

### Reproducir POV
```
POST /api/play
Body: image=nombre_imagen.bmp
```

### Pausar/Detener
```
POST /api/pause
POST /api/stop
```

### Configurar Ajustes
```
POST /api/settings
Body: speed=30&brightness=128&loop=true&orientation=vertical
```

### Activar Efecto
```
POST /api/effect
Body: effect=rainbow&speed=10
```

## Configuración Avanzada

Editar [config.h](src/config.h) para cambiar:
- Número máximo de LEDs
- Tamaño máximo de imagen
- Velocidades POV min/max
- Timeouts WiFi
- Configuración MQTT

## Compilación Exitosa

El proyecto ha sido compilado y está listo para cargar al ESP32:

- **RAM Usada**: 14.5% (47,528 bytes de 327,680 bytes)
- **Flash Usada**: 71.0% (931,017 bytes de 1,310,720 bytes)

## Troubleshooting

### No compila
- Asegurar que PlatformIO está actualizado
- Limpiar proyecto: `~/.platformio/penv/bin/pio run --target clean`

### No se conecta a WiFi
- Verificar credenciales en la configuración
- Conectar al AP "POV-Line-Setup" y reconfigurar

### Imágenes no se ven correctamente
- Verificar que la altura de la imagen coincide con el número de LEDs
- Probar con diferentes velocidades
- Asegurar que el formato BMP es 24-bit sin comprimir

### POV no funciona
- Verificar que hay suficiente espacio en LittleFS
- Revisar dimensiones de la imagen
- Probar con orientación diferente

## Créditos

Proyecto desarrollado con:
- **PlatformIO**: Framework de desarrollo
- **FastLED**: Control de LEDs
- **ESPAsyncWebServer**: Servidor web asíncrono
- **ArduinoJson**: Parsing JSON
- **PubSubClient**: Cliente MQTT

## Licencia

Proyecto de código abierto para uso educativo y personal.

---

**Version**: 1.0.0
**Date**: 2026-01-18
