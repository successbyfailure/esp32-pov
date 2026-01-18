/**
 * @file wokwi_test.cpp
 * @brief Test sketch para validación en Wokwi Simulator
 *
 * Este sketch valida los componentes básicos del proyecto POV-Line:
 * - Control de tira LED APA102 con FastLED
 * - Efectos básicos (rainbow, solid colors)
 * - Servidor web asíncrono
 * - Sistema de archivos LittleFS
 *
 * Para usar en Wokwi:
 * 1. Abrir proyecto en Wokwi for VS Code
 * 2. Compilar con: pio run -e esp32dev
 * 3. Iniciar simulación en Wokwi
 * 4. Acceder al servidor web en http://localhost (via port forwarding)
 */

#include <Arduino.h>
#include <FastLED.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

// ============================================================================
// CONFIGURACIÓN
// ============================================================================

// Pines LED (ESP32 classic)
#define LED_DATA_PIN    23
#define LED_CLOCK_PIN   18
#define NUM_LEDS        144

// WiFi (Wokwi-GUEST es el AP por defecto en Wokwi)
const char* ssid = "Wokwi-GUEST";
const char* password = "";  // Open network

// ============================================================================
// OBJETOS GLOBALES
// ============================================================================

CRGB leds[NUM_LEDS];
AsyncWebServer server(80);

// ============================================================================
// VARIABLES DE ESTADO
// ============================================================================

enum EffectMode {
  EFFECT_RAINBOW,
  EFFECT_SOLID,
  EFFECT_CHASE,
  EFFECT_POV_TEST
};

EffectMode currentEffect = EFFECT_RAINBOW;
uint8_t brightness = 128;
CRGB solidColor = CRGB::Blue;
uint8_t rainbowSpeed = 5;
unsigned long lastUpdate = 0;
uint16_t effectCounter = 0;

// ============================================================================
// FUNCIONES DE EFECTOS
// ============================================================================

void updateRainbow() {
  fill_rainbow(leds, NUM_LEDS, effectCounter, 255 / NUM_LEDS);
  effectCounter += rainbowSpeed;
}

void updateSolid() {
  fill_solid(leds, NUM_LEDS, solidColor);
}

void updateChase() {
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = effectCounter % NUM_LEDS;
  leds[pos] = CRGB::Red;
  effectCounter++;
}

void updatePOVTest() {
  // Simula POV mostrando columnas de colores
  static uint8_t column = 0;

  // Divide la tira en 3 secciones de color
  for (int i = 0; i < NUM_LEDS; i++) {
    if (i < NUM_LEDS / 3) {
      leds[i] = (column % 3 == 0) ? CRGB::Red : CRGB::Black;
    } else if (i < 2 * NUM_LEDS / 3) {
      leds[i] = (column % 3 == 1) ? CRGB::Green : CRGB::Black;
    } else {
      leds[i] = (column % 3 == 2) ? CRGB::Blue : CRGB::Black;
    }
  }

  column++;
  if (column >= 30) column = 0;  // 30 "columnas" virtuales
}

void updateEffect() {
  unsigned long now = millis();
  if (now - lastUpdate < 50) return;  // 20 FPS

  lastUpdate = now;

  switch (currentEffect) {
    case EFFECT_RAINBOW:
      updateRainbow();
      break;
    case EFFECT_SOLID:
      updateSolid();
      break;
    case EFFECT_CHASE:
      updateChase();
      break;
    case EFFECT_POV_TEST:
      updatePOVTest();
      break;
  }

  FastLED.show();
}

// ============================================================================
// SERVIDOR WEB
// ============================================================================

void setupWebServer() {
  // Página principal
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>POV-Line Wokwi Test</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      max-width: 600px;
      margin: 50px auto;
      padding: 20px;
      background: #1a1a1a;
      color: #fff;
    }
    h1 { color: #4CAF50; }
    button {
      background: #4CAF50;
      color: white;
      border: none;
      padding: 15px 30px;
      margin: 10px 5px;
      border-radius: 5px;
      cursor: pointer;
      font-size: 16px;
    }
    button:hover { background: #45a049; }
    input[type="range"] {
      width: 100%;
      margin: 10px 0;
    }
    .color-btn {
      width: 60px;
      height: 60px;
      margin: 5px;
      border: 2px solid #fff;
      display: inline-block;
    }
    .status {
      background: #333;
      padding: 15px;
      border-radius: 5px;
      margin: 20px 0;
    }
  </style>
</head>
<body>
  <h1>🎨 POV-Line Wokwi Test</h1>

  <div class="status">
    <p><strong>Status:</strong> Connected to Wokwi Simulator</p>
    <p><strong>LEDs:</strong> 144 APA102</p>
    <p><strong>Effect:</strong> <span id="currentEffect">Rainbow</span></p>
    <p><strong>Brightness:</strong> <span id="currentBrightness">128</span></p>
  </div>

  <h2>Effects</h2>
  <button onclick="setEffect('rainbow')">🌈 Rainbow</button>
  <button onclick="setEffect('solid')">⚫ Solid Color</button>
  <button onclick="setEffect('chase')">🔴 Chase</button>
  <button onclick="setEffect('pov')">📊 POV Test</button>

  <h2>Brightness</h2>
  <input type="range" min="10" max="255" value="128" oninput="setBrightness(this.value)">

  <h2>Solid Color</h2>
  <div>
    <button class="color-btn" style="background: red;" onclick="setColor(255,0,0)"></button>
    <button class="color-btn" style="background: green;" onclick="setColor(0,255,0)"></button>
    <button class="color-btn" style="background: blue;" onclick="setColor(0,0,255)"></button>
    <button class="color-btn" style="background: yellow;" onclick="setColor(255,255,0)"></button>
    <button class="color-btn" style="background: cyan;" onclick="setColor(0,255,255)"></button>
    <button class="color-btn" style="background: magenta;" onclick="setColor(255,0,255)"></button>
    <button class="color-btn" style="background: white;" onclick="setColor(255,255,255)"></button>
  </div>

  <script>
    function setEffect(effect) {
      fetch('/effect?mode=' + effect)
        .then(r => r.text())
        .then(d => {
          document.getElementById('currentEffect').textContent = d;
        });
    }

    function setBrightness(val) {
      fetch('/brightness?value=' + val)
        .then(() => {
          document.getElementById('currentBrightness').textContent = val;
        });
    }

    function setColor(r, g, b) {
      fetch('/color?r=' + r + '&g=' + g + '&b=' + b);
    }
  </script>
</body>
</html>
)rawliteral";
    request->send(200, "text/html", html);
  });

  // Endpoint: cambiar efecto
  server.on("/effect", HTTP_GET, [](AsyncWebServerRequest *request){
    String effectName = "Rainbow";

    if (request->hasParam("mode")) {
      String mode = request->getParam("mode")->value();

      if (mode == "rainbow") {
        currentEffect = EFFECT_RAINBOW;
        effectName = "Rainbow";
      } else if (mode == "solid") {
        currentEffect = EFFECT_SOLID;
        effectName = "Solid Color";
      } else if (mode == "chase") {
        currentEffect = EFFECT_CHASE;
        effectName = "Chase";
      } else if (mode == "pov") {
        currentEffect = EFFECT_POV_TEST;
        effectName = "POV Test";
      }

      effectCounter = 0;
    }

    request->send(200, "text/plain", effectName);
  });

  // Endpoint: cambiar brillo
  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      brightness = request->getParam("value")->value().toInt();
      FastLED.setBrightness(brightness);
    }
    request->send(200, "text/plain", "OK");
  });

  // Endpoint: cambiar color sólido
  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("r") && request->hasParam("g") && request->hasParam("b")) {
      uint8_t r = request->getParam("r")->value().toInt();
      uint8_t g = request->getParam("g")->value().toInt();
      uint8_t b = request->getParam("b")->value().toInt();
      solidColor = CRGB(r, g, b);
    }
    request->send(200, "text/plain", "OK");
  });

  // Endpoint: status
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{";
    json += "\"effect\":\"" + String(currentEffect) + "\",";
    json += "\"brightness\":" + String(brightness) + ",";
    json += "\"color\":{\"r\":" + String(solidColor.r) + ",\"g\":" + String(solidColor.g) + ",\"b\":" + String(solidColor.b) + "}";
    json += "}";
    request->send(200, "application/json", json);
  });

  server.begin();
  Serial.println("Web server started");
}

// ============================================================================
// SETUP Y LOOP
// ============================================================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n=================================");
  Serial.println("POV-Line Wokwi Test");
  Serial.println("=================================\n");

  // 1. Inicializar LEDs
  Serial.print("Initializing LEDs... ");
  FastLED.addLeds<APA102, LED_DATA_PIN, LED_CLOCK_PIN, BGR>(leds, NUM_LEDS);
  FastLED.setBrightness(brightness);
  FastLED.clear();
  FastLED.show();
  Serial.println("OK");

  // 2. Inicializar LittleFS
  Serial.print("Initializing LittleFS... ");
  if (!LittleFS.begin(true)) {
    Serial.println("FAILED");
    Serial.println("WARNING: LittleFS not available in simulation");
  } else {
    Serial.println("OK");
    Serial.printf("Total space: %d bytes\n", LittleFS.totalBytes());
    Serial.printf("Used space: %d bytes\n", LittleFS.usedBytes());
  }

  // 3. Conectar WiFi
  Serial.print("Connecting to WiFi... ");
  WiFi.begin(ssid, password);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" OK");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println("Access web interface at: http://localhost (via Wokwi port forwarding)");
  } else {
    Serial.println(" FAILED");
    Serial.println("WARNING: WiFi not connected");
  }

  // 4. Inicializar servidor web
  Serial.print("Starting web server... ");
  setupWebServer();
  Serial.println("OK");

  // 5. Test inicial de LEDs
  Serial.println("\nRunning LED test...");

  // Rojo
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(500);

  // Verde
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(500);

  // Azul
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(500);

  // Clear
  FastLED.clear();
  FastLED.show();

  Serial.println("LED test complete");

  Serial.println("\n=================================");
  Serial.println("Setup complete!");
  Serial.println("Starting main loop...");
  Serial.println("=================================\n");
}

void loop() {
  // Actualizar efecto LED
  updateEffect();

  // Pequeño delay para evitar watchdog
  delay(1);
}
