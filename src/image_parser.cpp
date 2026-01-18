#include "image_parser.h"

ImageParser::ImageParser() {
}

bool ImageParser::parseImageInfo(const char* filename, ImageInfo& info) {
  File file = LittleFS.open(filename, "r");
  if (!file) {
    Serial.printf("Error: No se pudo abrir archivo %s\n", filename);
    info.valid = false;
    return false;
  }

  strncpy(info.filename, filename, sizeof(info.filename) - 1);
  info.fileSize = file.size();

  bool result = false;

  // Detectar formato por extensión
  String fn = String(filename);
  fn.toLowerCase();

  if (fn.endsWith(".bmp")) {
    result = parseBMP(file, info);
    info.format = 0;
  } else if (fn.endsWith(".rgb") || fn.endsWith(".565")) {
    result = parseRGB565(file, info);
    info.format = 1;
  } else {
    Serial.println("Error: Formato de archivo no soportado");
    info.valid = false;
  }

  file.close();
  return result;
}

bool ImageParser::parseBMP(File& file, ImageInfo& info) {
  BMPHeader header;
  BMPInfoHeader infoHeader;

  if (!readBMPHeader(file, header, infoHeader)) {
    info.valid = false;
    return false;
  }

  // Validar formato
  if (infoHeader.compression != 0) {
    Serial.println("Error: Solo se soportan BMP sin compresión");
    info.valid = false;
    return false;
  }

  if (infoHeader.bitsPerPixel != 24) {
    Serial.println("Error: Solo se soportan BMP de 24 bits");
    info.valid = false;
    return false;
  }

  info.width = abs(infoHeader.width);
  info.height = abs(infoHeader.height);
  info.valid = true;

  Serial.printf("BMP parseado: %dx%d\n", info.width, info.height);

  return true;
}

bool ImageParser::parseRGB565(File& file, ImageInfo& info) {
  RGB565Header header;

  file.seek(0);
  if (file.read((uint8_t*)&header, sizeof(RGB565Header)) != sizeof(RGB565Header)) {
    Serial.println("Error: No se pudo leer header RGB565");
    info.valid = false;
    return false;
  }

  // Validar magic number
  if (strncmp(header.magic, "R565", 4) != 0) {
    Serial.println("Error: Header RGB565 inválido");
    info.valid = false;
    return false;
  }

  info.width = header.width;
  info.height = header.height;
  info.valid = true;

  Serial.printf("RGB565 parseado: %dx%d\n", info.width, info.height);

  return true;
}

bool ImageParser::getColumn(const char* filename, uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize) {
  ImageInfo info;
  if (!parseImageInfo(filename, info)) {
    return false;
  }

  if (columnIndex >= info.width) {
    return false;
  }

  File file = LittleFS.open(filename, "r");
  if (!file) {
    return false;
  }

  bool result = false;

  if (info.format == 0) {
    result = getColumnBMP(file, info, columnIndex, buffer, bufferSize);
  } else if (info.format == 1) {
    result = getColumnRGB565(file, info, columnIndex, buffer, bufferSize);
  }

  file.close();
  return result;
}

bool ImageParser::getColumnBMP(File& file, const ImageInfo& info, uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize) {
  BMPHeader header;
  BMPInfoHeader infoHeader;

  if (!readBMPHeader(file, header, infoHeader)) {
    return false;
  }

  uint16_t height = min((uint16_t)info.height, bufferSize);

  // Calcular padding de fila (las filas BMP están alineadas a 4 bytes)
  uint16_t rowSize = ((infoHeader.bitsPerPixel * info.width + 31) / 32) * 4;

  // BMP se almacena de abajo hacia arriba
  for (uint16_t y = 0; y < height; y++) {
    // Calcular posición en el archivo
    uint16_t bmpY = height - 1 - y;  // Invertir Y
    uint32_t rowOffset = header.dataOffset + (bmpY * rowSize);
    uint32_t pixelOffset = rowOffset + (columnIndex * 3);

    file.seek(pixelOffset);

    uint8_t pixel[3];
    if (file.read(pixel, 3) != 3) {
      return false;
    }

    // BMP almacena en formato BGR
    buffer[y] = CRGB(pixel[2], pixel[1], pixel[0]);
  }

  return true;
}

bool ImageParser::getColumnRGB565(File& file, const ImageInfo& info, uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize) {
  uint16_t height = min((uint16_t)info.height, bufferSize);

  // Saltar header
  file.seek(sizeof(RGB565Header));

  // Leer columna
  for (uint16_t y = 0; y < height; y++) {
    // Calcular posición en el archivo
    uint32_t pixelOffset = sizeof(RGB565Header) + ((y * info.width + columnIndex) * 2);

    file.seek(pixelOffset);

    uint16_t rgb565;
    if (file.read((uint8_t*)&rgb565, 2) != 2) {
      return false;
    }

    uint8_t r, g, b;
    rgb565ToRGB(rgb565, r, g, b);
    buffer[y] = CRGB(r, g, b);
  }

  return true;
}

bool ImageParser::readBMPHeader(File& file, BMPHeader& header, BMPInfoHeader& infoHeader) {
  file.seek(0);

  if (file.read((uint8_t*)&header, sizeof(BMPHeader)) != sizeof(BMPHeader)) {
    Serial.println("Error: No se pudo leer header BMP");
    return false;
  }

  if (header.signature != 0x4D42) {  // "BM"
    Serial.println("Error: Firma BMP inválida");
    return false;
  }

  if (file.read((uint8_t*)&infoHeader, sizeof(BMPInfoHeader)) != sizeof(BMPInfoHeader)) {
    Serial.println("Error: No se pudo leer info header BMP");
    return false;
  }

  return true;
}

void ImageParser::rgb565ToRGB(uint16_t rgb565, uint8_t& r, uint8_t& g, uint8_t& b) {
  r = ((rgb565 >> 11) & 0x1F) * 255 / 31;
  g = ((rgb565 >> 5) & 0x3F) * 255 / 63;
  b = (rgb565 & 0x1F) * 255 / 31;
}

// Instancia global
ImageParser imageParser;
