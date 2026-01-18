#ifndef POV_ENGINE_H
#define POV_ENGINE_H

#include <Arduino.h>
#include <FastLED.h>
#include "config.h"
#include "led_controller.h"
#include "image_parser.h"

class POVEngine {
private:
  char currentImageFile[64];
  ImageInfo currentImage;
  uint16_t currentColumn;
  uint16_t speed;  // FPS de columnas/filas
  unsigned long lastUpdate;
  unsigned long columnDelay;
  // Medición de FPS real
  uint16_t framesThisSecond;
  uint16_t measuredFps;
  unsigned long lastFpsTick;
  bool loopMode;
  POVOrientation orientation;
  bool reverseDirection;
  bool playing;
  bool paused;
  bool imageLoaded;
  CRGB* columnBuffer;

public:
  POVEngine();
  ~POVEngine();

  bool loadImage(const char* filename);
  void unloadImage();
  bool isImageLoaded();

  void play();
  void pause();
  void stop();
  void resume();

  bool isPlaying();
  bool isPaused();

  void setSpeed(uint16_t fps);
  uint16_t getSpeed();
  uint16_t getMeasuredFps();

  void setLoopMode(bool loop);
  bool getLoopMode();

  void setOrientation(POVOrientation orient);
  POVOrientation getOrientation();
  void setReverseDirection(bool reverse);
  bool isReverse();

  void update();

  const char* getCurrentImageName();
  uint16_t getCurrentColumn();
  uint16_t getTotalColumns();

private:
  void updateColumnDelay();
  void displayColumn(uint16_t column);
};

extern POVEngine povEngine;

#endif
