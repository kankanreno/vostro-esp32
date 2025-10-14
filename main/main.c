#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"
#include "lvgl_port.h"

// 修正函数名并添加实现
void lv_task_entry(void *pvParameter) {
    while(1) {
        lv_timer_handler();  // 正确调用LVGL任务处理器
        vTaskDelay(pdMS_TO_TICKS(5));  // 保持5ms调用间隔
    }
}

void app_main(void) {
    // 硬件初始化
    lvgl_port_init();

    // 启动LVGL任务
    xTaskCreate(lv_task_entry, "lv_task", 4096, NULL, 5, NULL);
}
