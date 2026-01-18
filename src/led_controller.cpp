#include "led_controller.h"

LEDController::LEDController() : leds(nullptr), numLeds(0), brightness(DEFAULT_BRIGHTNESS), ledType(DEFAULT_LED_TYPE), initialized(false) {
}

LEDController::~LEDController() {
  if (leds != nullptr) {
    delete[] leds;
  }
}

bool LEDController::init(uint16_t num, LEDStripType type) {
  if (num == 0 || num > MAX_LEDS) {
    Serial.println("Error: Número de LEDs inválido");
    return false;
  }

  numLeds = num;
  ledType = type;

  // Liberar memoria anterior si existe
  if (leds != nullptr) {
    delete[] leds;
  }

  // Asignar memoria para los LEDs
  leds = new CRGB[numLeds];
  if (leds == nullptr) {
    Serial.println("Error: No se pudo asignar memoria para LEDs");
    initialized = false;
    return false;
  }

  // Inicializar FastLED según el tipo de tira LED
  switch (ledType) {
    case LED_TYPE_WS2811:
    case LED_TYPE_WS2812:
    case LED_TYPE_WS2812B:
      // WS281x usa solo 1 pin (DATA)
      FastLED.addLeds<WS2811, LED_DATA_PIN, GRB>(leds, numLeds);
      Serial.printf("LEDs inicializados: %d x WS281x en pin DATA=%d\n", numLeds, LED_DATA_PIN);
      break;

    case LED_TYPE_APA102:
      // APA102 usa 2 pines (DATA + CLOCK)
      FastLED.addLeds<APA102, LED_DATA_PIN, LED_CLOCK_PIN, BGR>(leds, numLeds);
      Serial.printf("LEDs inicializados: %d x APA102 en pines DATA=%d, CLOCK=%d\n",
                    numLeds, LED_DATA_PIN, LED_CLOCK_PIN);
      break;

    default:
      Serial.println("Error: Tipo de LED no soportado");
      delete[] leds;
      leds = nullptr;
      initialized = false;
      return false;
  }

  FastLED.setBrightness(brightness);
  FastLED.clear();
  FastLED.show();

  initialized = true;
  return true;
}

void LEDController::setNumLeds(uint16_t num) {
  if (num != numLeds && num > 0 && num <= MAX_LEDS) {
    init(num, ledType);
  }
}

void LEDController::setLEDType(LEDStripType type) {
  if (type != ledType) {
    init(numLeds, type);
  }
}

LEDStripType LEDController::getLEDType() {
  return ledType;
}

void LEDController::setPixel(uint16_t index, CRGB color) {
  if (initialized && index < numLeds) {
    leds[index] = color;
  }
}

void LEDController::setPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
  if (initialized && index < numLeds) {
    leds[index] = CRGB(r, g, b);
  }
}

void LEDController::setBrightness(uint8_t value) {
  brightness = constrain(value, 0, MAX_BRIGHTNESS);
  if (initialized) {
    FastLED.setBrightness(brightness);
  }
}

uint8_t LEDController::getBrightness() {
  return brightness;
}

void LEDController::clear() {
  if (initialized) {
    FastLED.clear();
  }
}

void LEDController::fill(CRGB color) {
  if (initialized) {
    fill_solid(leds, numLeds, color);
  }
}

void LEDController::show() {
  if (initialized) {
    FastLED.show();
  }
}

CRGB* LEDController::getPixels() {
  return leds;
}

uint16_t LEDController::getNumLeds() {
  return numLeds;
}

bool LEDController::isInitialized() {
  return initialized;
}

// Instancia global
LEDController ledController;
