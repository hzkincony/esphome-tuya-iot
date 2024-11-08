#include "tuya_iot_component.h"
#include "esphome/components/network/util.h"
#include "sha/sha256.h"
#include <string>

namespace esphome {
  namespace tuya_iot {
    const char tuya_cacert_pem[] = {\
      "-----BEGIN CERTIFICATE-----\n"\
      "MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\n"\
      "EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\n"\
      "EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\n"\
      "ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\n"\
      "NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\n"\
      "EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\n"\
      "AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\n"\
      "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\n"\
      "E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\n"\
      "/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\n"\
      "DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\n"\
      "GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\n"\
      "tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\n"\
      "AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\n"\
      "FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\n"\
      "WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\n"\
      "9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\n"\
      "gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\n"\
      "2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\n"\
      "LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\n"\
      "4uJEvlz36hz1\n"\
      "-----END CERTIFICATE-----\n"};

    TuyaIotComponent *global_tuya_iot = nullptr;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

    TuyaIotComponent::TuyaIotComponent() : PollingComponent(5000) {
      global_tuya_iot = this;
    }

    void TuyaIotComponent::setup() {
      ESP_LOGD(TAG, "TuyaIotComponent::setup");

    }

    void TuyaIotComponent::mqtt_event_handler_(const Event &event) {
      ESP_LOGD(TAG, "Event dispatched from event loop event_id=%d", event.event_id);
      switch (event.event_id) {
      case MQTT_EVENT_BEFORE_CONNECT:
        ESP_LOGD(TAG, "MQTT_EVENT_BEFORE_CONNECT");
        break;

      case MQTT_EVENT_CONNECTED:
        ESP_LOGD(TAG, "MQTT_EVENT_CONNECTED");
        this->tuya_connected_ = true;

        this->resubscribe_subscriptions_();

        // static char action_execute_topic[200];
        // sprintf(action_execute_topic, "tylink/%s/thing/action/execute", device_id_);
        // esp_mqtt_client_subscribe(client_, action_execute_topic, 0);

        // static char property_set_topic[200];
        // sprintf(property_set_topic, "tylink/%s/thing/property/set", device_id_);
        // esp_mqtt_client_subscribe(client_, property_set_topic, 0);

        break;
      case MQTT_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "MQTT_EVENT_DISCONNECTED");
        // TODO is there a way to get the disconnect reason?
        this->tuya_connected_ = false;
        break;
      case MQTT_EVENT_ERROR:
        ESP_LOGD(TAG, "MQTT_EVENT_ERROR");
        if (event.error_handle.error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
          ESP_LOGD(TAG, "Last error code reported from esp-tls: 0x%x", event.error_handle.esp_tls_last_esp_err);
          ESP_LOGD(TAG, "Last tls stack error number: 0x%x", event.error_handle.esp_tls_stack_err);
          ESP_LOGD(TAG, "Last captured errno : %d (%s)", event.error_handle.esp_transport_sock_errno,
                   strerror(event.error_handle.esp_transport_sock_errno));
        } else if (event.error_handle.error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED) {
          ESP_LOGD(TAG, "Connection refused error: 0x%x", event.error_handle.connect_return_code);
        } else {
          ESP_LOGD(TAG, "Unknown error type: 0x%x", event.error_handle.error_type);
        }
        this->tuya_connected_ = false;
        break;
      case MQTT_EVENT_DATA:
        static std::string topic;
        if (event.topic.length() > 0) {
          topic = event.topic;
        }
        ESP_LOGD(TAG, "MQTT_EVENT_DATA topic=%s", topic.c_str());

        if (event.current_data_offset == 0) {
          this->payload_buffer_.reserve(event.total_data_len);
        }

        this->payload_buffer_.append(event.data.data(), event.data.size());

        if (event.data.size() + event.current_data_offset == event.total_data_len) {
          ESP_LOGD(TAG, "MQTT_EVENT_DATA data=%s", this->payload_buffer_.c_str());

          for (auto &subscription : this->subscriptions_) {
            if (topic.compare(subscription.topic) == 0) {
              // ESP_LOGD(TAG, "equal, topic=%s, sub=%s", topic.c_str(), subscription.topic.c_str());
              subscription.callback(topic, std::string(this->payload_buffer_));
            } else {
              // ESP_LOGD(TAG, "not equal, topic=%s, sub=%s", topic.c_str(), subscription.topic.c_str());
            }
          }

          this->payload_buffer_.clear();
        }
      default:
        ESP_LOGD(TAG, "Other event id:%d", event.event_id);
        break;
      }
    }

    void TuyaIotComponent::mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
      TuyaIotComponent *instance = static_cast<TuyaIotComponent *>(handler_args);
      if (instance) {
        auto event = *static_cast<esp_mqtt_event_t *>(event_data);
        instance->mqtt_event_handler_(Event(event));
      }
    }

    void TuyaIotComponent::dump_config() {

    }

    void TuyaIotComponent::resubscribe_subscriptions_() {
      for (auto &subscription : this->subscriptions_) {
        this->resubscribe_subscription_(&subscription);
      }
    }

    void TuyaIotComponent::resubscribe_subscription_(MQTTSubscription *sub) {
      if (sub->subscribed)
        return;

      sub->subscribed = this->subscribe_(sub->topic.c_str(), sub->qos);
    }

    // Subscribe
    bool TuyaIotComponent::subscribe_(const char *topic, uint8_t qos) {
      if (!this->tuya_connected_)
        return false;

      esp_mqtt_client_subscribe(client_, topic, qos);
      ESP_LOGD(TAG, "subscribe(topic='%s')", topic);
      return true;
    }

    void TuyaIotComponent::subscribe(const std::string &topic, mqtt_callback_t callback, uint8_t qos) {
      MQTTSubscription subscription{
        .topic = topic,
        .qos = qos,
        .callback = std::move(callback),
        .subscribed = false,
        .resubscribe_timeout = 0,
      };
      this->resubscribe_subscription_(&subscription);
      this->subscriptions_.push_back(subscription);
    }

    void TuyaIotComponent::subscribe_json(const std::string &topic, const mqtt_json_callback_t &callback, uint8_t qos) {
      auto f = [callback](const std::string &topic, const std::string &payload) {
        json::parse_json(payload, [topic, callback](JsonObject root) -> bool { callback(topic, root); return true; });
      };
      MQTTSubscription subscription{
        .topic = topic,
        .qos = qos,
        .callback = f,
        .subscribed = false,
        .resubscribe_timeout = 0,
      };
      this->resubscribe_subscription_(&subscription);
      this->subscriptions_.push_back(subscription);
    }

    bool TuyaIotComponent::publish(const std::string &topic, const std::string &payload, uint8_t qos, bool retain) {
      return this->publish(topic, payload.data(), payload.size(), qos, retain);
    }

    bool TuyaIotComponent::publish(const std::string &topic, const char *payload, size_t payload_length, uint8_t qos,
                                   bool retain) {
      return publish({.topic = topic, .payload = payload, .qos = qos, .retain = retain});
    }

    bool TuyaIotComponent::publish(const MQTTMessage &message) {
      if (!this->tuya_connected_) {
        // critical components will re-transmit their messages
        return false;
      }
      bool ret = esp_mqtt_client_publish(client_, message.topic.c_str(), message.payload.c_str(), message.payload.size(), message.qos, message.retain) != -1;
      if (ret) {
        ESP_LOGD(TAG, "publich succ, topic=%s, payload=%s", message.topic.c_str(), message.payload.c_str());
      } else {
        ESP_LOGD(TAG, "publich fail, topic=%s, payload=%s", message.topic.c_str(), message.payload.c_str());
      }
      delay(0);
      return ret;
    }

    bool TuyaIotComponent::publish_json(const std::string &topic, const json::json_build_t &f, uint8_t qos, bool retain) {
      std::string message = json::build_json(f);
      return this->publish(topic, message, qos, retain);
    }

    bool TuyaIotComponent::property_report_string(const std::string &value, uint8_t qos, bool retain) {
      std::string topic = std::string("tylink/") + std::string(this->device_id_) + std::string("/thing/property/report");
      return this->publish(topic, value, qos, retain);
    }

    bool TuyaIotComponent::property_report_json(const json::json_build_t &f, uint8_t qos, bool retain) {
      std::string message = json::build_json(f);
      return this->property_report_string(message, qos, retain);
    }

    bool  TuyaIotComponent::property_report(const std::string &key, const float &value) {
      auto msgId = gen_msg_id();
      auto now = time_->now().timestamp;
      json::json_build_t f = [=](JsonObject root) {
        root["msgId"] = msgId;
        root["time"] = now;
        root["data"][key] = value;
      };

      return this->property_report_json(f, 0, false);
    }

    bool  TuyaIotComponent::property_report(const std::string &key, const std::string &value) {
      auto msgId = gen_msg_id();
      auto now = time_->now().timestamp;
      json::json_build_t f = [=](JsonObject root) {
        root["msgId"] = msgId;
        root["time"] = now;
        root["data"][key] = value;
      };
      return this->property_report_json(f, 0, false);
    }

    bool  TuyaIotComponent::property_report(const std::string &key, const bool &value) {
      auto msgId = gen_msg_id();
      auto now = time_->now().timestamp;
      json::json_build_t f = [=](JsonObject root) {
        root["msgId"] = msgId;
        root["time"] = now;
        root["data"][key] = value;
      };
      return this->property_report_json(f, 0, false);
    }

    std::string TuyaIotComponent::gen_msg_id() {
      static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
      std::string tmp_s;
      tmp_s.reserve(32);

      for (int i = 0; i < 32; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
      }

      return tmp_s;
    }

    void TuyaIotComponent::update() {
      int32_t now = time_->now().timestamp;
      ESP_LOGD(TAG, "TuyaIotComponent::update time=%d", now);

      if (!network::is_connected()) {
        ESP_LOGD(TAG, "TuyaIotComponent::update network unconnect");
        return;
      }

      if (tuya_inited_ == false) {
        char content[200];
        sprintf(content, "deviceId=%s,timestamp=%d,secureMode=1,accessType=1", device_id_, now);
        ESP_LOGD(TAG, "content: %s", content);
        Sha256.initHmac((uint8_t * ) device_secret_, 16);
        Sha256.print(content);
        uint8_t * result = Sha256.resultHmac();
        std::string password("");
        for (int i = 0; i < 32; i++) {
          password = password.append(1, "0123456789abcdef"[result[i] >> 4]);
          password = password.append(1, "0123456789abcdef"[result[i] & 0xf]);
        }

        char username[200];
        sprintf(username, "%s|signMethod=hmacSha256,timestamp=%d,secureMode=1,accessType=1", device_id_, now);
        mqtt_cfg_.username = username;
        ESP_LOGD(TAG, "username: %s", mqtt_cfg_.username);

        char* password_str = new char[254];
        password_str[password.copy(password_str, password.size(), 0)] = '\0';
        mqtt_cfg_.password = password_str;
        ESP_LOGD(TAG, "password: %s", mqtt_cfg_.password);

        static char uri[50];
        sprintf(uri, "mqtts://%s:8883", region_domain_);
        mqtt_cfg_.uri = uri;
        mqtt_cfg_.cert_pem = tuya_cacert_pem;
        mqtt_cfg_.cert_len = sizeof(tuya_cacert_pem);
        mqtt_cfg_.skip_cert_common_name_check = true;
        mqtt_cfg_.use_global_ca_store = false;
        // mqtt_cfg_.transport = MQTT_TRANSPORT_OVER_SSL;
        mqtt_cfg_.protocol_ver = MQTT_PROTOCOL_V_3_1_1;
        static char client_id[50];
        sprintf(client_id, "tuyalink_%s", device_id_);
        mqtt_cfg_.client_id = client_id;
        client_ = esp_mqtt_client_init(&mqtt_cfg_);

        if (client_) {
          esp_mqtt_client_register_event(client_, MQTT_EVENT_ANY, mqtt_event_handler, this);
          esp_mqtt_client_start(client_);
          tuya_inited_ = true;
        } else {
          ESP_LOGD(TAG, "Failed to initialize IDF-MQTT");
          return;
        }
      }

      if (tuya_connected_ == true) {
        ESP_LOGD(TAG, "TuyaIotComponent::update connected=true");
      } else {
        ESP_LOGD(TAG, "TuyaIotComponent::update connected=false");
      }
    }

    // TuyaIotMessageTrigger
    TuyaIotMessageTrigger::TuyaIotMessageTrigger(std::string topic) : topic_(std::move(topic)) {}
    void TuyaIotMessageTrigger::set_qos(uint8_t qos) { this->qos_ = qos; }
    void TuyaIotMessageTrigger::set_payload(const std::string &payload) { this->payload_ = payload; }
    void TuyaIotMessageTrigger::setup() {
      global_tuya_iot->subscribe(
                                 this->topic_,
                                 [this](const std::string &topic, const std::string &payload) {
                                   if (this->payload_.has_value() && payload != *this->payload_) {
                                     return;
                                   }

                                   this->trigger(payload);
                                 },
                                 this->qos_);
    }
    void TuyaIotMessageTrigger::dump_config() {
      ESP_LOGCONFIG(TAG, "MQTT Message Trigger:");
      ESP_LOGCONFIG(TAG, "  Topic: '%s'", this->topic_.c_str());
      ESP_LOGCONFIG(TAG, "  QoS: %u", this->qos_);
    }
    float TuyaIotMessageTrigger::get_setup_priority() const { return setup_priority::AFTER_CONNECTION; }


  }
}
