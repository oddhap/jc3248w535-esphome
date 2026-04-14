# JC3248W535 ESPHome External Component

ESPHome external display component for the `JC3248W535` ESP32-S3 3.5" QSPI touch display.

This repository packages the working vendor QSPI LCD driver as an ESPHome `display` platform so it can be pulled in with `external_components`.

## Status

- Display driver works on the `JC3248W535` board
- Resolution is fixed at `320x480`
- Backlight is handled internally on `GPIO1`
- QSPI LCD pins are fixed for this board
- Touch is not included in this component yet

## Quick Start

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/oddhap/jc3248w535-esphome
      ref: main
    components: [jc3248w535]
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
    components: [jc3248w535]

display:
  - platform: jc3248w535
    id: main_display
    update_interval: 1s
    lambda: |-
      it.fill(Color::BLACK);
      it.printf(20, 20, id(my_font), Color(255, 255, 255), "Hei fra JC3248W535");

font:
  - file: "gfonts://Roboto"
    id: my_font
    size: 24
```

## Notes

- This component is intended for the `JC3248W535` board and uses its fixed hardware pinout.
- The component uses the vendor LCD initialization sequence that worked in local hardware testing.
- For now this repository only contains the display driver. Touch support can be added later as a separate component.
