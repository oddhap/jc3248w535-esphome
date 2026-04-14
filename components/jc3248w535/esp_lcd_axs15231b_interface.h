#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"

#define ESP_LCD_AXS15231B_VER_MAJOR (1)
#define ESP_LCD_AXS15231B_VER_MINOR (0)
#define ESP_LCD_AXS15231B_VER_PATCH (0)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int cmd;
  const void *data;
  size_t data_bytes;
  unsigned int delay_ms;
} axs15231b_lcd_init_cmd_t;

typedef struct {
  const axs15231b_lcd_init_cmd_t *init_cmds;
  uint16_t init_cmds_size;
  struct {
    unsigned int use_qspi_interface : 1;
  } flags;
} axs15231b_vendor_config_t;

esp_err_t esp_lcd_new_panel_axs15231b(const esp_lcd_panel_io_handle_t io,
                                      const esp_lcd_panel_dev_config_t *panel_dev_config,
                                      esp_lcd_panel_handle_t *ret_panel);

#ifdef __cplusplus
}
#endif
