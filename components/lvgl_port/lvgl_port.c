#include "lvgl_port.h"
#include "st7796.h"
#include "lvgl.h"

static st7796_conf_t st7796_conf = {
    .dc_pin = 12,
    .rst_pin = 11,
    .cs_pin = 10,
    .width = 320,
    .height = 480
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

void lvgl_port_init(void) {    
    // 初始化显示驱动
    st7796_init(&st7796_conf);
    
    // 初始化LVGL
    lv_init();
    
    // 创建显示设备
    lv_display_t * disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);
}
