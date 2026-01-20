#include "pov_engine.h"

POVEngine::POVEngine() : currentColumn(0), speed(DEFAULT_POV_SPEED), lastUpdate(0),
                         columnDelay(0), framesThisSecond(0), measuredFps(0), lastFpsTick(0),
                         loopMode(DEFAULT_LOOP_MODE), orientation(DEFAULT_POV_ORIENTATION), reverseDirection(false),
                         playing(false), paused(false), imageLoaded(false), columnBuffer(nullptr) {
  currentImageFile[0] = '\0';
  updateColumnDelay();
}

POVEngine::~POVEngine() {
  if (columnBuffer != nullptr) {
    delete[] columnBuffer;
  }
}

// Normaliza el nombre recibido evitando prefijos absolutos como /images/
static String normalizeImageName(const char* name) {
  String fname = String(name);
  if (fname.startsWith(IMAGES_DIR "/")) {
    fname = fname.substring(strlen(IMAGES_DIR) + 1);
  } else if (fname.startsWith("/")) {
    fname = fname.substring(1);
  }
  return fname;
}

bool POVEngine::loadImage(const char* filename) {
  // Construir path completo
  String cleanName = normalizeImageName(filename);
  String fullPath = String(IMAGES_DIR) + "/" + cleanName;

  // Parsear información de la imagen
  if (!imageParser.parseImageInfo(fullPath.c_str(), currentImage)) {
    Serial.printf("Error: No se pudo cargar imagen %s\n", filename);
    imageLoaded = false;
    return false;
  }

  // Validar dimensiones
  if (currentImage.height > ledController.getNumLeds()) {
    Serial.printf("Error: Imagen muy alta (%d LEDs configurados, imagen tiene %d)\n",
                  ledController.getNumLeds(), currentImage.height);
    imageLoaded = false;
    return false;
  }

  // Asignar buffer de columna
  if (columnBuffer != nullptr) {
    delete[] columnBuffer;
  }

  columnBuffer = new CRGB[MAX_LEDS];
  if (columnBuffer == nullptr) {
    Serial.println("Error: No se pudo asignar memoria para buffer de columna");
    imageLoaded = false;
    return false;
  }

  strncpy(currentImageFile, fullPath.c_str(), sizeof(currentImageFile) - 1);
  currentColumn = 0;
  imageLoaded = true;

  Serial.printf("Imagen cargada: %s (%dx%d)\n", filename, currentImage.width, currentImage.height);

  return true;
}

void POVEngine::unloadImage() {
  imageLoaded = false;
  playing = false;
  paused = false;
  currentColumn = 0;
  currentImageFile[0] = '\0';

  if (columnBuffer != nullptr) {
    delete[] columnBuffer;
    columnBuffer = nullptr;
  }

  ledController.clear();
  ledController.show();
}

bool POVEngine::isImageLoaded() {
  return imageLoaded;
}

void POVEngine::play() {
  if (!imageLoaded) {
    Serial.println("Error: No hay imagen cargada");
    return;
  }

  playing = true;
  paused = false;
  currentColumn = 0;
  lastUpdate = millis();

  Serial.println("POV iniciado");
}

void POVEngine::pause() {
  if (playing) {
    paused = true;
    Serial.println("POV pausado");
  }
}

void POVEngine::stop() {
  playing = false;
  paused = false;
  currentColumn = 0;

  ledController.clear();
  ledController.show();

  Serial.println("POV detenido");
}

void POVEngine::resume() {
  if (paused && imageLoaded) {
    paused = false;
    lastUpdate = millis();
    Serial.println("POV reanudado");
  }
}

bool POVEngine::isPlaying() {
  return playing && !paused;
}

bool POVEngine::isPaused() {
  return paused;
}

void POVEngine::setSpeed(uint16_t fps) {
  speed = constrain(fps, MIN_POV_SPEED, MAX_POV_SPEED);
  updateColumnDelay();
  Serial.printf("Velocidad POV: %d FPS\n", speed);
}

uint16_t POVEngine::getSpeed() {
  return speed;
}

uint16_t POVEngine::getMeasuredFps() {
  return measuredFps;
}

void POVEngine::setLoopMode(bool loop) {
  loopMode = loop;
  Serial.printf("Modo loop: %s\n", loop ? "ON" : "OFF");
}

bool POVEngine::getLoopMode() {
  return loopMode;
}

void POVEngine::setOrientation(POVOrientation orient) {
  orientation = orient;
  Serial.printf("Orientación POV: %s\n", orient == POV_VERTICAL ? "VERTICAL" : "HORIZONTAL");
}

void POVEngine::setReverseDirection(bool reverse) {
  if (reverseDirection != reverse) {
    reverseDirection = reverse;
    currentColumn = 0;  // reiniciar barrido al cambiar de dirección
    Serial.printf("Dirección POV: %s\n", reverse ? "RIGHT->LEFT" : "LEFT->RIGHT");
  }
}

bool POVEngine::isReverse() {
  return reverseDirection;
}

POVOrientation POVEngine::getOrientation() {
  return orientation;
}

void POVEngine::update() {
  if (!playing || paused || !imageLoaded) {
    return;
  }

  unsigned long currentTime = millis();
  if (currentTime - lastUpdate < columnDelay) {
    return;
  }

  // Mostrar columna actual
  displayColumn(currentColumn);

  // Avanzar a la siguiente columna/fila
  currentColumn++;

  // Medir FPS real (cuenta de columnas mostradas por segundo)
  framesThisSecond++;
  if (currentTime - lastFpsTick >= 1000) {
    measuredFps = framesThisSecond;
    framesThisSecond = 0;
    lastFpsTick = currentTime;
  }

  // Verificar fin de imagen (depende de la orientación)
  uint16_t maxColumns = (orientation == POV_VERTICAL) ? currentImage.width : currentImage.height;
  if (currentColumn >= maxColumns) {
    if (loopMode) {
      currentColumn = 0;
    } else {
      stop();
      Serial.println("POV finalizado");
      return;
    }
  }

  lastUpdate = currentTime;
}

const char* POVEngine::getCurrentImageName() {
  return imageLoaded ? currentImageFile : "";
}

uint16_t POVEngine::getCurrentColumn() {
  return currentColumn;
}

uint16_t POVEngine::getTotalColumns() {
  if (!imageLoaded) return 0;
  return (orientation == POV_VERTICAL) ? currentImage.width : currentImage.height;
}

void POVEngine::updateColumnDelay() {
  if (speed > 0) {
    columnDelay = 1000 / speed;
  } else {
    columnDelay = 1000 / DEFAULT_POV_SPEED;
  }
}

void POVEngine::displayColumn(uint16_t column) {
  if (!imageLoaded || columnBuffer == nullptr) {
    return;
  }

  uint16_t numLeds = ledController.getNumLeds();
  uint16_t maxColumns = (orientation == POV_VERTICAL) ? currentImage.width : currentImage.height;
  uint16_t displayCol = column;
  if (reverseDirection && column < maxColumns) {
    displayCol = maxColumns - 1 - column;
  }

  // Abrir archivo una sola vez para toda la columna/fila
  File file = LittleFS.open(currentImageFile, "r");
  if (!file) {
    Serial.printf("Error: No se pudo abrir %s para visualización\n", currentImageFile);
    return;
  }

  if (orientation == POV_VERTICAL) {
    // Modo vertical: leer columna vertical de la imagen
    if (!imageParser.getColumn(file, currentImage, displayCol, columnBuffer, MAX_LEDS)) {
      Serial.printf("Error: No se pudo leer columna %d\n", displayCol);
      file.close();
      return;
    }

    // Mostrar en LEDs verticalmente, escalando si hay más LEDs que alto de imagen
    for (uint16_t i = 0; i < numLeds; i++) {
      uint16_t srcIndex;
      if (currentImage.height <= 1) {
        srcIndex = 0;
      } else if (numLeds <= currentImage.height) {
        srcIndex = i;  // cabe sin escalar
      } else {
        // Escalado lineal: mapear 0..numLeds-1 a 0..height-1
        srcIndex = (uint32_t)i * (currentImage.height - 1) / (numLeds - 1);
      }
      srcIndex = min(srcIndex, (uint16_t)(MAX_LEDS - 1));
      ledController.setPixel(i, columnBuffer[srcIndex]);
    }
  } else {
    // Modo horizontal: leer fila horizontal de la imagen
    // La "columna" actual es realmente el índice de fila (Y)
    uint16_t rowIndex = column;
    uint16_t displayWidth = numLeds;

    // Buffer temporal
    CRGB tempBuffer[MAX_LEDS];

    // Leer píxeles de la fila horizontal, escalando si es necesario
    for (uint16_t x = 0; x < displayWidth; x++) {
      // Para leer una fila horizontal, leemos el píxel en (x, rowIndex)
      // Necesitamos leer la columna X y tomar el píxel en la posición rowIndex
      
      // Calcular columna origen (posible escalado si hay más LEDs que ancho)
      uint16_t srcX;
      if (currentImage.width <= 1) {
        srcX = 0;
      } else if (displayWidth <= currentImage.width) {
        srcX = x;
      } else {
        srcX = (uint32_t)x * (currentImage.width - 1) / (displayWidth - 1);
      }

      // Usamos la versión optimizada que recibe el archivo abierto
      if (!imageParser.getColumn(file, currentImage, srcX, tempBuffer, MAX_LEDS)) {
        ledController.setPixel(x, CRGB::Black);
        continue;
      }

      // Tomar el píxel en la fila rowIndex
      if (rowIndex < currentImage.height) {
        ledController.setPixel(x, tempBuffer[rowIndex]);
      } else {
        ledController.setPixel(x, CRGB::Black);
      }
    }

    // Apagar LEDs restantes
    for (uint16_t i = displayWidth; i < numLeds; i++) {
      ledController.setPixel(i, CRGB::Black);
    }
  }

  file.close();
  ledController.show();
}

// Instancia global
POVEngine povEngine;
