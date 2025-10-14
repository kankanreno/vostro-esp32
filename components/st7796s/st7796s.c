#include "st7796s.h"
#include "esp_log.h"
 
static const char *TAG = "ST7796S";
 
// 函数实现
static void send_cmd(st7796s_conf_t *st7796s_conf, uint8_t cmd) {
    gpio_set_level(st7796s_conf->dc_pin, 0);  // DC=0 表示命令
    spi_transaction_t trans = {  
        .length = 8,  /* 1字节命令 */  
        .tx_buffer = &cmd,  
        .flags = 0  
    };
    esp_err_t ret = spi_device_transmit(st7796s_conf->spi, &trans);
    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI write failed: 0x%X", ret);
    }
}
 
static void send_data(st7796s_conf_t *st7796s_conf, uint8_t *data, size_t len) {
    spi_transaction_t trans = {  
        .length = len * 8,  /* 字节转位 */  
        .tx_buffer = data,  
        .flags = 0
    }; 
    gpio_set_level(st7796s_conf->dc_pin, 1);  // DC=1 表示数据
    spi_device_transmit(st7796s_conf->spi, &trans);
}

void st7796s_init(st7796s_conf_t *st7796s_conf) {
    ESP_LOGI(TAG, "Initializing ST7796S");

    // 硬件复位
    gpio_set_direction(st7796s_conf->rst_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(st7796s_conf->rst_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(st7796s_conf->rst_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // SPI初始化
    spi_bus_config_t buscfg = {
        .miso_io_num = -1,
        .mosi_io_num = 13,
        .sclk_io_num = 14,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI3_HOST, &buscfg, 1));
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 40*1000*1000,
        .mode = 0,
        .spics_io_num = st7796s_conf->cs_pin,
        .queue_size = 3
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI3_HOST, &devcfg, &st7796s_conf->spi));

    // 定义初始化命令序列（使用结构体数组存储命令+参数）
    static const struct {
        uint8_t cmd;
        uint8_t data[16];
        uint8_t len;
    } init_cmds[] = {
        {0x11, {0}, 0x00},     // Sleep out
        {0x36, {0x08}, 0x01},  // MADCTL: BGR order
        {0x3A, {0x55}, 0x01},  // COLMOD: 16-bit RGB565
        {0xB2, {0x0C,0x0C,0x00,0x33,0x33}, 0x05}, // Porch control
        {0xB7, {0x01}, 0x01},  // Gate control
        {0xBB, {0x1A}, 0x01},  // VCOM setting
        {0xE8, {0x40,0x8A,0x00,0x00,0x29,0x19,0xA5,0x33}, 0x08}, // Driver timing
        {0x29, {0}, 0x00},     // Display on
    };
 
    // 执行初始化序列
    for (int i = 0; i < sizeof(init_cmds)/sizeof(init_cmds[0]); i++) {
        send_cmd(st7796s_conf, init_cmds[i].cmd);
        if (init_cmds[i].len > 0) {
            send_data(st7796s_conf, (uint8_t*)init_cmds[i].data, init_cmds[i].len);
        }
        vTaskDelay(10); // 命令间延迟
    }

    ESP_LOGI(TAG, "Display initialized successfully");
}
