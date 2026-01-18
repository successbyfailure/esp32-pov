#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>
#include "config.h"

#ifdef BUTTON_PIN

class Button {
private:
  uint8_t pin;
  bool lastState;
  bool currentState;
  unsigned long lastDebounceTime;
  unsigned long debounceDelay;

  // Para detectar pulsaciones largas
  unsigned long pressStartTime;
  bool longPressTriggered;
  unsigned long longPressThreshold;

public:
  Button();

  void init(uint8_t buttonPin = BUTTON_PIN);
  void update();

  // Estados del botón
  bool isPressed();
  bool wasPressed();    // Se resetea después de leer
  bool wasReleased();   // Se resetea después de leer
  bool isLongPress();   // Detecta pulsación larga

  // Configuración
  void setDebounceDelay(unsigned long ms);
  void setLongPressThreshold(unsigned long ms);

private:
  bool pressedFlag;
  bool releasedFlag;
};

extern Button button;

#endif // BUTTON_PIN

#endif
