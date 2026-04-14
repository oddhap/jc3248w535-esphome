#pragma once

#include "esphome/components/display/display_buffer.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

namespace esphome {
namespace jc3248w535 {

class JC3248W535 : public display::DisplayBuffer {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_width_internal() override { return 320; }
  int get_height_internal() override { return 480; }

  void init_backlight_();
  void init_lcd_();
  void write_display_data_();
  size_t get_buffer_length_() const;

  esp_lcd_panel_handle_t panel_{nullptr};
  esp_lcd_panel_io_handle_t io_handle_{nullptr};
};

}  // namespace jc3248w535
}  // namespace esphome
