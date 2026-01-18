#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <Arduino.h>

#ifdef HAS_ACCELEROMETER
#include <Wire.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#endif

class Accelerometer {
private:
#ifdef HAS_ACCELEROMETER
  Adafruit_LIS3DH* lis;
  bool initialized;

  // Detección de movimiento
  float lastX, lastY, lastZ;
  unsigned long lastMotionTime;
  bool isMoving;
  float motionThreshold;
  float dirAccumX;

  // Calibración
  float offsetX, offsetY, offsetZ;
  bool calibrated;
#endif

public:
  Accelerometer();
  ~Accelerometer();

  bool init();
  void update();

  // Detección de movimiento
  bool detectMotion();
  bool isStill();
  float getMotionMagnitude();
  int8_t getSweepDirection();  // -1: negativo, 1: positivo, 0: indeterminado

  // Calibración
  void calibrate();
  bool isCalibrated();

  // Datos crudos
  void getRawData(float &x, float &y, float &z);

  // Estado
  bool isInitialized();
};

extern Accelerometer accelerometer;

#endif
