#ifndef EFFECTS_H
#define EFFECTS_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "led_controller.h"

class Effects {
private:
  EffectType currentEffect;
  unsigned long lastUpdate;
  uint8_t effectSpeed;
  CRGB effectColor;
  uint8_t effectState;
  bool running;

public:
  Effects();

  void update();
  void stop();
  bool isRunning();
  EffectType getCurrentEffect();

  // Efectos disponibles
  void rainbow(uint8_t speed = 10);
  void solidColor(CRGB color);
  void solidColor(uint8_t r, uint8_t g, uint8_t b);
  void colorChase(CRGB color, uint8_t speed = 50);
  void fade(CRGB fromColor, CRGB toColor, uint16_t duration = 2000);

private:
  void updateRainbow();
  void updateColorChase();
  void updateFade();
};

extern Effects effects;

#endif
