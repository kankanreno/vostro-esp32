#include "lvgl_port.h"
#include "st7796s.h"
#include "gt911.h"
#include "lvgl.h"

static st7796s_conf_t st7796s_conf = {
    .dc_pin = 12,
    .rst_pin = 11,
    .cs_pin = 10,
    .width = 320,
    .height = 480
};

static gt911_conf_t gt911_conf = {
    .port = I2C_NUM_0,
    .irq_pin = 0x5D
};

// void disp_flush(lv_display_t *drv, const lv_area_t *area, uint8_t *color_map) {
//     spi_device_handle_t *spi = drv->user_data;
    
//     // 设置显示区域
//     uint8_t col_addr[] = {area->x1, area->x2};
//     uint8_t page_addr[] = {area->y1, area->y2};
    
//     send_cmd(spi, 0x2A);  // Column address
//     send_data(spi, col_addr, 2);
    
//     send_cmd(spi, 0x2B);  // Page address
//     send_data(spi, page_addr, 2);
    
//     send_cmd(spi, 0x2C);  // Memory write
//     send_data(spi, color_map, lv_area_get_size(area) * 2);
    
//     lv_disp_flush_ready(drv);
// }

static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map) {
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    int32_t x;
    int32_t y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            /*Put a pixel to the display. For example:*/
            /*put_px(x, y, *px_map)*/
            px_map++;
        }
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}

static void touch_read(lv_indev_t *indev_drv, lv_indev_data_t *data) {
    // 静态变量用于存储上一次的坐标，防止无触摸事件时数据未定义
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    gt911_touch_point_t points[GT911_MAX_TOUCH_POINTS];
    uint8_t touch_count = 0;

    // 读取当前触摸状态和坐标，例如通过I2C读取GT911寄存器
    if (gt911_read(points, &touch_count) && touch_count > 0) {
        // 有触摸点，设置状态为按压，并传入第一个触摸点的坐标（LVGL默认处理单点）
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.x = points[0].x; // 确保坐标与屏幕方向匹配
        data->point.y = points[0].y;

        // 更新静态变量，以便无触摸时返回最后一个有效坐标
        last_x = points[0].x;
        last_y = points[0].y;
    } else {
        // 没有检测到触摸，设置状态为释放
        // LVGL在释放状态时仍需要有效的坐标，通常返回最后一次按压的坐标
        data->state = LV_INDEV_STATE_RELEASED;
        data->point.x = last_x;
        data->point.y = last_y;
    }

    // 告知LVGL没有更多数据要读取了（非缓冲模式）
    // 如果你的驱动使用缓冲模式并还有数据，可设置为true，但GT911通常不需要
    // data->continue_reading = false;
}

void lvgl_port_init(void) {    
    // 初始化显示驱动
    st7796s_init(&st7796s_conf);
    
    // 初始化触摸驱动
    gt911_init(&gt911_conf);
    
    // 初始化LVGL
    lv_init();
    
    // 创建显示设备
    lv_display_t * disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);
    
    // 创建输入设备
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, touch_read);
}
