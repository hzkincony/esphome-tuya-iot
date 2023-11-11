# esphome-tuya-iot

```yaml
esphome:
  name: tuya-test
  friendly_name: tuya-test

external_components:
  - source:
      type: git
      url: https://github.com/hzkincony/esphome-tuya-iot
      ref: v1.2.0

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
    name: "switch test 1"
    id: switch_test_1
    on_turn_on:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report("output1", true);
    on_turn_off:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report("output1", false);
  - platform: gpio
    pin: 26
    name: "switch test 2"
    id: switch_test_2
    on_turn_on:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report("output2", true);
    on_turn_off:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report("output2", false);

tuya_iot:
  id: tuya_iot_component
  product_id: xxx
  device_id: xxx
  device_secret: xxx
  region: eu # eu, us, eus, weu, in, cn
  # eu: Central Europe Data Center
  # us: US West Data Center
  # eus: US East Data Center
  # weu: Western Europe Data Center
  # in: India Data Center
  # cn: Chinese Data Center

  on_event:
    - event_name: property/set
      then:
        - lambda: !lambda |-
              // Append the required switch in switch_map to respond to commands issued by Tuya.
              static std::map<float, switch_::Switch*> switch_map {
                { 1, id(switch_test_1) },
                { 2, id(switch_test_2) },
              };

              bool is_target_output = false;
              bool output_state = false;
              int output_number = 0;
              // Modify this number 2 to match the number of switches in the above switch_map.
              for (int i = 1; i <= 2; i++) {
                String key = "output" + String(i);
                if (x.containsKey("data") && x["data"].containsKey(key.c_str())) {
                  is_target_output = true;
                  output_state = x["data"][key.c_str()];
                  output_number = i;
                  break;
                }
              }

              if (is_target_output) {
                auto iterator = switch_map.find(output_number);
                if (iterator != switch_map.end()) {
                  auto sw = iterator->second;
                  if (output_state) {
                    sw->turn_on();
                  } else {
                    sw->turn_off();
                  }
                }
              }

              bool is_target_all_on = false;
              bool all_on_state = false;

              if (x.containsKey("data") && x["data"].containsKey("all_on")) {
                is_target_all_on = true;
                all_on_state = x["data"]["all_on"];
              }

              if (is_target_all_on) {
                if (all_on_state) {
                  for(const auto& pair : switch_map) {
                    auto sw = pair.second;
                    sw->turn_on();
                  }
                }
              }

              bool is_target_all_off = false;
              bool all_off_state = false;

              if (x.containsKey("data") && x["data"].containsKey("all_off")) {
                is_target_all_off = true;
                all_off_state = x["data"]["all_off"];
              }

              if (is_target_all_off) {
                if (all_off_state) {
                  for(const auto& pair : switch_map) {
                    auto sw = pair.second;
                    sw->turn_off();
                  }
                }
              }
```