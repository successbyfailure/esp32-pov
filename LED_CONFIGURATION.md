# Configuración de Tipo de LED

El proyecto POV-Line soporta dos tipos de tiras LED: **WS281x** (WS2811/WS2812/WS2812B) y **APA102**.

## Tiras LED Soportadas

### WS2811 / WS2812 / WS2812B (NeoPixel)
- **Conexión**: 1 pin (DATA solamente)
- **Protocolo**: Timing basado en pulsos (requiere timing preciso)
- **Velocidad**: ~800 kHz
- **Ventajas**:
  - Solo requiere 1 pin GPIO
  - Más económicas
  - Ampliamente disponibles (NeoPixel de Adafruit)
- **Desventajas**:
  - Sensibles a interferencias
  - No tienen control independiente de brillo global
  - Timing crítico (puede causar parpadeos con WiFi activo)

### APA102 (DotStar)
- **Conexión**: 2 pines (DATA + CLOCK)
- **Protocolo**: SPI (más robusto)
- **Velocidad**: Hasta varios MHz
- **Ventajas**:
  - Más estables y robustas
  - Control de brillo independiente por hardware
  - Menos sensibles a interferencias WiFi
  - Refresh rate más alto
- **Desventajas**:
  - Requieren 2 pines GPIO
  - Ligeramente más caras

## Configuración por Defecto

Por defecto, el proyecto está configurado para **WS2811** con **144 LEDs**:

```cpp
#define DEFAULT_LED_TYPE LED_TYPE_WS2811
#define DEFAULT_NUM_LEDS 144
```

## Cómo Cambiar el Tipo de LED

### Opción 1: Via Interfaz Web (Recomendado)

1. Conectarse al dispositivo vía WiFi
2. Abrir http://192.168.4.1 (en modo AP) o la IP asignada
3. Ir a **Configuración** → **Hardware LED**
4. Seleccionar tipo de LED:
   - WS2811 (también para WS2812/WS2812B)
   - APA102
5. Configurar número de LEDs (1-300)
6. Hacer click en **Guardar**
7. El dispositivo se reiniciará automáticamente

### Opción 2: Via API REST

```bash
# Configurar para WS2811 con 144 LEDs
curl -X POST http://192.168.4.1/api/settings \
  -d "ledType=WS2811&numLeds=144"

# Configurar para APA102 con 100 LEDs
curl -X POST http://192.168.4.1/api/settings \
  -d "ledType=APA102&numLeds=100"
```

### Opción 3: Editar config.json Manualmente

1. Conectarse via Serial Monitor
2. Acceder al sistema de archivos LittleFS
3. Editar `/config.json`:

```json
{
  "ledType": "WS2811",
  "numLeds": 144,
  ...
}
```

4. Reiniciar el dispositivo

### Opción 4: Cambiar Valor por Defecto en Código

Editar `src/config.h`:

```cpp
// Para WS2811/WS2812
#define DEFAULT_LED_TYPE LED_TYPE_WS2811
#define DEFAULT_NUM_LEDS 144

// Para APA102
#define DEFAULT_LED_TYPE LED_TYPE_APA102
#define DEFAULT_NUM_LEDS 100
```

Recompilar y cargar firmware.

## Conexión Física

### WS2811/WS2812 (1 pin)

```
ESP32 GPIO23 (DATA) ──────► WS2811 DIN
ESP32 VIN (5V)      ──────► WS2811 VCC
ESP32 GND           ──────► WS2811 GND
```

**Importante**:
- Los WS281x son sensibles a voltaje. Asegúrate de usar 5V (no 3.3V).
- Añade un capacitor de 1000µF entre VCC y GND cerca de la tira.
- Añade una resistencia de 220-470Ω en el pin DATA.

### APA102 (2 pines)

```
ESP32 GPIO23 (DATA)  ──────► APA102 DATA
ESP32 GPIO18 (CLOCK) ──────► APA102 CLOCK
ESP32 VIN (5V)       ──────► APA102 VCC
ESP32 GND            ──────► APA102 GND
```

**Importante**:
- APA102 también prefiere 5V aunque son más tolerantes a 3.3V.
- Añade un capacitor de 1000µF entre VCC y GND.
- No requieren resistencias en DATA/CLOCK (protocolo SPI robusto).

## Alimentación

**⚠️ IMPORTANTE**: Nunca alimentes tiras LED largas directamente desde el ESP32.

- **1-10 LEDs**: Puedes alimentar desde ESP32 (máx ~500mA)
- **10-50 LEDs**: Usa fuente externa 5V/2A
- **50-144 LEDs**: Usa fuente externa 5V/5A o más
- **144+ LEDs**: Usa fuente externa 5V/10A+

### Cálculo de Corriente

**WS2811/WS2812**:
- Por LED a máximo brillo blanco: ~60mA
- 144 LEDs a máximo: 144 × 60mA = 8.64A

**APA102**:
- Por LED a máximo brillo blanco: ~20mA por color (60mA total)
- 144 LEDs a máximo: 144 × 60mA = 8.64A

**Recomendación**: Limita el brillo a 50% (config.brightness = 128) para reducir consumo:
- 144 LEDs al 50%: ~4.32A

## Limitaciones de Cada Tipo

### WS281x
- Máximo ~1000 LEDs en teoría (limitado por timing y memoria)
- Recomendado: máx 300 LEDs
- Problemas conocidos:
  - Parpadeos con WiFi activo (usar canal WiFi fijo)
  - Primera actualización puede ser lenta

### APA102
- Máximo varios miles de LEDs (limitado solo por memoria)
- Recomendado: máx 500 LEDs
- Problemas conocidos:
  - Consumo ligeramente mayor por el chip de control

## Wokwi Simulator

El proyecto incluye dos archivos de configuración Wokwi:

- **diagram.json**: Configurado para WS2811 (NeoPixel)
- **diagram-apa102.json**: Configurado para APA102

Para simular con APA102:

```bash
# Renombrar archivos
mv diagram.json diagram-ws2811.json
mv diagram-apa102.json diagram.json

# Recompilar y ejecutar en Wokwi
pio run -e esp32dev
```

## Verificación del Tipo de LED

### Via Monitor Serial

Al iniciar, el sistema muestra:

```
[3/7] Inicializando controlador de LEDs...
LEDs inicializados: 144 x WS281x en pin DATA=23
```

o

```
LEDs inicializados: 144 x APA102 en pines DATA=23, CLOCK=18
```

### Via API REST

```bash
curl http://192.168.4.1/api/status
```

Respuesta:
```json
{
  "ledType": "WS2811",
  "numLeds": 144,
  ...
}
```

## Troubleshooting

### LEDs no encienden (WS281x)
1. Verifica que estés usando 5V (no 3.3V)
2. Añade resistencia de 220-470Ω en DATA
3. Revisa que DATA esté conectado a DIN (no DOUT)
4. Prueba con menos LEDs primero (10-20)

### LEDs no encienden (APA102)
1. Verifica conexiones DATA y CLOCK (no intercambiar)
2. Revisa polaridad de alimentación
3. Prueba con menos LEDs primero

### LEDs parpadean o colores incorrectos (WS281x)
1. WiFi puede causar interferencia → desactivar temporalmente
2. Añade capacitor 1000µF cerca de la tira
3. Reduce longitud de cables DATA
4. Usa cable apantallado para DATA

### Colores incorrectos en algunos LEDs (APA102)
1. Revisa calidad de conexiones CLOCK
2. Reduce velocidad SPI (no configurable actualmente)
3. Usa cables más cortos

### Brillo muy bajo
1. Verifica configuración de brillo (0-255)
2. Revisa que alimentación sea adecuada
3. Prueba con color blanco sólido

### No puedo cambiar número de LEDs
1. Verifica que esté en rango 1-300
2. Revisa memoria disponible (RAM)
3. Reinicia después de cambiar

## FAQ

### ¿Puedo mezclar WS2811 y APA102?
No, solo puedes usar un tipo a la vez.

### ¿Puedo usar más de 300 LEDs?
Técnicamente sí, pero requiere modificar MAX_LEDS en config.h. Ten en cuenta limitaciones de RAM del ESP32.

### ¿Funciona con otras tiras como SK6812?
SK6812 usa mismo protocolo que WS281x → configurar como WS2811.

### ¿Qué tipo de LED recomiendas?
- **Principiantes / Precio**: WS2811/WS2812
- **Estabilidad / Performance**: APA102
- **POV aplicaciones**: APA102 (refresh rate mayor)

### ¿Cuántos LEDs puedo conectar?
Depende de:
- **RAM disponible**: ~300 LEDs máximo en ESP32 classic
- **Alimentación**: Según fuente disponible
- **Aplicación POV**: Recomendado 61-144 LEDs

## Referencias

- [FastLED Documentation](https://github.com/FastLED/FastLED/wiki)
- [WS2812 Datasheet](https://cdn-shop.adafruit.com/datasheets/WS2812.pdf)
- [APA102 Datasheet](https://cdn-shop.adafruit.com/product-files/2343/APA102C.pdf)
- [Adafruit NeoPixel Überguide](https://learn.adafruit.com/adafruit-neopixel-uberguide)

---

**Última actualización**: 2026-01-18
**Versión**: 1.0.0
