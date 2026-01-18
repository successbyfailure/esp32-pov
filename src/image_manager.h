#ifndef IMAGE_MANAGER_H
#define IMAGE_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <FS.h>
#include <LittleFS.h>
#include "config.h"
#include "image_parser.h"

class ImageManager {
private:
  std::vector<ImageInfo> imageList;
  bool listLoaded;

public:
  ImageManager();

  bool init();
  std::vector<ImageInfo> listImages();
  bool deleteImage(const char* filename);
  bool getImageInfo(const char* filename, ImageInfo& info);
  size_t getFreeSpace();
  size_t getTotalSpace();
  size_t getUsedSpace();
  bool imageExists(const char* filename);
  void refreshList();

private:
  void loadImageList();
  bool isImageFile(const char* filename);
};

extern ImageManager imageManager;

#endif
