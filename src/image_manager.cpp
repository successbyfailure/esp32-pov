#include "image_manager.h"

ImageManager::ImageManager() : listLoaded(false) {
}

// Normaliza el nombre de archivo para que no lleve prefijo /images/ repetido
static String normalizeFilename(const char* name) {
  String fname = String(name);
  // Quitar prefijo absoluto
  if (fname.startsWith(IMAGES_DIR "/")) {
    fname = fname.substring(strlen(IMAGES_DIR) + 1);
  } else if (fname.startsWith("/")) {
    // Quitar solo la primera /
    fname = fname.substring(1);
  }
  return fname;
}

bool ImageManager::init() {
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
  if (!LittleFS.begin()) {
#else
  if (!LittleFS.begin(true)) {
#endif
    Serial.println("Error: No se pudo inicializar LittleFS");
    return false;
  }

  Serial.println("LittleFS inicializado correctamente");

  // Crear directorio de imágenes si no existe
  if (!LittleFS.exists(IMAGES_DIR)) {
    LittleFS.mkdir(IMAGES_DIR);
    Serial.println("Directorio de imágenes creado");
  }

  loadImageList();
  return true;
}

std::vector<ImageInfo> ImageManager::listImages() {
  if (!listLoaded) {
    loadImageList();
  }
  return imageList;
}

bool ImageManager::deleteImage(const char* filename) {
  String cleanName = normalizeFilename(filename);
  String fullPath = String(IMAGES_DIR) + "/" + cleanName;

  if (!LittleFS.exists(fullPath)) {
    Serial.printf("Error: Imagen %s no existe\n", filename);
    return false;
  }

  if (LittleFS.remove(fullPath)) {
    Serial.printf("Imagen %s eliminada\n", filename);
    refreshList();
    return true;
  }

  Serial.printf("Error: No se pudo eliminar %s\n", filename);
  return false;
}

bool ImageManager::getImageInfo(const char* filename, ImageInfo& info) {
  String cleanName = normalizeFilename(filename);
  String fullPath = String(IMAGES_DIR) + "/" + cleanName;

  if (!LittleFS.exists(fullPath)) {
    Serial.printf("Error: Imagen %s no existe\n", filename);
    info.valid = false;
    return false;
  }

  return imageParser.parseImageInfo(fullPath.c_str(), info);
}

size_t ImageManager::getFreeSpace() {
  size_t total = 0;
  size_t used = 0;
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
  FSInfo info;
  if (LittleFS.info(info)) {
    total = info.totalBytes;
    used = info.usedBytes;
  }
#else
  total = LittleFS.totalBytes();
  used = LittleFS.usedBytes();
#endif
  return (total > used) ? (total - used) : 0;
}

size_t ImageManager::getTotalSpace() {
  size_t total = 0;
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
  FSInfo info;
  if (LittleFS.info(info)) {
    total = info.totalBytes;
  }
#else
  total = LittleFS.totalBytes();
#endif
  return total;
}

size_t ImageManager::getUsedSpace() {
  size_t used = 0;
#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
  FSInfo info;
  if (LittleFS.info(info)) {
    used = info.usedBytes;
  }
#else
  used = LittleFS.usedBytes();
#endif
  return used;
}

bool ImageManager::imageExists(const char* filename) {
  String cleanName = normalizeFilename(filename);
  String fullPath = String(IMAGES_DIR) + "/" + cleanName;
  return LittleFS.exists(fullPath);
}

void ImageManager::refreshList() {
  loadImageList();
}

void ImageManager::loadImageList() {
  imageList.clear();

#if defined(ESP8266) || defined(ARDUINO_ARCH_ESP8266)
  Dir dir = LittleFS.openDir(IMAGES_DIR);
  bool ok = true;
  while (dir.next()) {
    String name = dir.fileName();
    if (isImageFile(name.c_str())) {
      ImageInfo info;
      String cleanName = normalizeFilename(name.c_str());
      String fullPath = String(IMAGES_DIR) + "/" + cleanName;
      if (imageParser.parseImageInfo(fullPath.c_str(), info)) {
        imageList.push_back(info);
      }
    }
  }
  listLoaded = ok;
  if (!ok) {
    Serial.println("Error: No se pudo leer directorio de imágenes");
  }
#else
  File root = LittleFS.open(IMAGES_DIR);
  if (!root || !root.isDirectory()) {
    Serial.println("Error: No se pudo abrir directorio de imágenes");
    listLoaded = false;
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory() && isImageFile(file.name())) {
      ImageInfo info;
      String cleanName = normalizeFilename(file.name());
      String fullPath = String(IMAGES_DIR) + "/" + cleanName;

      if (imageParser.parseImageInfo(fullPath.c_str(), info)) {
        imageList.push_back(info);
      }
    }
    file = root.openNextFile();
  }

  listLoaded = true;
  Serial.printf("Imágenes cargadas: %d\n", imageList.size());
#endif
}

bool ImageManager::isImageFile(const char* filename) {
  String fn = String(filename);
  fn.toLowerCase();
  return fn.endsWith(".bmp") || fn.endsWith(".rgb") || fn.endsWith(".565");
}

// Instancia global
ImageManager imageManager;
