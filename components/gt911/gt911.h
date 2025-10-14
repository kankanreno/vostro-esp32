#pragma once

#include "driver/i2c.h"
#include "driver/gpio.h"

// GT911 寄存器地址
#define GT911_I2C_ADDR1              0x5D
#define GT911_I2C_ADDR2              0x14

#define GT911_PRODUCT_ID_REG         0x8140
#define GT911_FIRMWARE_VERSION_REG   0x8144
#define GT911_STATUS_REG             0x814E
#define GT911_DATA_REG               0x8150

// GT911 状态寄存器位定义
#define GT911_BUFFER_READY           0x80
#define GT911_BUFFER_STATUS          0x80
#define GT911_LARGE_DETECT           0x40
#define GT911_POINT_COUNT_MASK       0x0F

// 最大触摸点数
#define GT911_MAX_TOUCH_POINTS       5

// 屏幕分辨率 - 根据你的实际屏幕修改
#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320

// 触摸点数据结构
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t id;
    uint8_t size;
    uint8_t pressure;
} gt911_touch_point_t;

typedef struct {
    i2c_port_t port;
    gpio_num_t irq_pin;
} gt911_conf_t;
 
esp_err_t gt911_init(gt911_conf_t *gt911_conf);
bool gt911_read(gt911_touch_point_t *points, uint8_t *count);
