#ifndef IMAGE_PARSER_H
#define IMAGE_PARSER_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <FastLED.h>
#include "config.h"

// Estructura de header BMP
#pragma pack(push, 1)
struct BMPHeader {
  uint16_t signature;
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t dataOffset;
};

struct BMPInfoHeader {
  uint32_t headerSize;
  int32_t width;
  int32_t height;
  uint16_t planes;
  uint16_t bitsPerPixel;
  uint32_t compression;
  uint32_t imageSize;
  int32_t xPixelsPerMeter;
  int32_t yPixelsPerMeter;
  uint32_t colorsUsed;
  uint32_t colorsImportant;
};
#pragma pack(pop)

// Header personalizado para RGB565
#pragma pack(push, 1)
struct RGB565Header {
  char magic[4];  // "R565"
  uint16_t width;
  uint16_t height;
};
#pragma pack(pop)

class ImageParser {
public:
  ImageParser();

  bool parseImageInfo(const char* filename, ImageInfo& info);
  bool parseBMP(File& file, ImageInfo& info);
  bool parseRGB565(File& file, ImageInfo& info);

  bool getColumn(const char* filename, uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize);
  bool getColumnBMP(File& file, const ImageInfo& info, uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize);
  bool getColumnRGB565(File& file, const ImageInfo& info, uint16_t columnIndex, CRGB* buffer, uint16_t bufferSize);

private:
  bool readBMPHeader(File& file, BMPHeader& header, BMPInfoHeader& infoHeader);
  void rgb565ToRGB(uint16_t rgb565, uint8_t& r, uint8_t& g, uint8_t& b);
};

extern ImageParser imageParser;

#endif
