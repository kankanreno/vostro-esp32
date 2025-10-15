#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "led_strip.h" // 包含LED灯带库

// 根据你的板子定义WS2812的数据引脚和LED数量
#define BLINK_GPIO 21
#define LED_NUMBERS 1

static const char *TAG = "WS2812 Example";
led_strip_handle_t led_strip;

// 任务函数：控制WS2812
static void ws2812_task(void *pvParameters) {
    while (1) {
        // 设置LED为红色 (G=0, R=7, B=0)
        ESP_LOGI(TAG, "Setting color: RED");
        led_strip_set_pixel(led_strip, 0, 7, 0, 0);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 设置LED为绿色 (G=7, R=0, B=0)
        ESP_LOGI(TAG, "Setting color: GREEN");
        led_strip_set_pixel(led_strip, 0, 0, 7, 0);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 设置LED为蓝色 (G=0, R=0, B=7)
        ESP_LOGI(TAG, "Setting color: BLUE");
        led_strip_set_pixel(led_strip, 0, 0, 0, 7);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    // LED灯带配置
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = LED_NUMBERS,
    };

    // RMT后端配置
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .mem_block_symbols = 64,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    // 创建FreeRTOS任务来控制WS2812
    xTaskCreate(ws2812_task,   // 任务函数
                "ws2812_task", // 任务名称
                2048,          // 堆栈大小
                NULL,          // 任务参数
                5,             // 任务优先级
                NULL);         // 任务句柄

    ESP_LOGI(TAG, "WS2812 task created");
}
