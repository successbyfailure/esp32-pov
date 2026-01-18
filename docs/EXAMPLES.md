# Ejemplos de Código - POV-Line

Colección de ejemplos prácticos para trabajar con el sistema POV-Line.

---

## Tabla de Contenidos

1. [Ejemplos Básicos](#ejemplos-básicos)
2. [Control de LEDs](#control-de-leds)
3. [Manejo de Imágenes](#manejo-de-imágenes)
4. [Web API](#web-api)
5. [MQTT/Home Assistant](#mqtthome-assistant)
6. [Casos de Uso Avanzados](#casos-de-uso-avanzados)

---

## Ejemplos Básicos

### Setup Mínimo

```cpp
#include <Arduino.h>
#include "led_controller.h"
#include "effects.h"

void setup() {
  Serial.begin(115200);

  // Inicializar LEDs
  if (ledController.init(144)) {
    Serial.println("LEDs OK");
  }

  // Efecto inicial
  effects.rainbow(10);
}

void loop() {
  effects.update();
  delay(1);
}
```

---

### Configuración WiFi Básica

```cpp
#include "wifi_manager.h"
#include "web_server.h"

void setup() {
  Serial.begin(115200);

  wifiManager.init();

  // Modo AP para configuración
  if (!wifiManager.connectWiFi("MiWiFi", "password123")) {
    wifiManager.startAP();
    Serial.printf("AP Mode - IP: %s\n", wifiManager.getIP().c_str());
  } else {
    Serial.printf("Connected - IP: %s\n", wifiManager.getIP().c_str());
  }

  webServer.init();
}

void loop() {
  wifiManager.loop();
  delay(1);
}
```

---

## Control de LEDs

### Ejemplo 1: Patrón de Color Simple

```cpp
void patternSimple() {
  uint16_t numLeds = ledController.getNumLeds();

  // Rojo en primera mitad
  for (int i = 0; i < numLeds / 2; i++) {
    ledController.setPixel(i, CRGB::Red);
  }

  // Azul en segunda mitad
  for (int i = numLeds / 2; i < numLeds; i++) {
    ledController.setPixel(i, CRGB::Blue);
  }

  ledController.show();
}
```

---

### Ejemplo 2: Gradiente de Color

```cpp
void gradientRainbow() {
  CRGB* pixels = ledController.getPixels();
  uint16_t numLeds = ledController.getNumLeds();

  for (int i = 0; i < numLeds; i++) {
    uint8_t hue = map(i, 0, numLeds - 1, 0, 255);
    pixels[i] = CHSV(hue, 255, 255);
  }

  ledController.show();
}
```

---

### Ejemplo 3: Animación de Ola

```cpp
class WaveEffect {
private:
  uint8_t wavePosition;
  unsigned long lastUpdate;

public:
  WaveEffect() : wavePosition(0), lastUpdate(0) {}

  void update() {
    if (millis() - lastUpdate < 50) return;

    ledController.clear();

    uint16_t numLeds = ledController.getNumLeds();

    for (int i = 0; i < numLeds; i++) {
      // Calcular distancia a la ola
      int distance = abs(i - wavePosition);

      // Brillo basado en distancia
      uint8_t brightness = 255 - (distance * 30);
      if (distance > 8) brightness = 0;

      ledController.setPixel(i, CHSV(160, 255, brightness));
    }

    ledController.show();

    wavePosition = (wavePosition + 1) % numLeds;
    lastUpdate = millis();
  }
};

// Uso
WaveEffect waveEffect;

void loop() {
  waveEffect.update();
  delay(1);
}
```

---

### Ejemplo 4: Fire Effect

```cpp
#define COOLING  55
#define SPARKING 120

byte heat[MAX_LEDS];

void fire() {
  uint16_t numLeds = ledController.getNumLeds();

  // Step 1: Cool down
  for(int i = 0; i < numLeds; i++) {
    heat[i] = qsub8(heat[i], random8(0, ((COOLING * 10) / numLeds) + 2));
  }

  // Step 2: Heat from each cell drifts up
  for(int k = numLeds - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3: Randomly ignite new sparks
  if(random8() < SPARKING) {
    int y = random8(7);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }

  // Step 4: Map heat to LED colors
  CRGB* pixels = ledController.getPixels();
  for(int j = 0; j < numLeds; j++) {
    CRGB color = HeatColor(heat[j]);
    pixels[j] = color;
  }

  ledController.show();
}

void loop() {
  fire();
  delay(30);
}
```

---

## Manejo de Imágenes

### Ejemplo 1: Cargar y Reproducir Imagen

```cpp
#include "pov_engine.h"
#include "image_manager.h"

void setup() {
  Serial.begin(115200);
  LittleFS.begin(true);

  ledController.init(144);
  imageManager.init();

  // Cargar y reproducir
  if (povEngine.loadImage("test.bmp")) {
    povEngine.setSpeed(30);
    povEngine.setLoopMode(true);
    povEngine.setOrientation(POV_VERTICAL);
    povEngine.play();
  } else {
    Serial.println("Error cargando imagen");
  }
}

void loop() {
  povEngine.update();
  delay(1);
}
```

---

### Ejemplo 2: Listar y Mostrar Info de Imágenes

```cpp
#include "image_manager.h"

void listAllImages() {
  auto images = imageManager.listImages();

  Serial.println("\n=== Imágenes Disponibles ===");
  Serial.printf("Total: %d imágenes\n\n", images.size());

  for (const auto& img : images) {
    Serial.printf("Nombre: %s\n", img.filename);
    Serial.printf("  Dimensiones: %dx%d\n", img.width, img.height);
    Serial.printf("  Tamaño: %d bytes (%.1f KB)\n",
                  img.fileSize, img.fileSize / 1024.0);
    Serial.printf("  Formato: %s\n",
                  img.format == 0 ? "BMP" : "RGB565");
    Serial.println();
  }

  Serial.printf("Espacio libre: %d KB\n",
                imageManager.getFreeSpace() / 1024);
}

void setup() {
  Serial.begin(115200);
  LittleFS.begin(true);
  imageManager.init();

  listAllImages();
}
```

---

### Ejemplo 3: Procesar Imagen Columna por Columna

```cpp
#include "image_parser.h"

void processImageManually() {
  const char* filename = "/images/test.bmp";

  ImageInfo info;
  if (!imageParser.parseImageInfo(filename, info)) {
    Serial.println("Error parseando imagen");
    return;
  }

  Serial.printf("Procesando: %dx%d\n", info.width, info.height);

  CRGB columnBuffer[MAX_LEDS];

  for (uint16_t col = 0; col < info.width; col++) {
    if (!imageParser.getColumn(filename, col, columnBuffer, info.height)) {
      Serial.printf("Error en columna %d\n", col);
      continue;
    }

    // Procesar columna
    Serial.printf("Columna %d: R=%d G=%d B=%d\n",
                  col,
                  columnBuffer[0].r,
                  columnBuffer[0].g,
                  columnBuffer[0].b);

    // Mostrar en LEDs
    for (int i = 0; i < info.height && i < ledController.getNumLeds(); i++) {
      ledController.setPixel(i, columnBuffer[i]);
    }
    ledController.show();

    delay(33);  // ~30 FPS
  }
}
```

---

### Ejemplo 4: Crear Imagen RGB565 Programáticamente

```cpp
void createRGB565Image() {
  const uint16_t width = 100;
  const uint16_t height = 144;

  File file = LittleFS.open("/images/generated.rgb", "w");
  if (!file) {
    Serial.println("Error creando archivo");
    return;
  }

  // Escribir header
  file.write((uint8_t*)"R565", 4);
  file.write((uint8_t*)&width, 2);
  file.write((uint8_t*)&height, 2);

  // Escribir píxeles
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      // Crear gradiente
      uint8_t r = map(x, 0, width - 1, 0, 31);   // 5 bits
      uint8_t g = map(y, 0, height - 1, 0, 63);  // 6 bits
      uint8_t b = 15;                             // 5 bits

      uint16_t rgb565 = (r << 11) | (g << 5) | b;
      file.write((uint8_t*)&rgb565, 2);
    }
  }

  file.close();
  Serial.println("Imagen creada: /images/generated.rgb");

  imageManager.refreshList();
}
```

---

## Web API

### Ejemplo 1: Cliente HTTP Simple (desde PC/Raspberry Pi)

```python
#!/usr/bin/env python3
import requests
import time

ESP32_IP = "192.168.1.100"
BASE_URL = f"http://{ESP32_IP}/api"

def get_status():
    """Obtiene el estado del sistema"""
    response = requests.get(f"{BASE_URL}/status")
    return response.json()

def play_image(image_name):
    """Reproduce una imagen"""
    data = {"image": image_name}
    response = requests.post(f"{BASE_URL}/play", data=data)
    return response.json()

def set_speed(fps):
    """Ajusta la velocidad"""
    data = {"speed": fps}
    response = requests.post(f"{BASE_URL}/settings", data=data)
    return response.json()

def rainbow_effect():
    """Activa efecto rainbow"""
    data = {"effect": "rainbow", "speed": 15}
    response = requests.post(f"{BASE_URL}/effect", data=data)
    return response.json()

# Ejemplo de uso
if __name__ == "__main__":
    # Obtener estado
    status = get_status()
    print(f"Estado: {status['state']}")
    print(f"IP WiFi: {status['wifiIP']}")

    # Activar rainbow
    rainbow_effect()
    time.sleep(5)

    # Reproducir imagen
    play_image("test.bmp")
    set_speed(60)

    # Monitorear progreso
    while True:
        status = get_status()
        print(f"Columna: {status['column']}/{status['totalColumns']}")
        time.sleep(1)
```

---

### Ejemplo 2: Upload de Imagen desde Python

```python
#!/usr/bin/env python3
import requests

def upload_image(filepath, esp32_ip):
    """Sube una imagen al ESP32"""
    url = f"http://{esp32_ip}/api/upload"

    with open(filepath, 'rb') as f:
        files = {'file': (filepath.split('/')[-1], f)}
        response = requests.post(url, files=files)

    return response.json()

# Uso
result = upload_image("/path/to/image.bmp", "192.168.1.100")
print(result)
```

---

### Ejemplo 3: JavaScript/Node.js Client

```javascript
const axios = require('axios');
const FormData = require('form-data');
const fs = require('fs');

const ESP32_IP = '192.168.1.100';
const BASE_URL = `http://${ESP32_IP}/api`;

async function getStatus() {
  const response = await axios.get(`${BASE_URL}/status`);
  return response.data;
}

async function playImage(imageName) {
  const params = new URLSearchParams();
  params.append('image', imageName);

  const response = await axios.post(`${BASE_URL}/play`, params);
  return response.data;
}

async function uploadImage(filepath) {
  const form = new FormData();
  form.append('file', fs.createReadStream(filepath));

  const response = await axios.post(`${BASE_URL}/upload`, form, {
    headers: form.getHeaders()
  });

  return response.data;
}

// Uso
(async () => {
  const status = await getStatus();
  console.log('Estado:', status.state);

  await uploadImage('./test.bmp');
  await playImage('test.bmp');
})();
```

---

## MQTT/Home Assistant

### Ejemplo 1: Monitorear Estado vía MQTT

```python
#!/usr/bin/env python3
import paho.mqtt.client as mqtt

MQTT_BROKER = "192.168.1.10"
MQTT_PORT = 1883

def on_connect(client, userdata, flags, rc):
    print(f"Conectado con código: {rc}")
    # Suscribirse a todos los topics
    client.subscribe("pov_line/#")

def on_message(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()
```

---

### Ejemplo 2: Controlar vía MQTT

```python
#!/usr/bin/env python3
import paho.mqtt.client as mqtt
import time

MQTT_BROKER = "192.168.1.10"
client = mqtt.Client()
client.connect(MQTT_BROKER, 1883, 60)

# Encender
client.publish("pov_line/command", "ON")
time.sleep(1)

# Ajustar brillo
client.publish("pov_line/brightness/set", "200")
time.sleep(1)

# Cambiar efecto
client.publish("pov_line/effect/set", "rainbow")
time.sleep(5)

# Apagar
client.publish("pov_line/command", "OFF")

client.disconnect()
```

---

### Ejemplo 3: Automatización Home Assistant

```yaml
# configuration.yaml
automation:
  - alias: "POV Line - Encender al atardecer"
    trigger:
      platform: sun
      event: sunset
    action:
      - service: light.turn_on
        target:
          entity_id: light.pov_line
        data:
          effect: "Rainbow"
          brightness: 180

  - alias: "POV Line - Apagar a medianoche"
    trigger:
      platform: time
      at: "00:00:00"
    action:
      - service: light.turn_off
        target:
          entity_id: light.pov_line

  - alias: "POV Line - Cambiar efecto cada hora"
    trigger:
      platform: time_pattern
      hours: "/1"
    action:
      - service: light.turn_on
        target:
          entity_id: light.pov_line
        data:
          effect: >
            {% set effects = ['POV', 'Rainbow', 'Chase'] %}
            {{ effects | random }}
```

---

## Casos de Uso Avanzados

### Ejemplo 1: Sistema de Notificaciones

```cpp
class NotificationSystem {
private:
  struct Notification {
    CRGB color;
    uint8_t priority;
    unsigned long timestamp;
  };

  std::vector<Notification> queue;

public:
  void addNotification(CRGB color, uint8_t priority = 5) {
    Notification notif;
    notif.color = color;
    notif.priority = priority;
    notif.timestamp = millis();
    queue.push_back(notif);
  }

  void update() {
    if (queue.empty()) return;

    // Mostrar notificación de mayor prioridad
    Notification* highest = nullptr;
    for (auto& notif : queue) {
      if (!highest || notif.priority > highest->priority) {
        highest = &notif;
      }
    }

    if (highest) {
      flashColor(highest->color);

      // Eliminar después de 3 segundos
      if (millis() - highest->timestamp > 3000) {
        queue.erase(
          std::remove_if(queue.begin(), queue.end(),
            [highest](const Notification& n) { return &n == highest; }),
          queue.end()
        );
      }
    }
  }

private:
  void flashColor(CRGB color) {
    static unsigned long lastFlash = 0;
    static bool state = false;

    if (millis() - lastFlash > 500) {
      state = !state;
      ledController.fill(state ? color : CRGB::Black);
      ledController.show();
      lastFlash = millis();
    }
  }
};

NotificationSystem notifications;

// Uso
void onMQTTMessage(const char* topic, const char* payload) {
  if (strcmp(topic, "alerts/high") == 0) {
    notifications.addNotification(CRGB::Red, 10);
  } else if (strcmp(topic, "alerts/medium") == 0) {
    notifications.addNotification(CRGB::Yellow, 5);
  }
}

void loop() {
  notifications.update();
}
```

---

### Ejemplo 2: Visualizador de Audio

```cpp
#ifdef ESP32_CLASSIC

class AudioVisualizer {
private:
  const uint8_t MIC_PIN = 34;
  uint16_t samples[64];
  uint8_t sampleIndex;

public:
  AudioVisualizer() : sampleIndex(0) {
    pinMode(MIC_PIN, INPUT);
  }

  void update() {
    // Leer micrófono
    uint16_t value = analogRead(MIC_PIN);
    samples[sampleIndex] = value;
    sampleIndex = (sampleIndex + 1) % 64;

    // Calcular promedio
    uint32_t sum = 0;
    for (int i = 0; i < 64; i++) {
      sum += samples[i];
    }
    uint16_t average = sum / 64;

    // Calcular nivel
    uint16_t level = abs(value - average);
    level = constrain(level, 0, 500);

    // Visualizar
    uint16_t numLedsOn = map(level, 0, 500, 0, ledController.getNumLeds());

    ledController.clear();
    for (int i = 0; i < numLedsOn; i++) {
      uint8_t hue = map(i, 0, ledController.getNumLeds(), 0, 255);
      ledController.setPixel(i, CHSV(hue, 255, 255));
    }
    ledController.show();
  }
};

AudioVisualizer audioViz;

void loop() {
  audioViz.update();
  delay(10);
}

#endif
```

---

### Ejemplo 3: Reloj Binario

```cpp
#include <time.h>

class BinaryClock {
public:
  void init() {
    // Configurar timezone
    configTime(3600, 3600, "pool.ntp.org");  // GMT+1, DST
  }

  void update() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      return;
    }

    int hour = timeinfo.tm_hour;
    int minute = timeinfo.tm_min;
    int second = timeinfo.tm_sec;

    displayBinary(hour, 0, CRGB::Red);      // Horas en rojo
    displayBinary(minute, 6, CRGB::Green);  // Minutos en verde
    displayBinary(second, 12, CRGB::Blue);  // Segundos en azul

    ledController.show();
  }

private:
  void displayBinary(int value, int startLed, CRGB color) {
    ledController.clear();

    for (int bit = 0; bit < 6; bit++) {
      if (value & (1 << bit)) {
        ledController.setPixel(startLed + bit, color);
      }
    }
  }
};

BinaryClock binaryClock;

void setup() {
  // ... WiFi setup ...
  binaryClock.init();
}

void loop() {
  static unsigned long lastUpdate = 0;

  if (millis() - lastUpdate > 1000) {
    binaryClock.update();
    lastUpdate = millis();
  }
}
```

---

### Ejemplo 4: Control por Sensor de Proximidad

```cpp
class ProximitySensor {
private:
  const uint8_t TRIG_PIN = 5;
  const uint8_t ECHO_PIN = 18;

public:
  void init() {
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
  }

  uint16_t getDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH);
    uint16_t distance = duration * 0.034 / 2;  // cm

    return distance;
  }
};

ProximitySensor proximity;

void setup() {
  ledController.init(144);
  proximity.init();
}

void loop() {
  uint16_t distance = proximity.getDistance();

  if (distance < 50) {
    // Persona cerca - iniciar POV
    if (!povEngine.isPlaying()) {
      povEngine.loadImage("welcome.bmp");
      povEngine.play();
    }
  } else {
    // Nadie cerca - modo ahorro
    if (povEngine.isPlaying()) {
      povEngine.stop();
    }
    effects.solidColor(CRGB::Black);
  }

  povEngine.update();
  delay(100);
}
```

---

## Performance Testing

### Ejemplo: Benchmark de FPS

```cpp
void benchmarkPOV() {
  unsigned long frames = 0;
  unsigned long startTime = millis();

  povEngine.loadImage("test.bmp");
  povEngine.play();

  while (millis() - startTime < 10000) {  // 10 segundos
    povEngine.update();
    if (povEngine.getCurrentColumn() == 0) {
      frames++;
    }
    delay(1);
  }

  povEngine.stop();

  float fps = frames / 10.0;
  Serial.printf("FPS promedio: %.2f\n", fps);
}
```

---

### Ejemplo: Monitor de Memoria

```cpp
void memoryMonitor() {
  Serial.println("\n=== Memory Stats ===");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Min free heap: %d bytes\n", ESP.getMinFreeHeap());
  Serial.printf("Heap size: %d bytes\n", ESP.getHeapSize());
  Serial.printf("PSRAM free: %d bytes\n", ESP.getFreePsram());

  Serial.println("\n=== LittleFS Stats ===");
  Serial.printf("Total: %d bytes\n", LittleFS.totalBytes());
  Serial.printf("Used: %d bytes\n", LittleFS.usedBytes());
  Serial.printf("Free: %d bytes\n",
                LittleFS.totalBytes() - LittleFS.usedBytes());

  Serial.println("\n=== LED Controller ===");
  Serial.printf("Num LEDs: %d\n", ledController.getNumLeds());
  Serial.printf("Buffer size: %d bytes\n",
                ledController.getNumLeds() * 3);
}
```

---

**Última actualización:** 2026-01-18
**Versión:** 1.0.0
