#include "jc3248w535.h"

#include <cstring>

#include "esphome/core/log.h"
#include "esphome/core/application.h"

#include "esp_check.h"
#include "esp_heap_caps.h"
#include "esp_lcd_panel_vendor.h"
#include "freertos/semphr.h"

#include "esp_lcd_axs15231b_interface.h"

namespace esphome {
namespace jc3248w535 {

static const char *const TAG = "jc3248w535";

static const int WIDTH = 320;
static const int HEIGHT = 480;
static const int TEST_MARKER_RADIUS = 16;
static const spi_host_device_t SPI_HOST = SPI2_HOST;

static const gpio_num_t PIN_CS = GPIO_NUM_45;
static const gpio_num_t PIN_CLK = GPIO_NUM_47;
static const gpio_num_t PIN_DATA0 = GPIO_NUM_21;
static const gpio_num_t PIN_DATA1 = GPIO_NUM_48;
static const gpio_num_t PIN_DATA2 = GPIO_NUM_40;
static const gpio_num_t PIN_DATA3 = GPIO_NUM_39;
static const gpio_num_t PIN_BACKLIGHT = GPIO_NUM_1;

static const axs15231b_lcd_init_cmd_t INIT_CMDS[] = {
    {0xBB, (uint8_t[]) {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5A, 0xA5}, 8, 0},
    {0xA0, (uint8_t[]) {0xC0, 0x10, 0x00, 0x02, 0x00, 0x00, 0x04, 0x3F, 0x20, 0x05, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00}, 17, 0},
    {0xA2, (uint8_t[]) {0x30, 0x3C, 0x24, 0x14, 0xD0, 0x20, 0xFF, 0xE0, 0x40, 0x19, 0x80, 0x80, 0x80, 0x20, 0xF9, 0x10, 0x02, 0xFF, 0xFF, 0xF0, 0x90, 0x01, 0x32, 0xA0, 0x91, 0xE0, 0x20, 0x7F, 0xFF, 0x00, 0x5A}, 31, 0},
    {0xD0, (uint8_t[]) {0xE0, 0x40, 0x51, 0x24, 0x08, 0x05, 0x10, 0x01, 0x20, 0x15, 0x42, 0xC2, 0x22, 0x22, 0xAA, 0x03, 0x10, 0x12, 0x60, 0x14, 0x1E, 0x51, 0x15, 0x00, 0x8A, 0x20, 0x00, 0x03, 0x3A, 0x12}, 30, 0},
    {0xA3, (uint8_t[]) {0xA0, 0x06, 0xAA, 0x00, 0x08, 0x02, 0x0A, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x55, 0x55}, 22, 0},
    {0xC1, (uint8_t[]) {0x31, 0x04, 0x02, 0x02, 0x71, 0x05, 0x24, 0x55, 0x02, 0x00, 0x41, 0x00, 0x53, 0xFF, 0xFF, 0xFF, 0x4F, 0x52, 0x00, 0x4F, 0x52, 0x00, 0x45, 0x3B, 0x0B, 0x02, 0x0D, 0x00, 0xFF, 0x40}, 30, 0},
    {0xC3, (uint8_t[]) {0x00, 0x00, 0x00, 0x50, 0x03, 0x00, 0x00, 0x00, 0x01, 0x80, 0x01}, 11, 0},
    {0xC4, (uint8_t[]) {0x00, 0x24, 0x33, 0x80, 0x00, 0xEA, 0x64, 0x32, 0xC8, 0x64, 0xC8, 0x32, 0x90, 0x90, 0x11, 0x06, 0xDC, 0xFA, 0x00, 0x00, 0x80, 0xFE, 0x10, 0x10, 0x00, 0x0A, 0x0A, 0x44, 0x50}, 29, 0},
    {0xC5, (uint8_t[]) {0x18, 0x00, 0x00, 0x03, 0xFE, 0x3A, 0x4A, 0x20, 0x30, 0x10, 0x88, 0xDE, 0x0D, 0x08, 0x0F, 0x0F, 0x01, 0x3A, 0x4A, 0x20, 0x10, 0x10, 0x00}, 23, 0},
    {0xC6, (uint8_t[]) {0x05, 0x0A, 0x05, 0x0A, 0x00, 0xE0, 0x2E, 0x0B, 0x12, 0x22, 0x12, 0x22, 0x01, 0x03, 0x00, 0x3F, 0x6A, 0x18, 0xC8, 0x22}, 20, 0},
    {0xC7, (uint8_t[]) {0x50, 0x32, 0x28, 0x00, 0xA2, 0x80, 0x8F, 0x00, 0x80, 0xFF, 0x07, 0x11, 0x9C, 0x67, 0xFF, 0x24, 0x0C, 0x0D, 0x0E, 0x0F}, 20, 0},
    {0xC9, (uint8_t[]) {0x33, 0x44, 0x44, 0x01}, 4, 0},
    {0xCF, (uint8_t[]) {0x2C, 0x1E, 0x88, 0x58, 0x13, 0x18, 0x56, 0x18, 0x1E, 0x68, 0x88, 0x00, 0x65, 0x09, 0x22, 0xC4, 0x0C, 0x77, 0x22, 0x44, 0xAA, 0x55, 0x08, 0x08, 0x12, 0xA0, 0x08}, 27, 0},
    {0xD5, (uint8_t[]) {0x40, 0x8E, 0x8D, 0x01, 0x35, 0x04, 0x92, 0x74, 0x04, 0x92, 0x74, 0x04, 0x08, 0x6A, 0x04, 0x46, 0x03, 0x03, 0x03, 0x03, 0x82, 0x01, 0x03, 0x00, 0xE0, 0x51, 0xA1, 0x00, 0x00, 0x00}, 30, 0},
    {0xD6, (uint8_t[]) {0x10, 0x32, 0x54, 0x76, 0x98, 0xBA, 0xDC, 0xFE, 0x93, 0x00, 0x01, 0x83, 0x07, 0x07, 0x00, 0x07, 0x07, 0x00, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x00, 0x84, 0x00, 0x20, 0x01, 0x00}, 30, 0},
    {0xD7, (uint8_t[]) {0x03, 0x01, 0x0B, 0x09, 0x0F, 0x0D, 0x1E, 0x1F, 0x18, 0x1D, 0x1F, 0x19, 0x40, 0x8E, 0x04, 0x00, 0x20, 0xA0, 0x1F}, 19, 0},
    {0xD8, (uint8_t[]) {0x02, 0x00, 0x0A, 0x08, 0x0E, 0x0C, 0x1E, 0x1F, 0x18, 0x1D, 0x1F, 0x19}, 12, 0},
    {0xD9, (uint8_t[]) {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, 12, 0},
    {0xDD, (uint8_t[]) {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F}, 12, 0},
    {0xDF, (uint8_t[]) {0x44, 0x73, 0x4B, 0x69, 0x00, 0x0A, 0x02, 0x90}, 8, 0},
    {0xE0, (uint8_t[]) {0x3B, 0x28, 0x10, 0x16, 0x0C, 0x06, 0x11, 0x28, 0x5C, 0x21, 0x0D, 0x35, 0x13, 0x2C, 0x33, 0x28, 0x0D}, 17, 0},
    {0xE1, (uint8_t[]) {0x37, 0x28, 0x10, 0x16, 0x0B, 0x06, 0x11, 0x28, 0x5C, 0x21, 0x0D, 0x35, 0x14, 0x2C, 0x33, 0x28, 0x0F}, 17, 0},
    {0xE2, (uint8_t[]) {0x3B, 0x07, 0x12, 0x18, 0x0E, 0x0D, 0x17, 0x35, 0x44, 0x32, 0x0C, 0x14, 0x14, 0x36, 0x3A, 0x2F, 0x0D}, 17, 0},
    {0xE3, (uint8_t[]) {0x37, 0x07, 0x12, 0x18, 0x0E, 0x0D, 0x17, 0x35, 0x44, 0x32, 0x0C, 0x14, 0x14, 0x36, 0x32, 0x2F, 0x0F}, 17, 0},
    {0xE4, (uint8_t[]) {0x3B, 0x07, 0x12, 0x18, 0x0E, 0x0D, 0x17, 0x39, 0x44, 0x2E, 0x0C, 0x14, 0x14, 0x36, 0x3A, 0x2F, 0x0D}, 17, 0},
    {0xE5, (uint8_t[]) {0x37, 0x07, 0x12, 0x18, 0x0E, 0x0D, 0x17, 0x39, 0x44, 0x2E, 0x0C, 0x14, 0x14, 0x36, 0x3A, 0x2F, 0x0F}, 17, 0},
    {0xA4, (uint8_t[]) {0x85, 0x85, 0x95, 0x82, 0xAF, 0xAA, 0xAA, 0x80, 0x10, 0x30, 0x40, 0x40, 0x20, 0xFF, 0x60, 0x30}, 16, 0},
    {0xA4, (uint8_t[]) {0x85, 0x85, 0x95, 0x85}, 4, 0},
    {0xBB, (uint8_t[]) {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 8, 0},
    {0x13, (uint8_t[]) {0x00}, 0, 0},
    {0x11, (uint8_t[]) {0x00}, 0, 120},
    {0x2C, (uint8_t[]) {0x00, 0x00, 0x00, 0x00}, 4, 0},
};

void JC3248W535::setup() {
  ESP_LOGCONFIG(TAG, "Setting up JC3248W535 display...");
  this->init_backlight_();
  this->init_lcd_();
  this->init_internal_(this->get_buffer_length_());
  memset(this->buffer_, 0x00, this->get_buffer_length_());
  this->write_display_data_();
}

void JC3248W535::dump_config() {
  LOG_DISPLAY("", "JC3248W535 Display", this);
  ESP_LOGCONFIG(TAG, "  Resolution: %dx%d", WIDTH, HEIGHT);
  ESP_LOGCONFIG(TAG, "  QSPI pins: CS=%d CLK=%d D0=%d D1=%d D2=%d D3=%d", PIN_CS, PIN_CLK, PIN_DATA0, PIN_DATA1,
                PIN_DATA2, PIN_DATA3);
  ESP_LOGCONFIG(TAG, "  Backlight pin: %d", PIN_BACKLIGHT);
  LOG_UPDATE_INTERVAL(this);
}

float JC3248W535::get_setup_priority() const { return setup_priority::PROCESSOR; }

void JC3248W535::update() {
  this->pixel_ops_since_feed_ = 0;
  this->do_update_();
  this->write_display_data_();
}

void JC3248W535::draw_pixel_at(int x, int y, Color color) {
  if (!this->get_clipping().inside(x, y))
    return;

  switch (this->get_rotation()) {
    case display::DISPLAY_ROTATION_0_DEGREES:
      break;
    case display::DISPLAY_ROTATION_90_DEGREES: {
      std::swap(x, y);
      x = this->get_width_internal() - x - 1;
      break;
    }
    case display::DISPLAY_ROTATION_180_DEGREES:
      x = this->get_width_internal() - x - 1;
      y = this->get_height_internal() - y - 1;
      break;
    case display::DISPLAY_ROTATION_270_DEGREES: {
      std::swap(x, y);
      y = this->get_height_internal() - y - 1;
      break;
    }
  }

  this->draw_absolute_pixel_internal(x, y, color);
  if (((++this->pixel_ops_since_feed_) & 0x7FFU) == 0) {
    App.feed_wdt();
  }
}

void JC3248W535::update_test_marker(int x, int y, bool active) {
  const int old_x1 = this->test_marker_x_ - TEST_MARKER_RADIUS;
  const int old_y1 = this->test_marker_y_ - TEST_MARKER_RADIUS;
  const int new_x1 = x - TEST_MARKER_RADIUS;
  const int new_y1 = y - TEST_MARKER_RADIUS;
  const int rect_size = TEST_MARKER_RADIUS * 2 + 1;

  if (!this->test_marker_visible_ && !active)
    return;

  int flush_x1 = 0;
  int flush_y1 = 0;
  int flush_x2 = 0;
  int flush_y2 = 0;

  if (this->test_marker_visible_) {
    flush_x1 = old_x1;
    flush_y1 = old_y1;
    flush_x2 = old_x1 + rect_size;
    flush_y2 = old_y1 + rect_size;
  }

  if (active) {
    if (this->test_marker_visible_) {
      flush_x1 = std::min(flush_x1, new_x1);
      flush_y1 = std::min(flush_y1, new_y1);
      flush_x2 = std::max(flush_x2, new_x1 + rect_size);
      flush_y2 = std::max(flush_y2, new_y1 + rect_size);
    } else {
      flush_x1 = new_x1;
      flush_y1 = new_y1;
      flush_x2 = new_x1 + rect_size;
      flush_y2 = new_y1 + rect_size;
    }
  }

  this->paint_test_background_rect_(flush_x1, flush_y1, flush_x2 - flush_x1, flush_y2 - flush_y1);
  if (active)
    this->paint_test_marker_rect_(x, y);
  this->write_display_data_();

  this->test_marker_visible_ = active;
  if (active) {
    this->test_marker_x_ = x;
    this->test_marker_y_ = y;
  }
}

void JC3248W535::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (x < 0 || y < 0 || x >= WIDTH || y >= HEIGHT)
    return;

  auto color565 = display::ColorUtil::color_to_565(color);
  uint32_t pos = (x + y * WIDTH) * 2;
  this->buffer_[pos++] = (color565 >> 8) & 0xFF;
  this->buffer_[pos] = color565 & 0xFF;
}

size_t JC3248W535::get_buffer_length_() const { return size_t(WIDTH) * size_t(HEIGHT) * 2; }

void JC3248W535::set_backlight_brightness(float brightness) {
  if (brightness < 0.0f)
    brightness = 0.0f;
  if (brightness > 1.0f)
    brightness = 1.0f;

  this->backlight_brightness_ = brightness;
  const uint32_t duty = static_cast<uint32_t>(brightness * 1023.0f + 0.5f);
  ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, duty));
  ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1));
}

void JC3248W535::init_backlight_() {
  const ledc_timer_config_t timer = {
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .duty_resolution = LEDC_TIMER_10_BIT,
      .timer_num = LEDC_TIMER_1,
      .freq_hz = 5000,
      .clk_cfg = LEDC_AUTO_CLK,
  };
  const ledc_channel_config_t channel = {
      .gpio_num = PIN_BACKLIGHT,
      .speed_mode = LEDC_LOW_SPEED_MODE,
      .channel = LEDC_CHANNEL_1,
      .intr_type = LEDC_INTR_DISABLE,
      .timer_sel = LEDC_TIMER_1,
      .duty = 1023,
      .hpoint = 0,
      .flags = {
          .output_invert = 0,
      },
  };
  ESP_ERROR_CHECK(ledc_timer_config(&timer));
  ESP_ERROR_CHECK(ledc_channel_config(&channel));
  this->set_backlight_brightness(this->backlight_brightness_);
}

void JC3248W535::init_lcd_() {
  spi_bus_config_t buscfg = {};
  esp_lcd_panel_io_spi_config_t io_config = {};
  axs15231b_vendor_config_t vendor_config = {};
  esp_lcd_panel_dev_config_t panel_config = {};

  buscfg.sclk_io_num = PIN_CLK;
  buscfg.data0_io_num = PIN_DATA0;
  buscfg.data1_io_num = PIN_DATA1;
  buscfg.data2_io_num = PIN_DATA2;
  buscfg.data3_io_num = PIN_DATA3;
  buscfg.max_transfer_sz = this->get_buffer_length_();

  ESP_ERROR_CHECK(spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

  io_config.cs_gpio_num = PIN_CS;
  io_config.dc_gpio_num = -1;
  io_config.spi_mode = 3;
  io_config.pclk_hz = 50 * 1000 * 1000;
  io_config.trans_queue_depth = 1;
  io_config.on_color_trans_done = &JC3248W535::color_trans_done_callback_;
  io_config.user_ctx = this;
  io_config.lcd_cmd_bits = 32;
  io_config.lcd_param_bits = 8;
  io_config.flags.quad_mode = 1;

  if (this->color_trans_done_sem_ == nullptr)
    this->color_trans_done_sem_ = xSemaphoreCreateBinary();

  ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t) SPI_HOST, &io_config, &this->io_handle_));

  vendor_config.init_cmds = INIT_CMDS;
  vendor_config.init_cmds_size = sizeof(INIT_CMDS) / sizeof(INIT_CMDS[0]);
  vendor_config.flags.use_qspi_interface = 1;

  panel_config.reset_gpio_num = GPIO_NUM_NC;
  panel_config.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB;
  panel_config.data_endian = LCD_RGB_DATA_ENDIAN_BIG;
  panel_config.bits_per_pixel = 16;
  panel_config.flags.reset_active_high = 0;
  panel_config.vendor_config = (void *) &vendor_config;

  ESP_ERROR_CHECK(esp_lcd_new_panel_axs15231b(this->io_handle_, &panel_config, &this->panel_));
  ESP_ERROR_CHECK(esp_lcd_panel_reset(this->panel_));
  ESP_ERROR_CHECK(esp_lcd_panel_init(this->panel_));
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(this->panel_, false));
}

void JC3248W535::write_display_data_() {
  this->queue_bitmap_and_wait_(0, 0, WIDTH, HEIGHT, this->buffer_);
}

void JC3248W535::queue_bitmap_and_wait_(int x, int y, int w, int h, const void *data) {
  if (w <= 0 || h <= 0)
    return;

  if (this->color_trans_done_sem_ != nullptr) {
    while (xSemaphoreTake(this->color_trans_done_sem_, 0) == pdTRUE) {
    }
  }

  ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(this->panel_, x, y, x + w, y + h, data));

  if (this->color_trans_done_sem_ != nullptr)
    xSemaphoreTake(this->color_trans_done_sem_, portMAX_DELAY);
}

void JC3248W535::ensure_transfer_buffer_(size_t required_bytes) {
  if (required_bytes <= this->transfer_buffer_bytes_ && this->transfer_buffer_ != nullptr)
    return;

  if (this->transfer_buffer_ != nullptr) {
    heap_caps_free(this->transfer_buffer_);
    this->transfer_buffer_ = nullptr;
    this->transfer_buffer_bytes_ = 0;
  }

  this->transfer_buffer_ =
      static_cast<uint8_t *>(heap_caps_malloc(required_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT));
  if (this->transfer_buffer_ == nullptr) {
    ESP_LOGE(TAG, "Failed to allocate %u bytes for display transfer buffer", static_cast<unsigned>(required_bytes));
    return;
  }

  this->transfer_buffer_bytes_ = required_bytes;
}

bool JC3248W535::color_trans_done_callback_(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata,
                                            void *user_ctx) {
  auto *self = static_cast<JC3248W535 *>(user_ctx);
  if (self == nullptr || self->color_trans_done_sem_ == nullptr)
    return false;

  BaseType_t high_task_wakeup = pdFALSE;
  xSemaphoreGiveFromISR(self->color_trans_done_sem_, &high_task_wakeup);
  return high_task_wakeup == pdTRUE;
}

void JC3248W535::flush_logical_rect_(int x, int y, int w, int h) {
  const int logical_width = this->get_width();
  const int logical_height = this->get_height();

  if (w <= 0 || h <= 0)
    return;

  int x1 = std::max(0, x);
  int y1 = std::max(0, y);
  int x2 = std::min(logical_width, x + w);
  int y2 = std::min(logical_height, y + h);

  if (x1 >= x2 || y1 >= y2)
    return;

  int native_x = 0;
  int native_y = 0;
  int native_w = 0;
  int native_h = 0;

  switch (this->get_rotation()) {
    case display::DISPLAY_ROTATION_0_DEGREES:
      native_x = x1;
      native_y = y1;
      native_w = x2 - x1;
      native_h = y2 - y1;
      break;
    case display::DISPLAY_ROTATION_90_DEGREES:
      native_x = WIDTH - y2;
      native_y = x1;
      native_w = y2 - y1;
      native_h = x2 - x1;
      break;
    case display::DISPLAY_ROTATION_180_DEGREES:
      native_x = WIDTH - x2;
      native_y = HEIGHT - y2;
      native_w = x2 - x1;
      native_h = y2 - y1;
      break;
    case display::DISPLAY_ROTATION_270_DEGREES:
      native_x = y1;
      native_y = HEIGHT - x2;
      native_w = y2 - y1;
      native_h = x2 - x1;
      break;
  }

  const size_t row_bytes = static_cast<size_t>(native_w) * 2;
  const size_t required_bytes = row_bytes * static_cast<size_t>(native_h);
  this->ensure_transfer_buffer_(required_bytes);
  if (this->transfer_buffer_ == nullptr)
    return;

  for (int row = 0; row < native_h; row++) {
    const uint8_t *src = this->buffer_ + (((native_y + row) * WIDTH) + native_x) * 2;
    memcpy(this->transfer_buffer_ + (row * row_bytes), src, row_bytes);
  }

  this->queue_bitmap_and_wait_(native_x, native_y, native_w, native_h, this->transfer_buffer_);
}

void JC3248W535::paint_test_background_rect_(int x, int y, int w, int h) {
  const int logical_width = this->get_width();
  const int logical_height = this->get_height();
  const int x_start = std::max(0, x);
  const int y_start = std::max(0, y);
  const int x_end = std::min(logical_width, x + w);
  const int y_end = std::min(logical_height, y + h);

  for (int py = y_start; py < y_end; py++) {
    for (int px = x_start; px < x_end; px++) {
      Color color;
      if (px == 0 || px == 239 || px == 240 || px == 479 || py == 0 || py == 159 || py == 160 || py == 319) {
        color = Color::WHITE;
      } else if (px < 240 && py < 160) {
        color = Color(120, 30, 30);
      } else if (px >= 240 && py < 160) {
        color = Color(30, 120, 30);
      } else if (px < 240 && py >= 160) {
        color = Color(30, 30, 120);
      } else {
        color = Color(120, 90, 20);
      }
      this->draw_pixel_at(px, py, color);
    }
  }
}

void JC3248W535::paint_test_marker_rect_(int x, int y) {
  const int outer_sq = TEST_MARKER_RADIUS * TEST_MARKER_RADIUS;
  const int inner_sq = 10 * 10;

  for (int py = y - TEST_MARKER_RADIUS; py <= y + TEST_MARKER_RADIUS; py++) {
    for (int px = x - TEST_MARKER_RADIUS; px <= x + TEST_MARKER_RADIUS; px++) {
      int dx = px - x;
      int dy = py - y;
      int dist_sq = dx * dx + dy * dy;

      if (dist_sq <= inner_sq) {
        this->draw_pixel_at(px, py, Color::WHITE);
      } else if (dist_sq >= outer_sq - TEST_MARKER_RADIUS && dist_sq <= outer_sq + TEST_MARKER_RADIUS) {
        this->draw_pixel_at(px, py, Color::BLACK);
      }
    }
  }
}

}  // namespace jc3248w535
}  // namespace esphome
