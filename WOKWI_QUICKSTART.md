# Wokwi Quick Start - POV-Line

Guía rápida para iniciar la simulación en Wokwi en menos de 5 minutos.

## ⚡ Inicio Rápido

### Opción 1: VS Code + Wokwi Extension (Recomendado)

```bash
# 1. Instalar Wokwi extension (una sola vez)
code --install-extension wokwi.wokwi-vscode

# 2. Compilar el proyecto
cd /home/darkjavi/Documentos/PlatformIO/Projects/pov-line
~/.platformio/penv/bin/pio run -e esp32dev

# 3. Abrir VS Code en el proyecto
code .

# 4. Iniciar simulación
# Presionar F1 → escribir "Wokwi: Start Simulator" → Enter
# O usar botón "Start Simulation" en barra lateral
```

**Listo!** El simulador iniciará y podrás:
- Ver los LEDs en tiempo real
- Abrir http://localhost para la interfaz web
- Monitorear logs en el panel de serial

### Opción 2: Wokwi Web (Navegador)

1. Ir a [wokwi.com](https://wokwi.com/projects/new/esp32)
2. Copiar contenido de `test/wokwi_test.cpp` al editor
3. Hacer clic en `diagram.json` en panel izquierdo
4. Copiar contenido de nuestro `diagram.json`
5. Presionar "Start Simulation"

---

## 🎮 Qué Probar

### 1. Ver Efecto Rainbow (Automático)

Al iniciar, verás un efecto rainbow en los 144 LEDs virtuales.

### 2. Acceder a Interfaz Web

1. Esperar mensaje en serial: `Access web interface at: http://localhost`
2. Abrir navegador en **http://localhost**
3. Verás panel de control con:
   - Botones de efectos
   - Slider de brillo
   - Selector de colores

### 3. Cambiar Efectos

**Desde la interfaz web**:
- Click en "🌈 Rainbow"
- Click en "⚫ Solid Color"
- Click en "🔴 Chase"
- Click en "📊 POV Test"

**Desde terminal (cURL)**:
```bash
# Cambiar a efecto Chase
curl http://localhost/effect?mode=chase

# Cambiar brillo a máximo
curl http://localhost/brightness?value=255

# Cambiar color sólido a rojo
curl "http://localhost/color?r=255&g=0&b=0"

# Ver estado
curl http://localhost/status
```

### 4. Ajustar Brillo

Mover el slider en la interfaz web y ver cómo los LEDs cambian de intensidad.

### 5. Seleccionar Colores

Click en los botones de color (rojo, verde, azul, etc.) para color sólido.

---

## 📊 Monitor Serial

El monitor serial mostrará logs completos:

```
========================================
POV Line - Firmware v1.0.0
========================================

[1/7] Inicializando sistema de archivos...
LittleFS montado correctamente
Espacio total: 1441792 bytes (1.4 MB)
Espacio disponible: 1441792 bytes

[2/7] Inicializando LEDs...
144 LEDs APA102 inicializados

[3/7] Conectando WiFi...
Conectado a: Wokwi-GUEST
IP: 192.168.1.100

[4/7] Iniciando servidor web...
Servidor web iniciado en puerto 80

...

Sistema listo!
Accede a la interfaz web en: http://localhost
```

---

## ❓ Troubleshooting Rápido

### Problema: Simulación no inicia

**Solución**:
```bash
# Limpiar y recompilar
~/.platformio/penv/bin/pio run --target clean
~/.platformio/penv/bin/pio run -e esp32dev
```

### Problema: No veo los LEDs

**Verificar**:
1. Que aparezca mensaje "144 LEDs APA102 inicializados" en serial
2. Que el brillo no esté en 0
3. Reiniciar simulación (Stop → Start)

### Problema: http://localhost no funciona

**Solución**:
1. Esperar 10-15 segundos después de "Servidor web iniciado"
2. Verificar que aparezca IP en serial
3. Verificar que `wokwi.toml` existe en el directorio raíz
4. Cerrar otros servicios usando puerto 80:
   ```bash
   # En Linux/Mac
   sudo lsof -i :80
   ```

### Problema: Error de LittleFS

**Ya está solucionado!** El proyecto incluye:
- `partitions.csv` con tabla de particiones
- `platformio.ini` configurado correctamente
- Solo recompilar con `pio run`

---

## 🎯 Siguientes Pasos

### Explorar el Proyecto Completo

El test simplificado (`test/wokwi_test.cpp`) es solo una demo. Para el proyecto completo:

1. **Ver arquitectura**:
   ```bash
   cat docs/ARCHITECTURE.md
   ```

2. **Ver API completa**:
   ```bash
   cat docs/API_REFERENCE.md
   ```

3. **Ver ejemplos**:
   ```bash
   cat docs/EXAMPLES.md
   ```

### Modificar Efectos LED

Edita `test/wokwi_test.cpp`:

```cpp
// Añadir tu propio efecto
void updateMiEfecto() {
  // Tu código aquí
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(i * 5, 255, 255);
  }
}
```

Recompila y reinicia simulación.

### Probar en Hardware Real

Una vez validado en Wokwi:

```bash
# 1. Conectar ESP32 via USB
# 2. Cargar firmware
~/.platformio/penv/bin/pio run --target upload

# 3. Cargar interfaz web
~/.platformio/penv/bin/pio run --target uploadfs

# 4. Monitorear
~/.platformio/penv/bin/pio device monitor
```

---

## 📚 Documentación Completa

Para más detalles:

- **Simulación**: [docs/WOKWI_SIMULATION.md](docs/WOKWI_SIMULATION.md)
- **Desarrollo**: [docs/DEVELOPMENT_GUIDE.md](docs/DEVELOPMENT_GUIDE.md)
- **API**: [docs/API_REFERENCE.md](docs/API_REFERENCE.md)
- **Ejemplos**: [docs/EXAMPLES.md](docs/EXAMPLES.md)
- **Test**: [test/README.md](test/README.md)

---

## 🆘 Ayuda

**Recursos**:
- Wokwi Docs: [docs.wokwi.com](https://docs.wokwi.com)
- FastLED Wiki: [github.com/FastLED/FastLED/wiki](https://github.com/FastLED/FastLED/wiki)
- PlatformIO: [docs.platformio.org](https://docs.platformio.org)

**Issues conocidos**:
- LittleFS no persiste entre sesiones → Normal en Wokwi
- Simulación más lenta que HW real → Esperado
- Upload de archivos lento → Usar imágenes pequeñas

---

**¡Listo para simular!** 🚀

Presiona `F1` → "Wokwi: Start Simulator" y disfruta del POV-Line virtual.

---

**Última actualización**: 2026-01-18
**Versión**: 1.0.0
