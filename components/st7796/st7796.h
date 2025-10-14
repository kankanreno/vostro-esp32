#pragma once

#include "driver/spi_master.h"
#include "driver/gpio.h"

// 屏幕分辨率 - 根据你的实际屏幕修改
#define MY_DISP_HOR_RES  480
#define MY_DISP_VER_RES 320
 
typedef struct {
    spi_device_handle_t spi;
    gpio_num_t dc_pin;
    gpio_num_t rst_pin;
    gpio_num_t cs_pin;
    uint16_t width;
    uint16_t height;
} st7796_conf_t;
 
void st7796_init(st7796_conf_t *st7796_conf);
void st7796_set_window(st7796_conf_t *st7796_conf, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void st7796_flush(st7796_conf_t *st7796_conf, const void *data, size_t len);
