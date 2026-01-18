#include "ha_integration.h"
#include "led_controller.h"
#include "pov_engine.h"
#include "effects.h"

extern Config config;

HAIntegration::HAIntegration() : mqttClient(nullptr), initialized(false), connected(false), lastReconnectAttempt(0) {
  sprintf(mqttClientId, "pov_line_%08X", (uint32_t)ESP.getEfuseMac());
}

HAIntegration::~HAIntegration() {
  if (mqttClient != nullptr) {
    delete mqttClient;
  }
}

void HAIntegration::init(const char* broker, uint16_t port, const char* user, const char* password) {
  if (mqttClient != nullptr) {
    delete mqttClient;
  }

  mqttClient = new PubSubClient(wifiClient);

  if (mqttClient == nullptr) {
    Serial.println("Error: No se pudo crear cliente MQTT");
    return;
  }

  mqttClient->setServer(broker, port);
  mqttClient->setCallback([this](char* topic, byte* payload, unsigned int length) {
    this->callback(topic, payload, length);
  });

  initialized = true;
  Serial.printf("MQTT inicializado: %s:%d\n", broker, port);
}

void HAIntegration::loop() {
  if (!initialized) {
    return;
  }

  if (!mqttClient->connected()) {
    unsigned long now = millis();
    if (now - lastReconnectAttempt > MQTT_RECONNECT_DELAY) {
      lastReconnectAttempt = now;
      connect();
    }
  } else {
    mqttClient->loop();
  }
}

bool HAIntegration::isConnected() {
  return connected && mqttClient != nullptr && mqttClient->connected();
}

void HAIntegration::connect() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  Serial.printf("Conectando a MQTT como %s...\n", mqttClientId);

  if (mqttClient->connect(mqttClientId)) {
    connected = true;
    Serial.println("Conectado a MQTT");

    // Suscribirse a topics de comandos
    mqttClient->subscribe(getCommandTopic().c_str());
    mqttClient->subscribe(getBrightnessCommandTopic().c_str());
    mqttClient->subscribe(getEffectCommandTopic().c_str());

    // Publicar discovery
    publishDiscovery();

    // Publicar estado inicial
    publishState();
  } else {
    connected = false;
    Serial.printf("Error conectando a MQTT, rc=%d\n", mqttClient->state());
  }
}

void HAIntegration::callback(char* topic, byte* payload, unsigned int length) {
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';

  Serial.printf("MQTT mensaje en %s: %s\n", topic, message);

  String topicStr = String(topic);

  if (topicStr == getCommandTopic()) {
    handleCommand(message);
  } else if (topicStr == getBrightnessCommandTopic()) {
    handleBrightnessCommand(message);
  } else if (topicStr == getEffectCommandTopic()) {
    handleEffectCommand(message);
  }

  // Publicar estado actualizado
  publishState();
}

void HAIntegration::handleCommand(const char* payload) {
  String command = String(payload);
  command.toUpperCase();

  if (command == "ON") {
    // Iniciar efecto rainbow por defecto
    effects.rainbow(10);
  } else if (command == "OFF") {
    povEngine.stop();
    effects.stop();
  }
}

void HAIntegration::handleBrightnessCommand(const char* payload) {
  uint8_t brightness = atoi(payload);
  ledController.setBrightness(brightness);
  config.brightness = brightness;
}

void HAIntegration::handleEffectCommand(const char* payload) {
  String effect = String(payload);
  effect.toLowerCase();

  povEngine.stop();

  if (effect == "pov") {
    // Iniciar POV si hay imagen cargada
    if (povEngine.isImageLoaded()) {
      povEngine.play();
    }
  } else if (effect == "rainbow") {
    effects.rainbow(10);
  } else if (effect == "solid color") {
    effects.solidColor(255, 255, 255);
  } else if (effect == "chase") {
    effects.colorChase(CRGB(255, 0, 0), 50);
  }
}

void HAIntegration::publishState() {
  if (!isConnected()) {
    return;
  }

  // Estado principal
  String state = (povEngine.isPlaying() || effects.isRunning()) ? "ON" : "OFF";
  mqttClient->publish(getStateTopic().c_str(), state.c_str(), true);

  // Brillo
  char brightnessStr[8];
  sprintf(brightnessStr, "%d", ledController.getBrightness());
  mqttClient->publish(getBrightnessStateTopic().c_str(), brightnessStr, true);

  // Efecto activo
  String currentEffect = "POV";
  if (povEngine.isPlaying()) {
    currentEffect = "POV";
  } else if (effects.isRunning()) {
    switch (effects.getCurrentEffect()) {
      case EFFECT_RAINBOW:
        currentEffect = "Rainbow";
        break;
      case EFFECT_SOLID_COLOR:
        currentEffect = "Solid Color";
        break;
      case EFFECT_COLOR_CHASE:
        currentEffect = "Chase";
        break;
      default:
        currentEffect = "None";
    }
  } else {
    currentEffect = "None";
  }

  mqttClient->publish(getEffectStateTopic().c_str(), currentEffect.c_str(), true);
}

void HAIntegration::publishDiscovery() {
  if (!isConnected()) {
    return;
  }

  JsonDocument doc;

  doc["name"] = config.deviceName;
  doc["unique_id"] = mqttClientId;
  doc["state_topic"] = getStateTopic();
  doc["command_topic"] = getCommandTopic();
  doc["brightness_state_topic"] = getBrightnessStateTopic();
  doc["brightness_command_topic"] = getBrightnessCommandTopic();
  doc["brightness_scale"] = 255;
  doc["effect_state_topic"] = getEffectStateTopic();
  doc["effect_command_topic"] = getEffectCommandTopic();

  JsonArray effectList = doc["effect_list"].to<JsonArray>();
  effectList.add("POV");
  effectList.add("Rainbow");
  effectList.add("Solid Color");
  effectList.add("Chase");

  JsonObject device = doc["device"].to<JsonObject>();
  device["identifiers"][0] = mqttClientId;
  device["name"] = config.deviceName;
  device["model"] = "POV Line";
  device["manufacturer"] = "Custom";
  device["sw_version"] = FIRMWARE_VERSION;

  String payload;
  serializeJson(doc, payload);

  mqttClient->publish(getDiscoveryTopic().c_str(), payload.c_str(), true);

  Serial.println("Discovery publicado en Home Assistant");
}

String HAIntegration::getStateTopic() {
  return String(MQTT_BASE_TOPIC) + "/state";
}

String HAIntegration::getCommandTopic() {
  return String(MQTT_BASE_TOPIC) + "/command";
}

String HAIntegration::getBrightnessStateTopic() {
  return String(MQTT_BASE_TOPIC) + "/brightness/state";
}

String HAIntegration::getBrightnessCommandTopic() {
  return String(MQTT_BASE_TOPIC) + "/brightness/set";
}

String HAIntegration::getEffectStateTopic() {
  return String(MQTT_BASE_TOPIC) + "/effect/state";
}

String HAIntegration::getEffectCommandTopic() {
  return String(MQTT_BASE_TOPIC) + "/effect/set";
}

String HAIntegration::getDiscoveryTopic() {
  return String(HA_DISCOVERY_PREFIX) + "/light/" + String(mqttClientId) + "/config";
}

// Instancia global
HAIntegration haIntegration;
