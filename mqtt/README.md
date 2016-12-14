# MQTT firmware for H801 LED dimmer

The is an alternative firmware for the H801 LED dimmer that uses MQTT as a control channel. This makes it easy to integrate into Home Assistant and other Home Automation applications.

## Channels

Channel | Remark
XXXXXXXX/w[12]/light/status | Status of W1/W2 channel
XXXXXXXX/w[12]/light/switch | Set W1/W2 ON or OFF
XXXXXXXX/w[12]/brightness/status | Brightness of W1/W2 channel
XXXXXXXX/w[12]/brightness/set | Set brightness of W1/W2 channel

## Home assistant example configuration

```yaml
light:
- platform: mqtt
  name: "W1"
  state_topic: "0085652A/w1/light/status"
  command_topic: "0085652A/w1/light/switch"
  brightness_state_topic: "0085652A/w1/brightness/status"
  brightness_command_topic: "0085652A/w1/brightness/set"
  state_value_template: "{{ value_json.state }}"
  brightness_value_template: "{{ value_json.brightness }}"
  qos: 0
  payload_on: "ON"
  payload_off: "OFF"
  optimistic: false

- platform: mqtt
  name: "W2"
  state_topic: "0085652A/w2/light/status"
  command_topic: "0085652A/w2/light/switch"
  brightness_state_topic: "0085652A/w2/brightness/status"
  brightness_command_topic: "0085652A/w2/brightness/set"
  state_value_template: "{{ value_json.state }}"
  brightness_value_template: "{{ value_json.brightness }}"
  qos: 0
  payload_on: "ON"
  payload_off: "OFF"
  optimistic: false

- platform: mqtt
  name: "W3" #RED Pin
  state_topic: "0085652A/w3/light/status"
  command_topic: "0085652A/w3/light/switch"
  brightness_state_topic: "0085652A/w3/brightness/status"
  brightness_command_topic: "0085652A/w3/brightness/set"
  state_value_template: "{{ value_json.state }}"
  brightness_value_template: "{{ value_json.brightness }}"
  qos: 0
  payload_on: "ON"
  payload_off: "OFF"
  optimistic: false
  
- platform: mqtt
  name: "W4" #GREEN Pin
  state_topic: "0085652A/w4/light/status"
  command_topic: "0085652A/w4/light/switch"
  brightness_state_topic: "0085652A/w4/brightness/status"
  brightness_command_topic: "0085652A/w4/brightness/set"
  state_value_template: "{{ value_json.state }}"
  brightness_value_template: "{{ value_json.brightness }}"
  qos: 0
  payload_on: "ON"
  payload_off: "OFF"
  optimistic: false
  
- platform: mqtt
  name: "W5" #BLUE Pin
  state_topic: "0085652A/w5/light/status"
  command_topic: "0085652A/w5/light/switch"
  brightness_state_topic: "0085652A/w5/brightness/status"
  brightness_command_topic: "0085652A/w5/brightness/set"
  state_value_template: "{{ value_json.state }}"
  brightness_value_template: "{{ value_json.brightness }}"
  qos: 0
  payload_on: "ON"
  payload_off: "OFF"
  optimistic: false
