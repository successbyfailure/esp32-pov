#include "accelerometer.h"

Accelerometer::Accelerometer() {
#ifdef HAS_ACCELEROMETER
  lis = nullptr;
  initialized = false;
  isMoving = false;
  motionThreshold = 0.35; // m/s² - umbral de movimiento (más alto para evitar falsos positivos)
  lastX = lastY = lastZ = 0;
  dirAccumX = 0;
  motionLevel = 0;
  offsetX = offsetY = offsetZ = 0;
  calibrated = false;
  lastMotionTime = 0;
#endif
}

Accelerometer::~Accelerometer() {
#ifdef HAS_ACCELEROMETER
  if (lis != nullptr) {
    delete lis;
  }
#endif
}

bool Accelerometer::init() {
#ifdef HAS_ACCELEROMETER
  Serial.println("Inicializando acelerómetro LIS3DH...");

  // Inicializar I2C
  Wire.begin(ACCEL_SDA_PIN, ACCEL_SCL_PIN);

  // Crear instancia del sensor
  lis = new Adafruit_LIS3DH();

  // Algunos badges usan 0x18 u 0x19, intentar ambos
  bool found = lis->begin(0x18);
  if (!found) {
    found = lis->begin(0x19);
  }

  if (!found) {
    Serial.println("Error: No se encontró el LIS3DH (direcciones 0x18/0x19)");
    delete lis;
    lis = nullptr;
    return false;
  }

  // Librería no expone getAddress; solo indicar que fue detectado
  Serial.println("LIS3DH detectado");

  // Configurar rango de medición (±2g, ±4g, ±8g, ±16g)
  lis->setRange(LIS3DH_RANGE_2_G);

  // Configurar data rate
  lis->setDataRate(LIS3DH_DATARATE_100_HZ);

  Serial.println("LIS3DH inicializado correctamente");
  initialized = true;

  // Calibrar automáticamente al inicio
  delay(100);
  calibrate();

  return true;
#else
  Serial.println("Acelerómetro no soportado en esta configuración");
  return false;
#endif
}

void Accelerometer::update() {
#ifdef HAS_ACCELEROMETER
  if (!initialized) return;

  sensors_event_t event;
  lis->getEvent(&event);

  // Aplicar calibración
  float x = event.acceleration.x - offsetX;
  float y = event.acceleration.y - offsetY;
  float z = event.acceleration.z - offsetZ;

  // Calcular cambio desde última lectura
  float deltaX = abs(x - lastX);
  float deltaY = abs(y - lastY);
  float deltaZ = abs(z - lastZ);

  // Acumular dirección en eje X (badge se mueve lateralmente)
  float signedDeltaX = x - lastX;
  dirAccumX = (dirAccumX * 0.8f) + (signedDeltaX * 0.2f);  // suavizado exponencial

  float totalDelta = sqrt(deltaX*deltaX + deltaY*deltaY + deltaZ*deltaZ);
  motionLevel = totalDelta;

  // Detectar movimiento
  if (totalDelta > motionThreshold) {
    isMoving = true;
    lastMotionTime = millis();
  } else {
    // Si no hay movimiento por más de 500ms, considerar quieto
    if (millis() - lastMotionTime > 500) {
      isMoving = false;
    }
  }

  // Guardar última lectura
  lastX = x;
  lastY = y;
  lastZ = z;
#endif
}

bool Accelerometer::detectMotion() {
#ifdef HAS_ACCELEROMETER
  return isMoving;
#else
  return false;
#endif
}

bool Accelerometer::isStill() {
#ifdef HAS_ACCELEROMETER
  return !isMoving;
#else
  return true;
#endif
}

float Accelerometer::getMotionMagnitude() {
#ifdef HAS_ACCELEROMETER
  if (!initialized) return 0;
  return motionLevel;
#else
  return 0;
#endif
}

int8_t Accelerometer::getSweepDirection() {
#ifdef HAS_ACCELEROMETER
  if (!initialized) return 0;

  const float dirThreshold = 0.1f;  // m/s² equivalente tras suavizado
  if (dirAccumX > dirThreshold) return 1;   // Movimiento positivo (ej. derecha)
  if (dirAccumX < -dirThreshold) return -1; // Movimiento negativo (ej. izquierda)
  return 0;
#else
  return 0;
#endif
}

void Accelerometer::calibrate() {
#ifdef HAS_ACCELEROMETER
  if (!initialized) return;

  Serial.println("Calibrando acelerómetro... mantener quieto");

  // Tomar múltiples lecturas
  const int samples = 50;
  float sumX = 0, sumY = 0, sumZ = 0;

  for (int i = 0; i < samples; i++) {
    sensors_event_t event;
    lis->getEvent(&event);

    sumX += event.acceleration.x;
    sumY += event.acceleration.y;
    sumZ += event.acceleration.z;

    delay(20);
  }

  // Calcular promedio
  offsetX = sumX / samples;
  offsetY = sumY / samples;

  // Z debería ser ~9.8 m/s² (gravedad)
  // Solo compensar el error, no la gravedad completa
  float expectedZ = 9.8;
  offsetZ = (sumZ / samples) - expectedZ;

  calibrated = true;

  Serial.printf("Calibración completa: X=%.2f, Y=%.2f, Z=%.2f\n",
                offsetX, offsetY, offsetZ);
#endif
}

bool Accelerometer::isCalibrated() {
#ifdef HAS_ACCELEROMETER
  return calibrated;
#else
  return false;
#endif
}

void Accelerometer::getRawData(float &x, float &y, float &z) {
#ifdef HAS_ACCELEROMETER
  if (!initialized) {
    x = y = z = 0;
    return;
  }

  x = lastX;
  y = lastY;
  z = lastZ;
#else
  x = y = z = 0;
#endif
}

bool Accelerometer::isInitialized() {
#ifdef HAS_ACCELEROMETER
  return initialized;
#else
  return false;
#endif
}

// Instancia global
Accelerometer accelerometer;
