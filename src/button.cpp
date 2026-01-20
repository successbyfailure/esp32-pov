#include "button.h"

#if defined(BUTTON_PIN) || defined(UP_BUTTON_PIN) || defined(DOWN_BUTTON_PIN)

Button::Button() {
  pin = BUTTON_PIN;
  lastState = HIGH;
  currentState = HIGH;
  lastDebounceTime = 0;
  debounceDelay = 50;
  pressStartTime = 0;
  longPressTriggered = false;
  longPressThreshold = 1000;  // 1 segundo
  pressedFlag = false;
  releasedFlag = false;
}

void Button::init(uint8_t buttonPin) {
  pin = buttonPin;
  pinMode(pin, INPUT_PULLUP);  // Asumiendo botón activo bajo
  currentState = digitalRead(pin);
  lastState = currentState;

  Serial.printf("Botón inicializado en GPIO%d\n", pin);
}

void Button::update() {
  bool reading = digitalRead(pin);

  // Debouncing
  if (reading != lastState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // Si el estado cambió después del debounce
    if (reading != currentState) {
      currentState = reading;

      // Botón presionado (activo bajo, HIGH→LOW)
      if (currentState == LOW) {
        pressedFlag = true;
        pressStartTime = millis();
        longPressTriggered = false;
      }
      // Botón liberado (LOW→HIGH)
      else {
        releasedFlag = true;
        longPressTriggered = false;
      }
    }
  }

  // Detectar pulsación larga
  if (currentState == LOW && !longPressTriggered) {
    if (millis() - pressStartTime >= longPressThreshold) {
      longPressTriggered = true;
    }
  }

  lastState = reading;
}

bool Button::isPressed() {
  return (currentState == LOW);
}

bool Button::wasPressed() {
  if (pressedFlag) {
    pressedFlag = false;
    return true;
  }
  return false;
}

bool Button::wasReleased() {
  if (releasedFlag) {
    releasedFlag = false;
    return true;
  }
  return false;
}

bool Button::isLongPress() {
  return longPressTriggered;
}

void Button::setDebounceDelay(unsigned long ms) {
  debounceDelay = ms;
}

void Button::setLongPressThreshold(unsigned long ms) {
  longPressThreshold = ms;
}

// Instancia global
#ifdef BUTTON_PIN
Button button;
#endif

#ifdef UP_BUTTON_PIN
Button buttonUp;
#endif

#ifdef DOWN_BUTTON_PIN
Button buttonDown;
#endif

#endif // any button pin
