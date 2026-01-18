#include "effects.h"

Effects::Effects() : currentEffect(EFFECT_NONE), lastUpdate(0), effectSpeed(10),
                     effectColor(CRGB::Black), effectState(0), running(false) {
}

void Effects::update() {
  if (!running || !ledController.isInitialized()) {
    return;
  }

  switch (currentEffect) {
    case EFFECT_RAINBOW:
      updateRainbow();
      break;
    case EFFECT_COLOR_CHASE:
      updateColorChase();
      break;
    case EFFECT_FADE:
      updateFade();
      break;
    case EFFECT_SOLID_COLOR:
      // No requiere actualización continua
      break;
    default:
      break;
  }
}

void Effects::stop() {
  running = false;
  currentEffect = EFFECT_NONE;
  ledController.clear();
  ledController.show();
}

bool Effects::isRunning() {
  return running;
}

EffectType Effects::getCurrentEffect() {
  return currentEffect;
}

void Effects::rainbow(uint8_t speed) {
  currentEffect = EFFECT_RAINBOW;
  effectSpeed = speed;
  effectState = 0;
  running = true;
  lastUpdate = millis();
}

void Effects::solidColor(CRGB color) {
  currentEffect = EFFECT_SOLID_COLOR;
  effectColor = color;
  running = true;

  ledController.fill(color);
  ledController.show();
}

void Effects::solidColor(uint8_t r, uint8_t g, uint8_t b) {
  solidColor(CRGB(r, g, b));
}

void Effects::colorChase(CRGB color, uint8_t speed) {
  currentEffect = EFFECT_COLOR_CHASE;
  effectColor = color;
  effectSpeed = speed;
  effectState = 0;
  running = true;
  lastUpdate = millis();
}

void Effects::fade(CRGB fromColor, CRGB toColor, uint16_t duration) {
  // Simplificado: implementación básica de fade
  currentEffect = EFFECT_FADE;
  effectColor = fromColor;
  running = true;

  // Transición simple
  ledController.fill(fromColor);
  ledController.show();
  delay(duration / 2);
  ledController.fill(toColor);
  ledController.show();
  delay(duration / 2);
}

void Effects::updateRainbow() {
  unsigned long currentTime = millis();
  if (currentTime - lastUpdate < (100 / effectSpeed)) {
    return;
  }

  CRGB* pixels = ledController.getPixels();
  uint16_t numLeds = ledController.getNumLeds();

  for (uint16_t i = 0; i < numLeds; i++) {
    pixels[i] = CHSV((effectState + (i * 256 / numLeds)) % 256, 255, 255);
  }

  ledController.show();
  effectState = (effectState + 1) % 256;
  lastUpdate = currentTime;
}

void Effects::updateColorChase() {
  unsigned long currentTime = millis();
  if (currentTime - lastUpdate < effectSpeed) {
    return;
  }

  ledController.clear();

  // Crear efecto de persecución con 3 LEDs
  CRGB* pixels = ledController.getPixels();
  uint16_t numLeds = ledController.getNumLeds();

  for (int i = 0; i < 3; i++) {
    uint16_t pos = (effectState + i) % numLeds;
    pixels[pos] = effectColor;
  }

  ledController.show();
  effectState = (effectState + 1) % numLeds;
  lastUpdate = currentTime;
}

void Effects::updateFade() {
  // Fade ya se ejecuta de una vez en la función fade()
  // Esta función está para compatibilidad
}

// Instancia global
Effects effects;
