#include "wifi_manager.h"

WiFiManager::WiFiManager() : apMode(false), connected(false), lastConnectionAttempt(0) {
}

void WiFiManager::init() {
  WiFi.mode(WIFI_MODE_NULL);
  Serial.println("WiFi Manager inicializado");
}

void WiFiManager::loop() {
  if (!apMode && !connected) {
    checkConnection();
  }
}

bool WiFiManager::startAP(const char* ssid, const char* password) {
  Serial.printf("Iniciando AP: %s\n", ssid);

  WiFi.mode(WIFI_AP);
  bool success = WiFi.softAP(ssid, password);

  if (success) {
    apMode = true;
    connected = false;
    Serial.printf("AP iniciado. IP: %s\n", WiFi.softAPIP().toString().c_str());
    return true;
  } else {
    Serial.println("Error: No se pudo iniciar AP");
    return false;
  }
}

bool WiFiManager::connectWiFi(const char* ssid, const char* password) {
  Serial.printf("Conectando a WiFi: %s\n", ssid);

  // Detener AP si está activo
  if (apMode) {
    WiFi.softAPdisconnect(true);
    apMode = false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  currentSSID = String(ssid);

  // Esperar conexión con timeout
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_CONNECT_TIMEOUT) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    Serial.printf("\nConectado a WiFi. IP: %s\n", WiFi.localIP().toString().c_str());
    return true;
  } else {
    connected = false;
    Serial.println("\nError: No se pudo conectar a WiFi");
    return false;
  }
}

void WiFiManager::disconnect() {
  if (apMode) {
    WiFi.softAPdisconnect(true);
    apMode = false;
  } else {
    WiFi.disconnect(true);
    connected = false;
  }

  WiFi.mode(WIFI_OFF);
  Serial.println("WiFi desconectado");
}

bool WiFiManager::isConnected() {
  if (!apMode) {
    connected = (WiFi.status() == WL_CONNECTED);
  }
  return connected;
}

bool WiFiManager::isAPMode() {
  return apMode;
}

String WiFiManager::getIP() {
  if (apMode) {
    return WiFi.softAPIP().toString();
  } else if (connected) {
    return WiFi.localIP().toString();
  }
  return "0.0.0.0";
}

String WiFiManager::getSSID() {
  if (apMode) {
    return String(AP_SSID);
  } else {
    return currentSSID;
  }
}

int WiFiManager::getRSSI() {
  if (connected && !apMode) {
    return WiFi.RSSI();
  }
  return 0;
}

void WiFiManager::checkConnection() {
  unsigned long currentTime = millis();

  if (currentTime - lastConnectionAttempt < WIFI_RETRY_INTERVAL) {
    return;
  }

  if (WiFi.status() != WL_CONNECTED && currentSSID.length() > 0) {
    Serial.println("Reintentando conexión WiFi...");
    WiFi.reconnect();
    lastConnectionAttempt = currentTime;
  }
}

// Instancia global
WiFiManager wifiManager;
