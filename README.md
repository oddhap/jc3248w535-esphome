# JC3248W535 ESPHome External Component

ESPHome external display component for the `JC3248W535` ESP32-S3 3.5" QSPI touch display.

This repository packages the working vendor QSPI LCD driver and AXS15231 touch controller as ESPHome external components.

## Status

- Display driver works on the `JC3248W535` board
- Resolution is fixed at `320x480`
- Backlight is handled internally on `GPIO1`
- QSPI LCD pins are fixed for this board
- Touch support is included through the `axs15231` touchscreen platform

## Quick Start

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/oddhap/jc3248w535-esphome
      ref: main
    components: [jc3248w535, axs15231]
```

## Example Integration

```yaml
esphome:
  name: jc3248w535-demo

esp32:
  board: esp32-s3-devkitc-1
  flash_size: 8MB
  framework:
    type: esp-idf

psram:
  mode: octal
  speed: 80MHz

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

logger:
api:
ota:
  - platform: esphome

external_components:
  - source:
      type: git
      url: https://github.com/oddhap/jc3248w535-esphome
      ref: main
    components: [jc3248w535, axs15231]

display:
  - platform: jc3248w535
    id: main_display
    update_interval: 1s
    lambda: |-
      it.fill(Color::BLACK);
      it.printf(20, 20, id(my_font), Color(255, 255, 255), "Hei fra JC3248W535");

i2c:
  sda: 4
  scl: 8

touchscreen:
  - platform: axs15231
    id: main_touch
    display: main_display
    interrupt_pin: GPIO3
    transform:
      swap_xy: true
      mirror_y: true

font:
  - file: "gfonts://Roboto"
    id: my_font
    size: 24
```

## Notes

- This component is intended for the `JC3248W535` board and uses its fixed hardware pinout.
- The component uses the vendor LCD initialization sequence that worked in local hardware testing.
- For the JC3248W535 board, the touch bus is typically `sda: 4`, `scl: 8`, address `0x3B`, and interrupt on `GPIO3`.
