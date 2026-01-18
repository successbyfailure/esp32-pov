# Documentación Técnica POV-Line

## Índice General

Esta carpeta contiene toda la documentación técnica para desarrolladores y agentes de programación que trabajen con el proyecto POV-Line.

---

## Documentos Disponibles

### 1. [ARCHITECTURE.md](ARCHITECTURE.md) - Arquitectura del Sistema
**Para**: Comprender la estructura completa del sistema

**Contenido**:
- Visión general del sistema
- Diagrama de componentes
- Descripción detallada de cada módulo
- Flujos de datos
- Gestión de memoria
- Consideraciones de rendimiento
- Extensibilidad del sistema

**Cuándo leer**:
- Al iniciar desarrollo en el proyecto
- Para entender cómo encajan las piezas
- Antes de añadir nueva funcionalidad mayor

**Nivel**: Intermedio a Avanzado

---

### 2. [API_REFERENCE.md](API_REFERENCE.md) - Referencia de API
**Para**: Consulta rápida de endpoints y funciones

**Contenido**:
- REST API completa (endpoints HTTP)
- MQTT Topics (Home Assistant)
- Estructuras de datos C++
- API interna C++ (clases y métodos)
- Códigos de error
- Ejemplos de requests/responses

**Cuándo usar**:
- Al integrar con el sistema
- Al desarrollar cliente HTTP/MQTT
- Para consultar firma de funciones
- Debugging de API calls

**Nivel**: Todos los niveles

---

### 3. [DEVELOPMENT_GUIDE.md](DEVELOPMENT_GUIDE.md) - Guía de Desarrollo
**Para**: Implementar nuevas features y debugging

**Contenido**:
- Configuración del entorno
- Patrones de código usados
- Cómo añadir nuevos efectos
- Cómo añadir nuevos formatos de imagen
- Cómo añadir nuevos endpoints API
- Testing y debugging
- Optimización de código
- Troubleshooting común
- Mejores prácticas

**Cuándo usar**:
- Al añadir nueva funcionalidad
- Al encontrar bugs
- Al optimizar rendimiento
- Setup inicial del proyecto

**Nivel**: Intermedio a Avanzado

---

### 4. [EXAMPLES.md](EXAMPLES.md) - Ejemplos de Código
**Para**: Aprender con ejemplos prácticos

**Contenido**:
- Ejemplos básicos (setup mínimo)
- Control de LEDs (patrones, animaciones)
- Manejo de imágenes
- Web API (clientes Python, JavaScript)
- MQTT/Home Assistant
- Casos de uso avanzados
- Performance testing

**Cuándo usar**:
- Para aprender rápido
- Como plantilla para tu código
- Para entender casos de uso reales
- Al integrar sistemas externos

**Nivel**: Principiante a Avanzado

---

### 5. [WOKWI_SIMULATION.md](WOKWI_SIMULATION.md) - Simulación con Wokwi
**Para**: Desarrollar y probar sin hardware físico

**Contenido**:
- Configuración de Wokwi Simulator
- Archivos diagram.json y wokwi.toml
- Test simplificado para validación rápida
- Uso del proyecto completo en simulador
- Características soportadas y limitaciones
- Troubleshooting de simulación
- Workflow recomendado con Wokwi

**Cuándo usar**:
- Al desarrollar sin acceso a hardware
- Para prototipar nuevas features
- Testing visual de efectos LED
- Validación de servidor web y API
- Aprendizaje y experimentación

**Nivel**: Principiante a Intermedio

---

## Guía de Lectura Recomendada

### Para Nuevos Desarrolladores

1. **Primero**: Lee el [README.md](../README.md) principal del proyecto
2. **Segundo**: Ojea [ARCHITECTURE.md](ARCHITECTURE.md) - secciones "Visión General" y "Módulos del Sistema"
3. **Tercero**: Revisa [EXAMPLES.md](EXAMPLES.md) - sección "Ejemplos Básicos"
4. **Cuarto**: Consulta [DEVELOPMENT_GUIDE.md](DEVELOPMENT_GUIDE.md) - sección "Configuración del Entorno"

### Para Añadir Nueva Funcionalidad

1. **Primero**: [ARCHITECTURE.md](ARCHITECTURE.md) - entender el módulo relevante
2. **Segundo**: [DEVELOPMENT_GUIDE.md](DEVELOPMENT_GUIDE.md) - sección "Cómo Añadir Funcionalidades"
3. **Tercero**: [EXAMPLES.md](EXAMPLES.md) - buscar ejemplos similares
4. **Cuarto**: [API_REFERENCE.md](API_REFERENCE.md) - documentar nueva API

### Para Integración Externa

1. **Primero**: [API_REFERENCE.md](API_REFERENCE.md) - sección "REST API Endpoints"
2. **Segundo**: [EXAMPLES.md](EXAMPLES.md) - sección "Web API" o "MQTT/Home Assistant"
3. **Tercero**: [DEVELOPMENT_GUIDE.md](DEVELOPMENT_GUIDE.md) - sección "Testing con cURL"

### Para Debugging

1. **Primero**: [DEVELOPMENT_GUIDE.md](DEVELOPMENT_GUIDE.md) - sección "Troubleshooting"
2. **Segundo**: [DEVELOPMENT_GUIDE.md](DEVELOPMENT_GUIDE.md) - sección "Testing y Debugging"
3. **Tercero**: [ARCHITECTURE.md](ARCHITECTURE.md) - entender flujo de datos del problema
4. **Cuarto**: [API_REFERENCE.md](API_REFERENCE.md) - verificar códigos de error

### Para Simulación con Wokwi

1. **Primero**: [WOKWI_SIMULATION.md](WOKWI_SIMULATION.md) - sección "Configuración del Entorno"
2. **Segundo**: [WOKWI_SIMULATION.md](WOKWI_SIMULATION.md) - sección "Uso del Test Simplificado"
3. **Tercero**: [EXAMPLES.md](EXAMPLES.md) - adaptar ejemplos para Wokwi
4. **Cuarto**: [WOKWI_SIMULATION.md](WOKWI_SIMULATION.md) - sección "Troubleshooting" si hay problemas

---

## Mapa Conceptual del Sistema

```
┌─────────────────────────────────────────────────┐
│              Usuario / Cliente                   │
│  (Web Browser, Home Assistant, Script Python)   │
└───────────────┬─────────────────────────────────┘
                │
    ┌───────────┼────────────┐
    │           │            │
    ▼           ▼            ▼
┌────────┐ ┌─────────┐ ┌──────────┐
│  HTTP  │ │  MQTT   │ │  Serial  │
│  API   │ │ Topics  │ │  Debug   │
└────┬───┘ └────┬────┘ └────┬─────┘
     │          │            │
     └──────────┼────────────┘
                │
                ▼
    ┌───────────────────────┐
    │    ESP32 Firmware      │
    │  (main.cpp + modules)  │
    └───────────────────────┘
                │
        ┌───────┼───────┐
        │       │       │
        ▼       ▼       ▼
    ┌────┐  ┌─────┐ ┌──────┐
    │LED │  │FS   │ │WiFi  │
    │Strip│ │Images│ │MQTT  │
    └────┘  └─────┘ └──────┘
```

---

## Convenciones de Documentación

### Formato de Código

**C++:**
```cpp
// Comentario
void function() {
  // Implementación
}
```

**Python:**
```python
# Comentario
def function():
    # Implementación
    pass
```

**JSON:**
```json
{
  "key": "value"
}
```

**YAML:**
```yaml
key: value
```

### Niveles de Prioridad

En la documentación usamos estos indicadores:

- **IMPORTANTE**: Información crítica
- **NOTA**: Información adicional útil
- **ADVERTENCIA**: Cuidado, puede causar problemas
- **TIP**: Sugerencia para mejor resultado

### Símbolos Usados

- `→` : Resultado o efecto
- `▼` : Siguiente paso
- `✓` : Correcto / Recomendado
- `✗` : Incorrecto / No recomendado

---

## Versionado de Documentación

**Versión Actual**: 1.0.0

### Changelog

#### v1.0.0 (2026-01-18)
- Documentación inicial completa
- ARCHITECTURE.md creado
- API_REFERENCE.md creado
- DEVELOPMENT_GUIDE.md creado
- EXAMPLES.md creado

---

## Contribuir a la Documentación

### Reglas

1. **Claridad**: Escribe para que un agente de IA o desarrollador nuevo pueda entender
2. **Ejemplos**: Incluye ejemplos de código funcionales
3. **Actualización**: Mantén la documentación sincronizada con el código
4. **Formato**: Sigue las convenciones de Markdown
5. **Verificación**: Prueba los ejemplos antes de documentar

### Qué Documentar

**SÍ documentar**:
- Nuevos módulos
- Nuevos endpoints API
- Cambios en estructuras de datos
- Nuevos patrones de código
- Casos de uso comunes
- Soluciones a problemas frecuentes

**NO es necesario documentar**:
- Cambios menores de implementación
- Refactoring que no cambia API
- Fixes de typos
- Mejoras de rendimiento internas (a menos que sean significativas)

### Template para Nueva Feature

```markdown
## Nombre de la Feature

**Descripción**: Una línea explicando qué hace

**Ubicación**: `src/modulo.cpp`

**Uso**:
```cpp
// Ejemplo de código
```

**Parámetros**:
- `param1`: Descripción

**Returns**: Qué devuelve

**Notas**:
- Nota importante 1
- Nota importante 2
```

---

## Contacto y Soporte

### Para Desarrolladores

- **Issues**: Reportar en GitHub (si aplicable)
- **Pull Requests**: Contribuciones bienvenidas
- **Discusiones**: Foro del proyecto (si existe)

### Para Agentes de IA

Esta documentación está optimizada para ser leída por agentes de programación:
- Estructura clara y jerárquica
- Ejemplos de código completos y funcionales
- Patrones explícitos
- Convenciones documentadas
- Casos de uso detallados

**Recomendaciones para agentes**:
1. Leer ARCHITECTURE.md primero para contexto
2. Consultar API_REFERENCE.md para detalles técnicos
3. Usar EXAMPLES.md como plantillas
4. Seguir patrones en DEVELOPMENT_GUIDE.md

---

## Recursos Externos

### Documentación de Librerías

- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/)
- [FastLED](https://github.com/FastLED/FastLED/wiki)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [ArduinoJson](https://arduinojson.org/v7/)
- [PubSubClient](https://pubsubclient.knolleary.net/)

### Tutoriales ESP32

- [ESP32 GPIO Reference](https://randomnerdtutorials.com/esp32-pinout-reference-gpios/)
- [ESP32 Deep Sleep](https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/)
- [ESP32 Web Server](https://randomnerdtutorials.com/esp32-web-server-arduino-ide/)

### Home Assistant

- [MQTT Discovery](https://www.home-assistant.io/integrations/mqtt/#mqtt-discovery)
- [Light Platform](https://www.home-assistant.io/integrations/light.mqtt/)

---

## Glosario

**APA102**: Tipo de LED strip direccionable con data y clock separados (SPI)

**BMP**: Bitmap image format sin compresión

**CRGB**: Estructura de FastLED para color RGB (8-bit por canal)

**ESP32**: Microcontrolador WiFi/Bluetooth de Espressif

**FPS**: Frames Per Second (velocidad de animación)

**HSV**: Hue, Saturation, Value (modelo de color)

**LittleFS**: Sistema de archivos para flash en ESP32

**MQTT**: Message Queue Telemetry Transport (protocolo IoT)

**POV**: Persistence of Vision (persistencia de visión)

**RGB565**: Formato de color comprimido (5 bits R, 6 bits G, 5 bits B)

**SPI**: Serial Peripheral Interface (protocolo de comunicación)

**Watchdog**: Timer que reinicia el sistema si se bloquea

---

## Licencia de la Documentación

Esta documentación es parte del proyecto POV-Line y está disponible bajo la misma licencia que el código fuente.

---

**Última actualización**: 2026-01-18
**Mantenedor**: darkjavi
**Versión del Proyecto**: 1.0.0
