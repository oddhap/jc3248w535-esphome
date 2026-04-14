#pragma once

#include "esphome/components/display/display_buffer.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace esphome {
namespace jc3248w535 {

class JC3248W535 : public display::DisplayBuffer {
 public:
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;
  void update_test_marker(int x, int y, bool active);

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_COLOR; }

 protected:
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  int get_width_internal() override { return 320; }
  int get_height_internal() override { return 480; }

  void init_backlight_();
  void init_lcd_();
  void write_display_data_();
  void queue_bitmap_and_wait_(int x, int y, int w, int h, const void *data);
  void flush_logical_rect_(int x, int y, int w, int h);
  void paint_test_background_rect_(int x, int y, int w, int h);
  void paint_test_marker_rect_(int x, int y);
  void ensure_transfer_buffer_(size_t required_bytes);
  static bool color_trans_done_callback_(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata,
                                         void *user_ctx);
  size_t get_buffer_length_() const;

  esp_lcd_panel_handle_t panel_{nullptr};
  esp_lcd_panel_io_handle_t io_handle_{nullptr};
  SemaphoreHandle_t color_trans_done_sem_{nullptr};
  uint8_t *transfer_buffer_{nullptr};
  size_t transfer_buffer_bytes_{0};
  bool test_marker_visible_{false};
  int test_marker_x_{0};
  int test_marker_y_{0};
};

}  // namespace jc3248w535
}  // namespace esphome
