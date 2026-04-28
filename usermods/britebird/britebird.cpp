#include "wled.h"

#ifndef WLED_DISABLE_MQTT

/*
 * Britebird usermod: publishes the /json/info object as JSON payload to
 * <mqttDeviceTopic>/info on every MQTT connect and then periodically.
 */
class BritebirdUsermod : public Usermod {
 private:
  bool          publishInfoPending     = false;
  // unsigned long lastInfoPublish        = 0;
  bool          initDone               = false;

  // static constexpr uint32_t INFO_PUBLISH_INTERVAL_MS = 5UL * 60UL * 1000UL; // 5 min

  void publishInfo() {
    if (!WLED_MQTT_CONNECTED) return;
    if (!requestJSONBufferLock(USERMOD_ID_BRITEBIRD)) return;

    pDoc->clear();
    serializeInfo(pDoc->to<JsonObject>());

    char topic[MQTT_MAX_TOPIC_LEN + 16];
    snprintf_P(topic, sizeof(topic) - 1, PSTR("%s/info"), mqttDeviceTopic);

    String payload;
    serializeJson(*pDoc, payload);
    mqtt->publish(topic, 0, false, payload.c_str());

    releaseJSONBufferLock();
    // lastInfoPublish = millis();
  }

 public:
  void setup() override {
    // lastInfoPublish = millis(); // avoid immediate publish before MQTT connects
    initDone = true;
  }

  void loop() override {
    if (!initDone || !WLED_MQTT_CONNECTED) return;

    if (publishInfoPending) {
      publishInfoPending = false;
      publishInfo();
    }
    // else if (millis() - lastInfoPublish > INFO_PUBLISH_INTERVAL_MS) {
    //   publishInfo();
    // }
  }

  void onMqttConnect(bool sessionPresent) override {
    publishInfoPending = true;
  }

  void addToJsonInfo(JsonObject& root) override {
    JsonObject user = root["u"];
    if (user.isNull()) user = root.createNestedObject("u");
    JsonArray info = user.createNestedArray(F("Britebird"));
    info.add(F("active"));
  }

  uint16_t getId() override {
    return USERMOD_ID_BRITEBIRD;
  }
};

BritebirdUsermod britebird_usermod;
REGISTER_USERMOD(britebird_usermod);

#endif  // WLED_DISABLE_MQTT
