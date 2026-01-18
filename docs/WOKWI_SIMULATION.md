# Simulación con Wokwi

## Tabla de Contenidos

- [Introducción](#introducción)
- [Requisitos](#requisitos)
- [Configuración del Entorno](#configuración-del-entorno)
- [Archivos de Configuración Wokwi](#archivos-de-configuración-wokwi)
- [Uso del Test Simplificado](#uso-del-test-simplificado)
- [Uso del Proyecto Completo](#uso-del-proyecto-completo)
- [Características Soportadas](#características-soportadas)
- [Limitaciones](#limitaciones)
- [Troubleshooting](#troubleshooting)
- [Recursos](#recursos)

---

## Introducción

El proyecto POV-Line puede ser simulado completamente en **Wokwi Simulator**, permitiendo desarrollo y pruebas sin necesidad de hardware físico.

**Wokwi** soporta todos los componentes críticos del proyecto:
- ✅ ESP32 (classic, C3, S3)
- ✅ Tira LED APA102 (con FastLED)
- ✅ Sistema de archivos LittleFS
- ✅ ESPAsyncWebServer
- ✅ WiFi con acceso a internet

**Beneficios de la simulación**:
- Desarrollo sin hardware
- Debugging visual de LEDs
- Pruebas de servidor web
- Validación de lógica antes de deployment
- Testing de diferentes configuraciones

---

## Requisitos

### Software Necesario

1. **VS Code** (recomendado) o navegador web
2. **Wokwi for VS Code** extension (para desarrollo local)
   - O cuenta en [wokwi.com](https://wokwi.com) (para navegador)
3. **PlatformIO** extension para VS Code
4. **Git** (para clonar el proyecto)

### Instalación de Wokwi for VS Code

```bash
# Abrir VS Code
# Ir a Extensions (Ctrl+Shift+X)
# Buscar "Wokwi Simulator"
# Instalar extensión oficial de Wokwi
```

O desde línea de comandos:
```bash
code --install-extension wokwi.wokwi-vscode
```

---

## Configuración del Entorno

### 1. Preparar el Proyecto

```bash
# Clonar o navegar al proyecto
cd /path/to/pov-line

# Compilar el firmware
pio run -e esp32dev
```

### 2. Verificar Archivos Wokwi

El proyecto incluye los siguientes archivos para Wokwi:

```
pov-line/
├── diagram.json       # Diagrama de hardware (ESP32 + LEDs)
├── wokwi.toml         # Configuración de proyecto
└── test/
    └── wokwi_test.cpp # Test simplificado para validación
```

### 3. Abrir en Wokwi

**Opción A: VS Code (recomendado)**
1. Abrir proyecto en VS Code
2. Presionar `F1` → "Wokwi: Start Simulator"
3. O usar el ícono de Wokwi en la barra lateral

**Opción B: Navegador Web**
1. Ir a [wokwi.com/projects/new/esp32](https://wokwi.com/projects/new/esp32)
2. Subir archivos: `diagram.json`, `wokwi.toml`, código fuente
3. Iniciar simulación

### 4. Tabla de Particiones

El proyecto incluye un archivo `partitions.csv` que define las particiones de memoria:

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x140000,
app1,     app,  ota_1,   0x150000,0x140000,
spiffs,   data, spiffs,  0x290000,0x170000,
```

**Importante**: La partición `spiffs` (1.4 MB) es usada para LittleFS. El nombre "spiffs" se mantiene por compatibilidad con herramientas, pero el sistema de archivos es LittleFS.

**Configuración automática**:
- `platformio.ini` ya incluye `board_build.partitions = partitions.csv`
- `wokwi.toml` ya mapea la partición correctamente
- No se requiere acción adicional

---

## Archivos de Configuración Wokwi

### diagram.json

Define el hardware virtual:

```json
{
  "version": 1,
  "author": "darkjavi",
  "editor": "wokwi",
  "parts": [
    {
      "type": "wokwi-esp32-devkit-v1",
      "id": "esp",
      "top": 0,
      "left": 0,
      "attrs": {}
    },
    {
      "type": "wokwi-apa102",
      "id": "led1",
      "top": -48,
      "left": 124.8,
      "attrs": { "count": "144" }
    }
  ],
  "connections": [
    [ "led1:DI", "esp:D23", "green", [ "v0" ] ],
    [ "led1:CI", "esp:D18", "blue", [ "v0" ] ],
    [ "led1:VCC", "esp:VIN", "red", [ "v0" ] ],
    [ "led1:GND", "esp:GND.1", "black", [ "v0" ] ]
  ]
}
```

**Componentes**:
- `wokwi-esp32-devkit-v1`: Placa ESP32 classic
- `wokwi-apa102`: Tira LED con 144 LEDs

**Conexiones**:
- `D23` (GPIO23) → DATA (MOSI para APA102)
- `D18` (GPIO18) → CLOCK (SCK para APA102)
- `VIN` → VCC (5V)
- `GND` → GND

### wokwi.toml

Configuración de proyecto y red:

```toml
[wokwi]
version = 1
elf = ".pio/build/esp32dev/firmware.elf"
firmware = ".pio/build/esp32dev/firmware.bin"

# Port forwarding para acceder al servidor web
[[net.forward]]
from = "localhost:80"
to = "target:80"

[[net.forward]]
from = "localhost:1883"
to = "target:1883"

[serial]
baudrate = 115200

# LittleFS filesystem
[[fs]]
type = "littlefs"
partition = "spiffs"
mountpoint = "/"
```

**Configuración de Red**:
- Puerto 80 (HTTP) → Accesible en `http://localhost`
- Puerto 1883 (MQTT) → Broker local si se usa

**Filesystem**:
- LittleFS simulado
- Montado en raíz (`/`)

---

## Uso del Test Simplificado

### Propósito

El archivo `test/wokwi_test.cpp` es un sketch simplificado para validar componentes básicos:

✅ Control de LEDs APA102
✅ Efectos: Rainbow, Solid, Chase, POV Test
✅ Servidor web con interfaz HTML
✅ Control de brillo y colores
✅ Sistema de archivos LittleFS

### Compilar y Ejecutar

**Opción 1: Usando VS Code + Wokwi Extension**

1. Modificar `platformio.ini` temporalmente para usar el test:

```ini
[env:esp32dev]
board = esp32dev
build_flags =
    -DESP32_CLASSIC
    -DLED_DATA_PIN=23
    -DLED_CLOCK_PIN=18
build_src_filter = +<../test/wokwi_test.cpp>  # Añadir esta línea
```

2. Compilar:
```bash
pio run -e esp32dev
```

3. Iniciar simulación:
   - Presionar `F1` → "Wokwi: Start Simulator"
   - O clic en ícono Wokwi

4. Observar:
   - Monitor serial muestra logs de inicialización
   - LEDs ejecutan test RGB (rojo → verde → azul)
   - Efecto rainbow inicia automáticamente

**Opción 2: Copiar código directamente en Wokwi web**

1. Ir a [wokwi.com](https://wokwi.com/projects/new/esp32)
2. Reemplazar código con contenido de `test/wokwi_test.cpp`
3. Copiar `diagram.json` al proyecto
4. Iniciar simulación

### Acceder a la Interfaz Web

1. Una vez iniciada la simulación, el ESP32 se conecta a WiFi
2. Monitor serial muestra:
   ```
   IP Address: 192.168.1.100
   Access web interface at: http://localhost
   ```

3. Abrir navegador en: **http://localhost** (gracias al port forwarding)

4. Interfaz web permite:
   - Cambiar efectos (Rainbow, Solid, Chase, POV Test)
   - Ajustar brillo (10-255)
   - Seleccionar colores predefinidos
   - Ver estado actual

### Ejemplo de Output del Monitor Serial

```
=================================
POV-Line Wokwi Test
=================================

Initializing LEDs... OK
Initializing LittleFS... OK
Total space: 1441792 bytes
Used space: 0 bytes
Connecting to WiFi... ......... OK
IP Address: 192.168.1.100
Access web interface at: http://localhost (via Wokwi port forwarding)
Starting web server... OK

Running LED test...
LED test complete

=================================
Setup complete!
Starting main loop...
=================================
```

---

## Uso del Proyecto Completo

### Simulación del Firmware Principal

Para simular el proyecto completo (`main.cpp` con todos los módulos):

1. **Asegurar que `platformio.ini` apunta a `src/`**:
   ```ini
   [env:esp32dev]
   board = esp32dev
   build_flags =
       -DESP32_CLASSIC
       -DLED_DATA_PIN=23
       -DLED_CLOCK_PIN=18
   # NO usar build_src_filter personalizado
   ```

2. **Compilar**:
   ```bash
   pio run -e esp32dev
   ```

3. **Iniciar Wokwi**:
   - `F1` → "Wokwi: Start Simulator"

4. **Funcionalidades disponibles**:
   - ✅ Todos los módulos del sistema
   - ✅ Gestión de imágenes (upload, list, delete)
   - ✅ Motor POV con streaming
   - ✅ Efectos decorativos
   - ✅ API REST completa (12 endpoints)
   - ✅ Configuración WiFi y MQTT
   - ⚠️ LittleFS con limitaciones (ver abajo)

### Limitaciones en Modo Completo

**LittleFS en Wokwi**:
- Soportado pero con tamaño limitado
- No persiste entre sesiones de simulación
- Subir imágenes BMP puede ser lento

**Workaround para imágenes**:
1. Crear imágenes de prueba pequeñas (10x10 px)
2. Convertir a Base64 y embedder en código
3. O usar modo POV Test con patrones generados

**WiFi**:
- Se conecta automáticamente a "Wokwi-GUEST" (red virtual)
- IP asignada: 192.168.1.x (varía)
- Puerto 80 mapeado a `localhost:80`

**MQTT**:
- Requiere broker externo accesible via internet
- O usar broker local con port forwarding

---

## Características Soportadas

### ✅ Completamente Funcional

| Característica | Estado | Notas |
|----------------|--------|-------|
| Control LED APA102 | ✅ | Visualización en tiempo real |
| Efectos Rainbow | ✅ | 100% funcional |
| Efectos Solid Color | ✅ | 100% funcional |
| Efectos Chase | ✅ | 100% funcional |
| POV Test (patrón) | ✅ | Simula POV sin imagen real |
| Servidor web | ✅ | Accesible en localhost |
| API REST | ✅ | Todos los endpoints |
| WiFi | ✅ | Red virtual Wokwi-GUEST |
| Monitor serial | ✅ | Logs completos |
| Control de brillo | ✅ | 100% funcional |

### ⚠️ Parcialmente Funcional

| Característica | Estado | Limitación |
|----------------|--------|------------|
| LittleFS | ⚠️ | Tamaño limitado, no persiste |
| Upload de imágenes | ⚠️ | Funciona pero lento |
| POV con imagen BMP | ⚠️ | Requiere imagen pequeña |
| MQTT | ⚠️ | Requiere broker externo |

### ❌ No Soportado

| Característica | Estado | Alternativa |
|----------------|--------|-------------|
| Persistencia de archivos | ❌ | Usar imágenes embebidas en código |
| Sensores externos | ❌ | No aplicable al proyecto actual |

---

## Limitaciones

### 1. Performance

**Velocidad de simulación**:
- Wokwi simula en tiempo real, pero puede ser más lento que hardware
- Efectos LED complejos pueden tener lag visual
- Upload de archivos grandes es lento

**Workaround**:
- Reducir número de LEDs para testing (de 144 a 30-50)
- Usar imágenes pequeñas (10x10 o 20x20 px)

### 2. Sistema de Archivos

**LittleFS**:
- Espacio limitado (típicamente 1.4 MB)
- No persiste entre sesiones de simulación
- Cada reinicio borra contenido

**Workaround**:
```cpp
// Crear imagen de prueba embebida en setup()
void createTestImage() {
  File file = LittleFS.open("/test.rgb", "w");
  // Escribir datos de imagen simple...
  file.close();
}
```

### 3. WiFi y Red

**Red Virtual**:
- Solo "Wokwi-GUEST" disponible
- IP dinámica (cambia en cada sesión)
- Port forwarding requerido para acceso externo

**MQTT**:
- No hay broker MQTT integrado en Wokwi
- Requiere broker público (ej: test.mosquitto.org)
- O configurar broker local con forwarding

### 4. Componentes Hardware

**APA102**:
- Simulación visual correcta
- Timing exacto puede diferir de hardware real
- Sin limitaciones prácticas para desarrollo

---

## Troubleshooting

### Problema: Simulación no inicia

**Síntoma**: Wokwi no arranca, error al compilar

**Solución**:
1. Verificar que el firmware esté compilado:
   ```bash
   pio run -e esp32dev
   ```

2. Verificar que existe `.pio/build/esp32dev/firmware.elf`

3. Revisar `wokwi.toml` para ruta correcta del ELF

4. Reiniciar VS Code y Wokwi extension

### Problema: LEDs no se ven

**Síntoma**: Compilación OK pero LEDs negros

**Solución**:
1. Verificar conexiones en `diagram.json`:
   ```json
   [ "led1:DI", "esp:D23", "green", [ "v0" ] ]
   [ "led1:CI", "esp:D18", "blue", [ "v0" ] ]
   ```

2. Verificar que `LED_DATA_PIN` y `LED_CLOCK_PIN` coinciden:
   ```cpp
   #define LED_DATA_PIN    23
   #define LED_CLOCK_PIN   18
   ```

3. Revisar que `FastLED.show()` se llama en loop

4. Verificar brillo no está en 0:
   ```cpp
   FastLED.setBrightness(128);
   ```

### Problema: No puedo acceder a http://localhost

**Síntoma**: Navegador no conecta al servidor web

**Solución**:
1. Verificar que WiFi se conectó (monitor serial):
   ```
   IP Address: 192.168.1.100
   ```

2. Verificar port forwarding en `wokwi.toml`:
   ```toml
   [[net.forward]]
   from = "localhost:80"
   to = "target:80"
   ```

3. Esperar 10-15 segundos después de "Web server started"

4. Probar diferentes navegadores (Chrome, Firefox)

5. Verificar que no hay otro servicio usando puerto 80:
   ```bash
   sudo lsof -i :80
   ```

### Problema: LittleFS no funciona

**Síntoma**: "LittleFS mount failed" en serial

**Solución**:
1. En Wokwi, LittleFS puede tardar en iniciar
2. Usar formato forzado:
   ```cpp
   if (!LittleFS.begin(true)) {  // true = format if failed
     Serial.println("Failed");
   }
   ```

3. Alternativamente, usar SPIFFS en lugar de LittleFS:
   ```cpp
   #include <SPIFFS.h>
   SPIFFS.begin(true);
   ```

### Problema: Upload muy lento

**Síntoma**: Subir imágenes toma minutos

**Solución**:
1. Reducir tamaño de imagen (max 10 KB)
2. Usar formato RGB565 en lugar de BMP
3. Para testing, usar imágenes generadas en código

### Problema: Monitor serial vacío

**Síntoma**: No aparecen logs

**Solución**:
1. Verificar baudrate en código y monitor:
   ```cpp
   Serial.begin(115200);
   ```

2. En Wokwi, abrir "Serial Monitor" desde panel lateral

3. Verificar que conexiones de serial están en diagram.json:
   ```json
   [ "esp:TX0", "$serialMonitor:RX", "", [] ]
   [ "esp:RX0", "$serialMonitor:TX", "", [] ]
   ```

---

## Recursos

### Documentación Oficial Wokwi

- **Guía ESP32**: [docs.wokwi.com/guides/esp32](https://docs.wokwi.com/guides/esp32)
- **WiFi en Wokwi**: [docs.wokwi.com/guides/esp32-wifi](https://docs.wokwi.com/guides/esp32-wifi)
- **diagram.json**: [docs.wokwi.com/diagram-format](https://docs.wokwi.com/diagram-format)
- **Componentes soportados**: [docs.wokwi.com/getting-started/supported-hardware](https://docs.wokwi.com/getting-started/supported-hardware)

### Ejemplos de Proyectos Wokwi

- **ESP32 AsyncWebServer**: [github.com/wokwi/esp32-async-web-server-example](https://github.com/wokwi/esp32-async-web-server-example)
- **APA102 LEDs**: Buscar en [wokwi.com](https://wokwi.com) → "APA102"
- **LittleFS ESP32**: [wokwi.com/projects/410024921729348609](https://wokwi.com/projects/410024921729348609)

### Comunidad y Soporte

- **GitHub Issues**: [github.com/wokwi/wokwi-features/issues](https://github.com/wokwi/wokwi-features/issues)
- **Discord**: Canal oficial de Wokwi
- **Documentación del Proyecto**: Ver [docs/README.md](README.md)

---

## Workflow Recomendado

### Para Desarrollo de Nuevas Features

1. **Prototipar en Wokwi**:
   - Crear módulo nuevo
   - Probar lógica básica
   - Validar en simulador

2. **Refinar en Hardware**:
   - Ajustar timings
   - Verificar performance real
   - Testing con carga completa

3. **Iterar**:
   - Bugs encontrados en HW → reproducir en Wokwi
   - Fixes en Wokwi → validar en HW

### Para Testing de Cambios

**Antes de commit**:
```bash
# 1. Compilar
pio run -e esp32dev

# 2. Probar en Wokwi (test simplificado)
# Modificar platformio.ini para usar test/wokwi_test.cpp
# Iniciar simulación

# 3. Verificar LEDs y web interface

# 4. Probar en hardware si disponible

# 5. Commit
git commit -m "Feature: ..."
```

---

## Ejemplos de Uso

### Ejemplo 1: Probar Nuevo Efecto LED

**Objetivo**: Añadir efecto "Sparkle"

1. **Crear efecto en `effects.cpp`**:
   ```cpp
   void Effects::sparkle(CRGB color, uint8_t density) {
     fadeToBlackBy(leds, numLeds, 10);
     for (int i = 0; i < density; i++) {
       leds[random16(numLeds)] = color;
     }
   }
   ```

2. **Añadir a test Wokwi** (`test/wokwi_test.cpp`):
   ```cpp
   void updateSparkle() {
     fadeToBlackBy(leds, NUM_LEDS, 10);
     for (int i = 0; i < 5; i++) {
       leds[random(NUM_LEDS)] = CRGB::White;
     }
   }
   ```

3. **Compilar y probar en Wokwi**:
   - Ver efecto visual en tiempo real
   - Ajustar parámetros (density, fade rate)

4. **Una vez satisfecho, implementar en código principal**

### Ejemplo 2: Debuggear Problema de Performance

**Objetivo**: POV engine muy lento

1. **Añadir timing logs**:
   ```cpp
   unsigned long start = millis();
   imageParser.getColumn(file, col, buffer, height);
   Serial.printf("Column read time: %lu ms\n", millis() - start);
   ```

2. **Ejecutar en Wokwi**:
   - Monitor serial muestra timings
   - Identificar cuellos de botella

3. **Optimizar**:
   ```cpp
   // Antes: leer byte por byte
   // Después: leer en chunks
   file.read((uint8_t*)buffer, height * 3);
   ```

4. **Re-probar en Wokwi** → confirmar mejora

### Ejemplo 3: Validar API REST

**Objetivo**: Verificar endpoint `/api/brightness`

1. **Iniciar simulación Wokwi**

2. **Abrir navegador en http://localhost**

3. **Probar endpoint con cURL**:
   ```bash
   curl -X POST http://localhost/api/settings \
     -H "Content-Type: application/json" \
     -d '{"brightness": 200}'
   ```

4. **Verificar en Wokwi**:
   - LEDs cambian de brillo visualmente
   - Monitor serial muestra log

5. **Confirmar respuesta JSON**:
   ```bash
   curl http://localhost/api/status
   # {"brightness": 200, ...}
   ```

---

## Consejos y Mejores Prácticas

### 1. Desarrollo Iterativo

- ✅ Probar pequeños cambios en Wokwi primero
- ✅ Validar lógica antes de cargar a hardware
- ✅ Usar monitor serial extensivamente

### 2. Debugging Visual

- ✅ Wokwi muestra LEDs en tiempo real → usar para debugging de patrones
- ✅ Añadir colores de debug (ej: rojo = error, verde = OK)
- ✅ Usar `Serial.printf()` liberalmente

### 3. Performance Testing

- ⚠️ Simulación es más lenta que HW real
- ⚠️ No confiar en timings exactos
- ✅ Usar para validar lógica, no performance absoluta

### 4. Gestión de Archivos

- ✅ Para testing rápido: embedder datos en código
- ✅ Para testing realista: usar imágenes pequeñas
- ⚠️ No esperar persistencia entre sesiones

---

## Conclusión

Wokwi Simulator es una herramienta poderosa para desarrollar y probar el proyecto POV-Line sin hardware físico.

**Ventajas clave**:
- ✅ Desarrollo rápido de prototipos
- ✅ Debugging visual de LEDs
- ✅ Testing de servidor web
- ✅ Validación de lógica
- ✅ Gratis y accesible

**Limitaciones**:
- ⚠️ Performance diferente de HW real
- ⚠️ LittleFS no persiste
- ⚠️ Requiere imágenes pequeñas

**Workflow recomendado**:
1. Desarrollar lógica en Wokwi
2. Refinar en hardware real
3. Iterar según sea necesario

---

**Última actualización**: 2026-01-18
**Versión del documento**: 1.0.0
**Autor**: darkjavi
