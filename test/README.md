# Test Directory

Este directorio contiene archivos de prueba y validación para el proyecto POV-Line.

## Archivos

### wokwi_test.cpp

**Propósito**: Test simplificado para validación rápida en Wokwi Simulator.

**Incluye**:
- Control básico de tira LED APA102 con FastLED
- 4 efectos LED: Rainbow, Solid Color, Chase, POV Test
- Servidor web asíncrono con interfaz HTML
- Control de brillo y colores via API REST
- Inicialización de LittleFS
- Conexión WiFi automática (Wokwi-GUEST)

**Uso**:

1. **En Wokwi for VS Code**:
   ```bash
   # Modificar platformio.ini temporalmente
   # [env:esp32dev]
   # build_src_filter = +<../test/wokwi_test.cpp>

   pio run -e esp32dev
   # F1 → "Wokwi: Start Simulator"
   ```

2. **En Wokwi web**:
   - Copiar contenido de `wokwi_test.cpp`
   - Pegar en nuevo proyecto ESP32
   - Copiar `diagram.json` del directorio raíz
   - Iniciar simulación

3. **Acceder a interfaz web**:
   - Abrir http://localhost en navegador
   - Controlar efectos, brillo y colores

**Diferencias con main.cpp**:
- No incluye gestión completa de imágenes
- No incluye MQTT/Home Assistant
- No incluye configuración persistente completa
- Enfocado en validación rápida de componentes básicos

**Ventajas**:
- ✅ Compilación más rápida
- ✅ Menos dependencias
- ✅ Ideal para aprender FastLED
- ✅ Testing rápido de efectos LED
- ✅ Validación de servidor web

**Cuándo usar**:
- Para probar efectos LED nuevos
- Para aprender el sistema sin complejidad
- Para validar hardware básico (ESP32 + LEDs)
- Para demos y presentaciones

## Estructura del Test

```cpp
// Setup básico
void setup() {
  1. Inicializar Serial
  2. Inicializar LEDs (FastLED)
  3. Inicializar LittleFS
  4. Conectar WiFi
  5. Inicializar servidor web
  6. Test RGB de LEDs
}

// Loop simple
void loop() {
  updateEffect();  // Actualiza efecto actual
}
```

## Efectos Incluidos

### 1. Rainbow
Arco iris animado que recorre toda la tira.

```cpp
void updateRainbow() {
  fill_rainbow(leds, NUM_LEDS, effectCounter, 255 / NUM_LEDS);
  effectCounter += rainbowSpeed;
}
```

### 2. Solid Color
Color sólido en toda la tira (configurable via web).

```cpp
void updateSolid() {
  fill_solid(leds, NUM_LEDS, solidColor);
}
```

### 3. Chase
Efecto de persecución con punto rojo móvil.

```cpp
void updateChase() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = effectCounter % NUM_LEDS;
  leds[pos] = CRGB::Red;
  effectCounter++;
}
```

### 4. POV Test
Simulación simple de POV con 3 secciones de color.

```cpp
void updatePOVTest() {
  // Divide tira en 3 secciones: Rojo, Verde, Azul
  // Alterna entre columnas virtuales
}
```

## API del Test

### GET /
Interfaz web HTML con controles.

### GET /effect?mode={rainbow|solid|chase|pov}
Cambia el efecto activo.

**Ejemplo**:
```bash
curl http://localhost/effect?mode=rainbow
```

### GET /brightness?value={10-255}
Ajusta brillo de los LEDs.

**Ejemplo**:
```bash
curl http://localhost/brightness?value=200
```

### GET /color?r={0-255}&g={0-255}&b={0-255}
Configura color sólido (para modo "solid").

**Ejemplo**:
```bash
curl "http://localhost/color?r=255&g=0&b=255"
```

### GET /status
Obtiene estado actual en JSON.

**Respuesta**:
```json
{
  "effect": "1",
  "brightness": 128,
  "color": {
    "r": 0,
    "g": 0,
    "b": 255
  }
}
```

## Modificaciones Comunes

### Cambiar Número de LEDs

```cpp
#define NUM_LEDS        144  // Cambiar a tu cantidad
```

### Cambiar Pines GPIO

```cpp
#define LED_DATA_PIN    23   // Para ESP32 classic
#define LED_CLOCK_PIN   18
```

### Añadir Nuevo Efecto

1. Crear función de efecto:
   ```cpp
   void updateMiEfecto() {
     // Tu lógica aquí
   }
   ```

2. Añadir a enum:
   ```cpp
   enum EffectMode {
     EFFECT_RAINBOW,
     EFFECT_SOLID,
     EFFECT_CHASE,
     EFFECT_POV_TEST,
     EFFECT_MI_EFECTO  // Nuevo
   };
   ```

3. Añadir a switch en updateEffect():
   ```cpp
   case EFFECT_MI_EFECTO:
     updateMiEfecto();
     break;
   ```

4. Añadir botón en HTML:
   ```html
   <button onclick="setEffect('mi_efecto')">Mi Efecto</button>
   ```

5. Añadir case en endpoint /effect:
   ```cpp
   else if (mode == "mi_efecto") {
     currentEffect = EFFECT_MI_EFECTO;
     effectName = "Mi Efecto";
   }
   ```

## Troubleshooting

### LEDs no se ven en Wokwi
- Verificar que diagram.json tiene conexiones correctas
- Verificar que NUM_LEDS coincide con diagram.json
- Verificar que FastLED.show() se llama en updateEffect()

### Servidor web no accesible
- Esperar 10-15 segundos después de "Web server started"
- Verificar port forwarding en wokwi.toml
- Verificar que WiFi se conectó (ver monitor serial)

### Efectos no cambian
- Abrir DevTools del navegador (F12)
- Verificar que requests fetch() tienen éxito
- Verificar que servidor responde con código 200

### Compilación falla
- Verificar que todas las librerías están instaladas
- Limpiar proyecto: `pio run --target clean`
- Re-compilar: `pio run -e esp32dev`

## Recursos Adicionales

- **Documentación Wokwi**: [docs/WOKWI_SIMULATION.md](../docs/WOKWI_SIMULATION.md)
- **FastLED Reference**: [https://github.com/FastLED/FastLED/wiki](https://github.com/FastLED/FastLED/wiki)
- **ESPAsyncWebServer**: [https://github.com/me-no-dev/ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

---

**Última actualización**: 2026-01-18
**Versión**: 1.0.0
