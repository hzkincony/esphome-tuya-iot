#pragma once

#include "esphome.h"

#include "esphome/core/component.h"
#include <mqtt_client.h>

#define TAG "tuya.iot"


namespace esphome {
namespace tuya_iot_component {

struct Event {
  esp_mqtt_event_id_t event_id;
  std::vector<char> data;
  int total_data_len;
  int current_data_offset;
  std::string topic;
  int msg_id;
  bool retain;
  int qos;
  bool dup;
  bool session_present;
  esp_mqtt_error_codes_t error_handle;

  // Construct from esp_mqtt_event_t
  // Any pointer values that are unsafe to keep are converted to safe copies
  Event(const esp_mqtt_event_t &event)
      : event_id(event.event_id),
        data(event.data, event.data + event.data_len),
        total_data_len(event.total_data_len),
        current_data_offset(event.current_data_offset),
        topic(event.topic, event.topic_len),
        msg_id(event.msg_id),
        retain(event.retain),
        qos(event.qos),
        dup(event.dup),
        session_present(event.session_present),
        error_handle(*event.error_handle) {}
};

class TuyaIotComponent : public PollingComponent {
    public:
    TuyaIotComponent() : PollingComponent(5000) {};

    void setup() override;
    void set_product_id(const char* product_id) { product_id_ = product_id; };
    void set_device_id(const char* device_id) { device_id_ = device_id; };
    void set_device_secret(const char* device_secret) { device_secret_ = device_secret; };
    void set_time(homeassistant::HomeassistantTime* time) {time_ = time;};
    void dump_config() override;
    void update() override;
    

    protected:
    const char* product_id_;
    const char* device_id_;
    const char* device_secret_;
    homeassistant::HomeassistantTime* time_;
    esp_mqtt_client_config_t mqtt_cfg_{};
    esp_mqtt_client_handle_t client_;
    bool tuya_inited_ = false;
    bool tuya_connected_ = false;

    void mqtt_event_handler_(const Event &event);
    static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
};



}
}