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
    name: "tuya test 103"
    id: tuya_test_switch_103
    on_turn_on:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report("output1", true);
    on_turn_off:
      - lambda: !lambda |-
          id(tuya_iot_component).property_report("output1", false);

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
```