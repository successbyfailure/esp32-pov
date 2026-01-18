#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"

class LEDController {
private:
  CRGB* leds;
  uint16_t numLeds;
  uint8_t brightness;
  LEDStripType ledType;
  bool initialized;

public:
  LEDController();
  ~LEDController();

  bool init(uint16_t num = DEFAULT_NUM_LEDS, LEDStripType type = DEFAULT_LED_TYPE);
  void setNumLeds(uint16_t num);
  void setLEDType(LEDStripType type);
  LEDStripType getLEDType();
  void setPixel(uint16_t index, CRGB color);
  void setPixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
  void setBrightness(uint8_t value);
  uint8_t getBrightness();
  void clear();
  void fill(CRGB color);
  void show();
  CRGB* getPixels();
  uint16_t getNumLeds();
  bool isInitialized();
};

extern LEDController ledController;

#endif
