#include "axs15231_touchscreen.h"

#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

namespace esphome {
namespace axs15231 {

static const char *const TAG = "ax15231.touchscreen";

constexpr static const uint8_t AXS_READ_TOUCHPAD[11] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08};

#define ERROR_CHECK(err)            \
  if ((err) != i2c::ERROR_OK) {     \
    this->status_set_warning();     \
    ESP_LOGW(TAG, "Touch I2C error"); \
    return;                         \
  }

template<typename T>
auto read_touchpad_(T *device, uint8_t *data, size_t len, int)
    -> decltype(device->write_read(AXS_READ_TOUCHPAD, sizeof(AXS_READ_TOUCHPAD), data, len)) {
  return device->write_read(AXS_READ_TOUCHPAD, sizeof(AXS_READ_TOUCHPAD), data, len);
}

template<typename T> i2c::ErrorCode read_touchpad_(T *device, uint8_t *data, size_t len, long) {
  auto err = device->write(AXS_READ_TOUCHPAD, sizeof(AXS_READ_TOUCHPAD), false);
  if (err != i2c::ERROR_OK)
    return err;
  return device->read(data, len);
}

void AXS15231Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AXS15231 Touchscreen...");
  if (this->reset_pin_ != nullptr) {
    this->reset_pin_->setup();
    this->reset_pin_->digital_write(false);
    delay(5);
    this->reset_pin_->digital_write(true);
    delay(10);
  }
  if (this->interrupt_pin_ != nullptr) {
    this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT);
    this->interrupt_pin_->setup();
    this->attach_interrupt_(this->interrupt_pin_, gpio::INTERRUPT_FALLING_EDGE);
  }
  if (this->x_raw_max_ == 0) {
    this->x_raw_max_ = this->display_->get_native_width();
  }
  if (this->y_raw_max_ == 0) {
    this->y_raw_max_ = this->display_->get_native_height();
  }
  ESP_LOGCONFIG(TAG, "AXS15231 Touchscreen setup complete");
}

void AXS15231Touchscreen::update_touches() {
  uint8_t data[8]{};

  const auto err = read_touchpad_(this, data, sizeof(data), 0);
  ERROR_CHECK(err);
  this->status_clear_warning();

  if (data[0] != 0 || data[1] == 0)
    return;

  const uint16_t x = encode_uint16(data[2] & 0x0F, data[3]);
  const uint16_t y = encode_uint16(data[4] & 0x0F, data[5]);
  this->add_raw_touch_position_(0, x, y);
}

void AXS15231Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "AXS15231 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  LOG_PIN("  Reset Pin: ", this->reset_pin_);
  ESP_LOGCONFIG(TAG, "  Width: %d", this->x_raw_max_);
  ESP_LOGCONFIG(TAG, "  Height: %d", this->y_raw_max_);
}

}  // namespace axs15231
}  // namespace esphome
