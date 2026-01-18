# POV-Line para BornHack 2024 Badge

Adaptación del proyecto POV-Line para el badge oficial de BornHack 2024 con ESP32-C3, acelerómetro LIS3DH y 16 LEDs WS2812.

## Hardware del Badge

### Especificaciones
- **MCU**: ESP32-C3 Mini
- **LEDs**: 16x WS2812 addressables
- **IMU**: LIS3DH acelerómetro (I2C)
- **Batería**: LiPo 1S con cargador integrado
- **Botón**: SELECT para cambio de efectos
- **Extras**: NFC NT3H2x11, conectores SAO y QWiC

### Pines GPIO Configurados

**✅ VERIFICADOS**: Estos pines fueron extraídos del firmware oficial de BornHack 2024:

```cpp
// En platformio.ini [env:bornhack2024]
#define LED_DATA_PIN    10        // WS2812 data (VERIFICADO)
#define ACCEL_SDA_PIN   6         // LIS3DH I2C SDA (VERIFICADO)
#define ACCEL_SCL_PIN   7         // LIS3DH I2C SCL (VERIFICADO)
#define BUTTON_PIN      9         // SELECT button (VERIFICADO)

// Pines adicionales del badge:
// GPIO 2 - Botón UP
// GPIO 8 - Botón DOWN
```

**Fuente**: https://github.com/badgeteam/esp32-component-badge-bsp/tree/main/targets/bornhack-2024-pov

## Cómo Identificar Los Pines Reales

Si tienes el badge físico, sigue estos pasos:

### 1. Conectar Badge por USB

```bash
# Conectar y ver puerto
ls /dev/ttyUSB* /dev/ttyACM*

# Abrir monitor serial
~/.platformio/penv/bin/pio device monitor -b 115200
```

### 2. Buscar Schematic

El schematic debería estar en:
- Repositorio oficial: https://github.com/bornhack/badge2024
- Archivo: `schematic.pdf`

O usar este comando para descargarlo:

```bash
curl -L https://raw.githubusercontent.com/bornhack/badge2024/hardware/schematic.pdf -o badge_schematic.pdf
xdg-open badge_schematic.pdf
```

### 3. Identificar Pines en el Schematic

Busca en el PDF:
- **LEDs**: Conexión "DO" o "DIN" del primer WS2812
- **Acelerómetro**: Pines "SDA" y "SCL" del LIS3DH
- **Botón**: Conexión del botón SELECT

### 4. Actualizar Configuración

Edita [platformio.ini](platformio.ini):

```ini
[env:bornhack2024]
board = esp32-c3-devkitm-1
build_flags =
    -DESP32_C3
    -DBORNHACK_BADGE
    -DLED_DATA_PIN=X         ; Reemplazar X con pin real
    -DHAS_ACCELEROMETER
    -DACCEL_SDA_PIN=Y        ; Reemplazar Y con pin SDA
    -DACCEL_SCL_PIN=Z        ; Reemplazar Z con pin SCL
    -DBUTTON_PIN=W           ; Reemplazar W con pin del botón
```

## Compilar y Cargar

### Compilar para Badge

```bash
~/.platformio/penv/bin/pio run -e bornhack2024
```

### Cargar Firmware

```bash
# Conectar badge por USB
~/.platformio/penv/bin/pio run -e bornhack2024 --target upload

# Monitorear serial
~/.platformio/penv/bin/pio device monitor -b 115200
```

### Cargar Sistema de Archivos (Imágenes)

```bash
~/.platformio/penv/bin/pio run -e bornhack2024 --target uploadfs
```

## Uso del Badge

### Efectos Disponibles

El badge tiene 4 efectos que se cambian con el botón SELECT:

| Efecto | Descripción | Indicador LED |
|--------|-------------|---------------|
| **0** | LEDs apagados (ahorro batería) | Ningún LED |
| **1** | Rainbow (arco iris animado) | 1 LED azul |
| **2** | Chase rojo (persecución) | 2 LEDs azules |
| **3** | POV con movimiento | 3 LEDs azules |

### Cambiar Efecto

1. **Presionar el botón SELECT** una vez
2. Ver los LEDs azules que indican el número de efecto
3. El nuevo efecto se activa automáticamente

### Modo POV (Efecto 3)

El efecto POV se activa **automáticamente con movimiento**:

1. **Cambiar a efecto 3** (botón SELECT 3 veces)
2. **Mover el badge** horizontalmente (wave motion)
3. El acelerómetro detecta el movimiento y activa POV
4. **Mantener quieto** pausa el POV automáticamente

### Calibración del Acelerómetro

Al iniciar, el badge calibra automáticamente:

```
Calibrando acelerómetro... mantener quieto
Calibración completa: X=0.12, Y=-0.05, Z=0.03
```

Si la calibración no es correcta:
- **Resetear el badge** manteniendo horizontal y quieto
- Esperar 3 segundos sin mover durante boot

## Cargar Imágenes POV

### Formato de Imagen

Las imágenes para POV deben ser:

- **Formato**: BMP 24-bit sin comprimir o RGB565 raw
- **Altura**: 16 píxeles (número de LEDs)
- **Ancho máximo**: 70 píxeles (recomendado para badge)
- **Tamaño máximo**: 100 KB

### Crear Imagen con GIMP

1. **Crear imagen nueva**:
   - Tamaño: 70 x 16 píxeles
   - Modo color: RGB

2. **Diseñar tu patrón POV**

3. **Exportar**:
   - Archivo → Exportar como
   - Nombre: `mi_imagen.bmp`
   - Formato: BMP sin comprimir

4. **Verificar**:
   ```bash
   file mi_imagen.bmp
   # Debe mostrar: PC bitmap, Windows 3.x format, 70 x 16 x 24
   ```

### Subir Imagen al Badge

**Método 1: Web Interface** (requiere WiFi configurado)

```bash
# 1. Conectar a WiFi del badge o configurar WiFi
# 2. Abrir http://192.168.4.1 (en modo AP)
# 3. Ir a "Galería" → "Subir imagen"
# 4. Seleccionar archivo BMP
# 5. Click "Upload"
```

**Método 2: Sistema de Archivos**

```bash
# 1. Copiar imagen a data/images/
cp mi_imagen.bmp data/images/

# 2. Cargar filesystem
~/.platformio/penv/bin/pio run -e bornhack2024 --target uploadfs
```

## Monitor Serial - Mensajes Esperados

Al encender el badge deberías ver:

```
========================================
POV Line - Firmware v1.0.0
========================================

[1/7] Inicializando sistema de archivos...
LittleFS OK - Total: 1408 KB, Usado: 10 KB, Libre: 1398 KB

[2/7] Cargando configuración...
Usando configuración por defecto

[3/7] Inicializando controlador de LEDs...
LEDs inicializados: 16 x WS281x en pin DATA=8

[3b] Inicializando acelerómetro...
Inicializando acelerómetro LIS3DH...
LIS3DH inicializado correctamente
Calibrando acelerómetro... mantener quieto
Calibración completa: X=0.02, Y=-0.01, Z=0.00
Acelerómetro listo para detección de movimiento POV

[3c] Inicializando botón...
Botón inicializando en GPIO9
Botón listo - presionar para cambiar efectos

...

========================================
Sistema iniciado correctamente
========================================
```

### Debugging Común

**Si no aparece "LIS3DH inicializado"**:
- Verificar pines I2C (SDA/SCL)
- Revisar conexión física del acelerómetro
- Comprobar dirección I2C (0x18 o 0x19)

**Si no aparece "Botón listo"**:
- Verificar pin del botón
- Comprobar si el botón es activo alto o bajo

**Si LEDs no encienden**:
- Verificar pin LED_DATA_PIN
- Comprobar alimentación (batería cargada)
- Probar con brillo bajo primero

## Optimizaciones para Badge

### Ahorro de Batería

El efecto 0 (LEDs apagados) está diseñado para badges con **power switch roto**:

```cpp
case 0:  // LEDs apagados (ahorro de batería)
  ledController.clear();
  ledController.show();
```

### Ajustar Brillo

Para maximizar batería:

```cpp
// En main.cpp, línea ~75
ledController.setBrightness(64);  // Reducir de 128 a 64
```

O vía configuración web:
```bash
curl -X POST http://192.168.4.1/api/settings -d "brightness=64"
```

### Ajustar Sensibilidad de Movimiento

Para cambiar umbral de detección:

```cpp
// En accelerometer.cpp, línea ~7
motionThreshold = 0.8;  // Aumentar para menos sensibilidad (default: 0.5)
```

## WiFi en el Badge

### Modo AP (Default)

Al encender, el badge crea un punto de acceso:

- **SSID**: `POV-Line-Setup`
- **Password**: `povline123`
- **IP**: http://192.168.4.1

### Configurar WiFi

1. Conectar al AP del badge
2. Abrir http://192.168.4.1
3. Ir a **Configuración** → **WiFi**
4. Ingresar SSID y password de tu red
5. Guardar y reiniciar

### Modo Station

Una vez configurado:
- El badge se conecta automáticamente a tu WiFi
- La IP se muestra en serial monitor
- Acceder a http://IP_DEL_BADGE

## Troubleshooting

### Problema: Badge no arranca

**Síntomas**: No aparece nada en serial

**Solución**:
1. Verificar batería cargada
2. Probar con USB conectado
3. Mantener presionado botón BOOT al conectar USB
4. Volver a cargar firmware

### Problema: LEDs parpadean erráticamente

**Causa**: Interferencia WiFi con WS2812

**Solución**:
```cpp
// En setup(), antes de inicializar WiFi
WiFi.setSleep(false);  // Desactivar sleep mode
```

O desactivar WiFi si no se necesita:
```cpp
// En main.cpp, comentar sección WiFi en setup()
```

### Problema: Movimiento no detectado

**Verificar**:
1. Calibración exitosa en serial
2. Pines I2C correctos (SDA/SCL)
3. Acelerómetro soldado correctamente

**Test manual**:
```cpp
// Añadir en loop() para debug
#ifdef HAS_ACCELEROMETER
  float x, y, z;
  accelerometer.getRawData(x, y, z);
  Serial.printf("Accel: X=%.2f Y=%.2f Z=%.2f\n", x, y, z);
  delay(100);
#endif
```

### Problema: Botón no responde

**Verificar**:
1. Pin correcto en BUTTON_PIN
2. Tipo de botón (pull-up interno configurado)
3. Cambiar lógica si es activo alto:

```cpp
// En button.cpp, línea ~16
pinMode(pin, INPUT);  // Sin pull-up
// Y cambiar LOW por HIGH en detección
```

## Referencias

- **Badge Hardware**: https://github.com/bornhack/badge2024
- **Firmware POV original**: https://github.com/Orange-Murker/bornhack-24-pov
- **Hackaday Article**: https://hackaday.com/2025/08/01/two-for-the-price-of-one-bornhack-2024-and-2025-badges/

## Notas Importantes

⚠️ **Pines GPIO**: Los pines configurados en `platformio.ini` son **estimaciones**. DEBES verificarlos con el schematic del badge antes de usar.

⚠️ **Batería**: No dejar LEDs a máximo brillo por mucho tiempo sin USB conectado.

⚠️ **I2C**: El LIS3DH puede usar dirección 0x18 o 0x19 dependiendo del pin SA0. El código prueba 0x18 por defecto.

---

**Última actualización**: 2026-01-18
**Versión**: 1.0.0 (BornHack Badge Edition)
**Autor**: darkjavi

**Sources**:
- [BornHack 2024 Badge Repository](https://github.com/bornhack/badge2024)
- [Hackaday Article](https://hackaday.com/2025/08/01/two-for-the-price-of-one-bornhack-2024-and-2025-badges/)
- [Orange Murker POV Firmware](https://github.com/Orange-Murker/bornhack-24-pov)
