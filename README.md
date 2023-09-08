# esphome-tuya-iot

```yaml
esphome:
  name: tuya-test
  friendly_name: tuya-test

external_components:
  - source:
      type: git
      url: https://github.com/idreamshen/esphome-tuya-iot
    refresh: 5min

esp32:
  board: esp32dev
  framework:
    type: arduino

# Enable logging
logger:

# Enable Home Assistant API
api:

ota:
  password: "xxxxxxx"

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

  # Enable fallback hotspot (captive portal) in case wifi connection fails
  ap:
    ssid: "Tuya-Test Fallback Hotspot"
    password: "xxxx"

captive_portal:

time:
  - platform: homeassistant
    id: homeassistant_time

switch:
  - platform: gpio
    pin: 25
    name: "tuya test 103"
    id: tuya_test_switch_103
    on_turn_on:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report_json([=](JsonObject root) {
            root["msgId"] = id(tuya_iot_component).gen_msg_id();
            root["time"] = id(homeassistant_time).now().timestamp;
            root["data"]["output1"] = true;
          });
    on_turn_off:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report_json([=](JsonObject root) {
            root["msgId"] = id(tuya_iot_component).gen_msg_id();
            root["time"] = id(homeassistant_time).now().timestamp;
            root["data"]["output1"] = false;
          });
  - platform: gpio
    pin: 26
    name: "tuya test 104"
    id: tuya_test_switch_104
    on_turn_on:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report_json([=](JsonObject root) {
            root["msgId"] = id(tuya_iot_component).gen_msg_id();
            root["time"] = id(homeassistant_time).now().timestamp;
            root["data"]["output2"] = true;
          });
    on_turn_off:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report_json([=](JsonObject root) {
            root["msgId"] = id(tuya_iot_component).gen_msg_id();
            root["time"] = id(homeassistant_time).now().timestamp;
            root["data"]["output2"] = false;
          });

tuya_iot:
  id: tuya_iot_component
  product_id: xxx
  device_id: xxx
  device_secret: xxx

  on_json_message:
      - topic: tylink/${device_id}/thing/property/set
        qos: 0
        then:
          - logger.log: MQTT message property/set
          - lambda: !lambda |-
              const char *msgId = "None";
              if (x.containsKey("msgId")) {
                msgId = x["msgId"];
              }

              bool is_target_output1 = false;
              bool output1_state = false;

              if (x.containsKey("data") && x["data"].containsKey("output1")) {
                is_target_output1 = true;
                output1_state = x["data"]["output1"];
              }
              
              if (is_target_output1) {
                if (output1_state) {
                  id(tuya_test_switch_103).turn_on();
                  id(tuya_iot_component).property_report_json([=](JsonObject root) {
                    root["msgId"] = msgId;
                    root["time"] = id(homeassistant_time).now().timestamp;
                    root["data"]["output1"] = true;
                  });
                } else {
                  id(tuya_test_switch_103).turn_off();
                  id(tuya_iot_component).property_report_json([=](JsonObject root) {
                    root["msgId"] = msgId;
                    root["time"] = id(homeassistant_time).now().timestamp;
                    root["data"]["output1"] = false;
                  });
                }
              }
          - lambda: !lambda |-
              const char *msgId = "None";
              if (x.containsKey("msgId")) {
                msgId = x["msgId"];
              }

              bool is_target_output2 = false;
              bool output2_state = false;

              if (x.containsKey("data") && x["data"].containsKey("output2")) {
                is_target_output2 = true;
                output2_state = x["data"]["output2"];
              }
              
              if (is_target_output2) {
                if (output2_state) {
                  id(tuya_test_switch_104).turn_on();
                  id(tuya_iot_component).property_report_json([=](JsonObject root) {
                    root["msgId"] = msgId;
                    root["time"] = id(homeassistant_time).now().timestamp;
                    root["data"]["output2"] = true;
                  });
                } else {
                  id(tuya_test_switch_104).turn_off();
                  id(tuya_iot_component).property_report_json([=](JsonObject root) {
                    root["msgId"] = msgId;
                    root["time"] = id(homeassistant_time).now().timestamp;
                    root["data"]["output2"] = false;
                  });
                }
              }
          - lambda: !lambda |-
              const char *msgId = "None";
              if (x.containsKey("msgId")) {
                msgId = x["msgId"];
              }
              
              bool is_target_all_on = false;
              bool all_on_state = false;

              if (x.containsKey("data") && x["data"].containsKey("all_on")) {
                is_target_all_on = true;
                all_on_state = x["data"]["all_on"];
              }
              
              if (is_target_all_on) {
                if (all_on_state) {
                  id(tuya_test_switch_103).turn_on();
                  id(tuya_test_switch_104).turn_on();
                  id(tuya_iot_component).property_report_json([=](JsonObject root) {
                    root["msgId"] = msgId;
                    root["time"] = id(homeassistant_time).now().timestamp;
                    root["data"]["output1"] = true;
                    root["data"]["output2"] = true;
                  });
                }
              }
          - lambda: !lambda |-
              const char *msgId = "None";
              if (x.containsKey("msgId")) {
                msgId = x["msgId"];
              }
              
              bool is_target_all_off = false;
              bool all_off_state = false;

              if (x.containsKey("data") && x["data"].containsKey("all_off")) {
                is_target_all_off = true;
                all_off_state = x["data"]["all_off"];
              }
              
              if (is_target_all_off) {
                if (all_off_state) {
                  id(tuya_test_switch_103).turn_off();
                  id(tuya_test_switch_104).turn_off();
                  id(tuya_iot_component).property_report_json([=](JsonObject root) {
                    root["msgId"] = msgId;
                    root["time"] = id(homeassistant_time).now().timestamp;
                    root["data"]["output1"] = false;
                    root["data"]["output2"] = false;
                  });
                }
              }
    
```