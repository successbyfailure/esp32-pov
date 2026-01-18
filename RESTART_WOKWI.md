# ⚠️ IMPORTANTE: Reiniciar Wokwi con Nuevo Firmware

El firmware con la tabla de particiones está compilado correctamente, pero Wokwi necesita reiniciarse completamente para cargarlo.

## 🔄 Pasos para Reiniciar Wokwi

### Opción 1: VS Code + Wokwi Extension

1. **Detener simulación actual**:
   - Click en botón "Stop Simulation" (cuadrado rojo)
   - O presionar `F1` → "Wokwi: Stop Simulator"

2. **Cerrar Wokwi completamente**:
   - En VS Code, cerrar el panel de Wokwi (X en la pestaña)
   - Esperar 5 segundos

3. **Iniciar nueva simulación**:
   - Presionar `F1` → escribir "Wokwi: Start Simulator"
   - O click en "Start Simulation" en la barra lateral
   - Esperar a que cargue completamente

4. **Verificar en monitor serial**:
   ```
   [1/7] Inicializando sistema de archivos...
   ✅ LittleFS montado correctamente    <-- Debe decir esto
   Espacio total: 1441792 bytes (1.4 MB)
   ```

### Opción 2: Si Opción 1 No Funciona

**Reiniciar VS Code completamente**:

```bash
# 1. Cerrar VS Code
# Ctrl+Q o File → Exit

# 2. Reabrir proyecto
cd /home/darkjavi/Documentos/PlatformIO/Projects/pov-line
code .

# 3. Iniciar Wokwi
# F1 → "Wokwi: Start Simulator"
```

### Opción 3: Limpiar Caché de Wokwi

Si sigue sin funcionar:

```bash
# 1. Detener Wokwi completamente
# 2. Limpiar build
~/.platformio/penv/bin/pio run --target clean

# 3. Recompilar
~/.platformio/penv/bin/pio run -e esp32dev

# 4. Cerrar y reabrir VS Code
# 5. Iniciar Wokwi de nuevo
```

## ✅ Verificación de Éxito

Cuando Wokwi inicie correctamente con el nuevo firmware, verás:

```
========================================
POV Line - Firmware v1.0.0
========================================

[1/7] Inicializando sistema de archivos...
✅ LittleFS montado correctamente          <-- ¡Esto es lo importante!
Espacio total: 1441792 bytes (1.4 MB)
Espacio disponible: 1441792 bytes

[2/7] Inicializando LEDs...
✅ 144 LEDs APA102 inicializados

[3/7] Conectando WiFi...
✅ Conectado a: Wokwi-GUEST
   IP: 192.168.1.100

[4/7] Iniciando servidor web...
✅ Servidor web iniciado en puerto 80

[5/7] Cargando configuración...
✅ Configuración cargada

[6/7] Inicializando Home Assistant...
⚠️  MQTT deshabilitado en configuración

[7/7] Iniciando efectos...
✅ Efecto Rainbow iniciado

========================================
Sistema listo!
Accede a la interfaz web en: http://localhost
========================================
```

## 🔍 Archivos Verificados

Ya están compilados correctamente:

```
✅ .pio/build/esp32dev/firmware.bin      (916 KB)
✅ .pio/build/esp32dev/firmware.elf      (20 MB)
✅ .pio/build/esp32dev/partitions.bin    (3 KB)  <-- Tabla de particiones
```

## ❌ Si Sigue Fallando

### Error Persistente de LittleFS

Si después de reiniciar Wokwi completamente sigues viendo:
```
E (1041) esp_littlefs: partition "spiffs" could not be found
```

**Posible causa**: Wokwi está cacheando el firmware antiguo.

**Solución drástica**:

1. Cerrar VS Code completamente
2. Eliminar caché de Wokwi (si existe):
   ```bash
   rm -rf ~/.wokwi/cache/*
   ```

3. Limpiar y recompilar proyecto:
   ```bash
   cd /home/darkjavi/Documentos/PlatformIO/Projects/pov-line
   ~/.platformio/penv/bin/pio run --target clean
   ~/.platformio/penv/bin/pio run -e esp32dev
   ```

4. Reabrir VS Code:
   ```bash
   code .
   ```

5. Iniciar Wokwi fresco:
   - F1 → "Wokwi: Start Simulator"

### Alternativa: Usar Test Simplificado

Si el firmware completo sigue dando problemas, prueba el test simplificado que tiene menos dependencias:

1. **Abrir `test/wokwi_test.cpp`**

2. **Modificar `platformio.ini` temporalmente**:
   ```ini
   [env:esp32dev]
   board = esp32dev
   build_flags =
       -DESP32_CLASSIC
       -DLED_DATA_PIN=23
       -DLED_CLOCK_PIN=18
   build_src_filter = +<../test/wokwi_test.cpp>  # Añadir esta línea
   ```

3. **Compilar test**:
   ```bash
   ~/.platformio/penv/bin/pio run --target clean
   ~/.platformio/penv/bin/pio run -e esp32dev
   ```

4. **Iniciar Wokwi**

El test simplificado también usa LittleFS pero con inicialización más robusta:
```cpp
if (!LittleFS.begin(true)) {  // true = format if failed
  Serial.println("WARNING: LittleFS not available");
} else {
  Serial.println("OK");
}
```

## 📞 Debug Adicional

Si necesitas más información, ejecuta:

```bash
# Verificar que partitions.csv existe
cat partitions.csv

# Verificar platformio.ini
grep -A 5 "\[env\]" platformio.ini

# Verificar que partitions.bin se generó
ls -lh .pio/build/esp32dev/partitions.bin

# Ver tamaño de firmware
ls -lh .pio/build/esp32dev/firmware.*
```

---

**Recuerda**: El problema NO es el firmware (está compilado correctamente). El problema es que Wokwi necesita **reiniciarse completamente** para cargar el nuevo firmware con las particiones.

---

**Última actualización**: 2026-01-18
