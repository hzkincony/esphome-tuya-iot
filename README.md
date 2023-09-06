# esphome-tuya-iot

```yaml
esphome:
  name: tuya-test
  friendly_name: tuya-test

external_components:
  - source:
      type: git
      url: https://github.com/idreamshen/esphome-tuya-iot

esp32:
  board: esp32dev
  framework:
    type: arduino

# Enable logging
logger:

captive_portal:

time:
  - platform: homeassistant
    id: homeassistant_time

tuya_iot:
  product_id: xxx
  device_id: xxxx
  device_secret: xxxx
    
```