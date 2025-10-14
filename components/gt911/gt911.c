#include "gt911.h"
#include "esp_log.h"

static const char *TAG = "GT911";

// I2C写函数
bool i2c_write(uint8_t dev_addr, uint8_t *data, uint16_t len) {
    // TODO: 这里使用HAL库示例，请根据你的平台调整
    // HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, 
    //                                                   dev_addr << 1, 
    //                                                   data, 
    //                                                   len, 
    //                                                   HAL_MAX_DELAY);
    return true;
}

// I2C读函数
bool i2c_read(uint8_t dev_addr, uint8_t *data, uint16_t len) {
    // TODO: 这里使用HAL库示例，请根据你的平台调整
    // HAL_StatusTypeDef status = HAL_I2C_Master_Receive(&hi2c1, 
    //                                                  dev_addr << 1, 
    //                                                  data, 
    //                                                  len, 
    //                                                  HAL_MAX_DELAY);
    return true;
}

// 读取GT911寄存器
bool gt911_read_register(uint16_t reg, uint8_t *data, uint16_t len) {
    uint8_t reg_addr[2];
    
    // GT911使用16位寄存器地址，大端序
    reg_addr[0] = (reg >> 8) & 0xFF;
    reg_addr[1] = reg & 0xFF;
    
    // 先发送寄存器地址
    if (!i2c_write(GT911_I2C_ADDR1, reg_addr, 2)) {
        return false;
    }
    
    // 然后读取数据
    return i2c_read(GT911_I2C_ADDR1, data, len);
}

// 写入GT911寄存器
bool gt911_write_register(uint16_t reg, uint8_t *data, uint16_t len) {
    uint8_t buffer[2 + len];
    
    // 构建写入缓冲区：寄存器地址 + 数据
    buffer[0] = (reg >> 8) & 0xFF;
    buffer[1] = reg & 0xFF;
    memcpy(&buffer[2], data, len);
    
    return i2c_write(GT911_I2C_ADDR1, buffer, 2 + len);
}

// 清除状态寄存器
void gt911_clear_status(void) {
    uint8_t clear_cmd = 0x00;
    gt911_write_register(GT911_STATUS_REG, &clear_cmd, 1);
}

// gt911 init
esp_err_t gt911_init(gt911_conf_t *gt911_conf) {
    esp_err_t ret;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = 33,
        .scl_io_num = 32,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000
    };

    ret = i2c_param_config(gt911_conf->port, &conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C param config failed");
        return ret;
    }
    
    ret = i2c_driver_install(gt911_conf->port, conf.mode, 0, 0, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed");
        return ret;
    }
    
    gpio_set_direction(gt911_conf->irq_pin, GPIO_MODE_INPUT);

    ESP_LOGI(TAG, "GT911 initialized successfully");
    return ESP_OK;
}

// gt911 read
bool gt911_read(gt911_touch_point_t *points, uint8_t *count) {
    uint8_t status = 0;
    uint8_t touch_count = 0;
    uint8_t buffer[1 + GT911_MAX_TOUCH_POINTS * 8]; // 状态寄存器 + 触摸数据
    
    // 读取状态寄存器
    if (!gt911_read_register(GT911_STATUS_REG, &status, 1)) {
        return false;
    }
    
    // 检查是否有新的触摸数据
    if (!(status & GT911_BUFFER_READY)) {
        *count = 0;
        return true;  // 没有触摸数据，但不是错误
    }
    
    // 获取触摸点数量
    touch_count = status & GT911_POINT_COUNT_MASK;
    if (touch_count > GT911_MAX_TOUCH_POINTS) {
        touch_count = GT911_MAX_TOUCH_POINTS;
    }
    
    // 读取所有触摸点数据
    if (touch_count > 0) {
        if (!gt911_read_register(GT911_DATA_REG, buffer, 1 + touch_count * 8)) {
            return false;
        }
        
        // 解析每个触摸点
        for (int i = 0; i < touch_count; i++) {
            uint8_t* point_data = &buffer[1 + i * 8];  // 跳过状态字节
            
            // 解析坐标 (GT911使用小端字节序)
            points[i].x = (point_data[1] << 8) | point_data[0];
            points[i].y = (point_data[3] << 8) | point_data[2];
            
            // 解析其他信息
            points[i].id = (point_data[5] >> 4) & 0x0F;  // 触摸点ID
            points[i].size = point_data[6];              // 触摸面积
            points[i].pressure = point_data[7];          // 压力值
            
            // 坐标校准和边界检查
            if (points[i].x > SCREEN_WIDTH) points[i].x = SCREEN_WIDTH;
            if (points[i].y > SCREEN_HEIGHT) points[i].y = SCREEN_HEIGHT;
        }
    }
    
    *count = touch_count;
    
    // 清除状态寄存器，准备下一次读取
    gt911_clear_status();
    
    return true;
}
