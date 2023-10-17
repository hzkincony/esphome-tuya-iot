#pragma once

#include "esphome.h"

#include "esphome/core/component.h"
#include <mqtt_client.h>

#define TAG "tuya.iot"


namespace esphome {
  namespace tuya_iot {

    using mqtt_callback_t = std::function<void(const std::string &, const std::string &)>;
    using mqtt_json_callback_t = std::function<void(const std::string &, JsonObject)>;

    struct MQTTSubscription {
      std::string topic;
      uint8_t qos;
      mqtt_callback_t callback;
      bool subscribed;
      uint32_t resubscribe_timeout;
    };

    struct MQTTMessage {
      std::string topic;
      std::string payload;
      uint8_t qos;  ///< QoS. Only for last will testaments.
      bool retain;
    };

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
      TuyaIotComponent();

      void setup() override;
      void set_product_id(const char* product_id) { product_id_ = product_id; };
      void set_device_id(const char* device_id) { device_id_ = device_id; };
      const char* get_device_id() { return device_id_; };
      void set_device_secret(const char* device_secret) { device_secret_ = device_secret; };
      void set_region_domain(const char* region_domain) { region_domain_ = region_domain; };
      void set_time(homeassistant::HomeassistantTime* time) {time_ = time;};
      void dump_config() override;
      void update() override;
      void subscribe(const std::string &topic, mqtt_callback_t callback, uint8_t qos = 0);
      void subscribe_json(const std::string &topic, const mqtt_json_callback_t &callback, uint8_t qos = 0);
      bool publish(const MQTTMessage &message);
      bool publish(const std::string &topic, const char *payload, size_t payload_length, uint8_t qos = 0, bool retain = false);
      bool publish(const std::string &topic, const std::string &payload, uint8_t qos = 0, bool retain = false);
      bool publish_json(const std::string &topic, const json::json_build_t &f, uint8_t qos = 0, bool retain = false);
      bool property_report_string(const std::string &value, uint8_t qos = 0, bool retain = false);
      bool property_report_json(const json::json_build_t &f, uint8_t qos = 0, bool retain = false);
      bool property_report(const std::string &key, const float &value);
      bool property_report(const std::string &key, const std::string &value);
      bool property_report(const std::string &key, const bool &value);

      std::string gen_msg_id();

    protected:
      const char* product_id_;
      const char* device_id_;
      const char* device_secret_;
      const char* region_domain_;
      homeassistant::HomeassistantTime* time_;
      esp_mqtt_client_config_t mqtt_cfg_{};
      esp_mqtt_client_handle_t client_;
      bool tuya_inited_ = false;
      bool tuya_connected_ = false;
      std::string payload_buffer_;

      void mqtt_event_handler_(const Event &event);
      static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
      bool subscribe_(const char *topic, uint8_t qos);
      void resubscribe_subscription_(MQTTSubscription *sub);
      void resubscribe_subscriptions_();
      std::vector<MQTTSubscription> subscriptions_;

    };

    extern TuyaIotComponent *global_tuya_iot;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)


    class TuyaIotMessageTrigger : public Trigger<std::string>, public Component {
    public:
      explicit TuyaIotMessageTrigger(std::string topic);

      void set_qos(uint8_t qos);
      void set_payload(const std::string &payload);
      void setup() override;
      void dump_config() override;
      float get_setup_priority() const override;

    protected:
      std::string topic_;
      uint8_t qos_{0};
      optional<std::string> payload_;
    };

    class TuyaIotJsonMessageTrigger : public Trigger<JsonObjectConst> {
    public:
      explicit TuyaIotJsonMessageTrigger(const std::string &topic, uint8_t qos) {
        global_tuya_iot->subscribe_json(
                                        topic, [this](const std::string &topic, JsonObject root) { this->trigger(root); }, qos);
      }
    };

    class TuyaIotEventTrigger : public Trigger<JsonObjectConst> {
    public:
      explicit TuyaIotEventTrigger(const char* device_id, const std::string &event_name) {
        std::string topic = std::string("tylink/") + std::string(device_id) + std::string("/thing/") + event_name;
        global_tuya_iot->subscribe_json(
                                        topic, [this](const std::string &topic, JsonObject root) { this->trigger(root); }, 0);
      }
    };



  }
}
