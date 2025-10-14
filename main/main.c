// ===== 引入工具包 =====
// 就像做手工前要准备好剪刀、胶水一样，
// 写程序也要先告诉电脑我们需要用到哪些"工具"。

#include "freertos/FreeRTOS.h"   // 任务管理工具（让程序能同时做多件事）
#include "freertos/task.h"       // 延时工具（让程序"等一等"）
#include "esp_log.h"             // 打印工具（在屏幕上显示文字，方便我们调试）
#include "led_strip.h"           // LED 灯带工具（控制 WS2812 彩灯）

// TAG 就像是给日志消息贴的"标签"，方便我们在屏幕上找到属于自己的消息
static const char *TAG = "main";

// ===== 硬件设置 =====
// 告诉程序：彩灯插在开发板的第 21 号引脚上
// （就像告诉机器人"灯插在 21 号插座上"）
#define RGB_LED_GPIO        GPIO_NUM_21

// 每种颜色亮多久？500 毫秒 = 半秒钟
// （数 "1、2" 差不多就是半秒）
#define RGB_BLINK_PERIOD_MS 500

// 一共有 7 种颜色要轮流显示
#define COLOR_COUNT         7

// ===== 颜色定义 =====
// 用一个"小盒子"（结构体）来存放每种颜色的信息：
// r = 红色有多亮，g = 绿色有多亮，b = 蓝色有多亮
// 数值范围 0~255，0 是关掉，255 是最亮
// 我们用 127（大约一半），这样灯不会太刺眼
//
//  小知识：所有颜色都是红、绿、蓝三种光混合出来的！
//   红+绿 = 黄，绿+蓝 = 青，红+蓝 = 品红，红+绿+蓝 = 白

typedef struct {
    uint8_t r, g, b;       // 红、绿、蓝的亮度（0~255）
    const char *name;      // 颜色的名字（给人看的）
} rgb_color_t;

// 把 7 种颜色排成一排，像一个"颜色转盘"
static const rgb_color_t colors[COLOR_COUNT] = {
    {   0, 127,   0, "RED"     },    // 红色：G 通道给值（GRB 顺序，R 和 G 对调）
    { 127,   0,   0, "GREEN"   },    // 绿色：R 通道给值
    {   0,   0, 127, "BLUE"    },    // 蓝色：B 不变
    { 127, 127,   0, "YELLOW"  },    // 黄色：红+绿（对调后仍正确，因为两个都给了值）
    { 127,   0, 127, "CYAN"    },    // 青色：绿+蓝 → G=0, R=127, B=127
    {   0, 127, 127, "MAGENTA" },    // 品红：红+蓝 → G=127, R=0, B=127
    { 127, 127, 127, "WHITE"   },    // 白色：三个都给值（对调不影响）
};

// ===== 主程序 =====
// app_main 是程序的"起点"，就像赛跑的起跑线，
// 开发板一通电，就会从这里开始运行！

void app_main(void) {
    // 在屏幕上打印一条消息，告诉我们程序开始啦
    ESP_LOGI(TAG, "Starting...");

    // ----- 第 1 步：跟彩灯"打招呼"（初始化） -----
    // 就像玩新玩具前要先打开开关、装好电池一样，
    // 我们要先告诉开发板："我要用 WS2812 彩灯啦！"

    led_strip_handle_t strip;  // 创建一个"遥控器"，用来控制彩灯

    // 告诉"遥控器"彩灯的基本信息
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_LED_GPIO,   // 灯插在 21 号引脚
        .max_leds = 1,                     // 只有 1 颗灯
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,  // 颜色顺序是 绿-红-蓝（WS2812 的规定）
        .led_model = LED_MODEL_WS2812,    // 灯的型号是 WS2812
    };

    // 告诉"遥控器"通信速度（就像两个人说话要用相同的语速）
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,  // 每秒 1000 万次信号
    };

    // 正式创建"遥控器"，如果失败了程序会停下来报错
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));
    ESP_LOGI(TAG, "WS2812 LED initialized on GPIO%d", RGB_LED_GPIO);

    // ----- 第 2 步：开始让彩灯变色！ -----
    // index 就像一个"手指"，指向当前要显示的颜色
    // 0 = 红色，1 = 绿色，2 = 蓝色 ... 6 = 白色
    int index = 0;

    // while(1) 意思是"永远不停地循环"
    // 就像旋转木马，转完一圈又一圈，永远不会停
    while (1) {
        // 🎨 步骤 A：设置颜色
        // 从"颜色转盘"中取出第 index 个颜色，告诉彩灯"变成这个颜色！"
        led_strip_set_pixel(strip, 0, colors[index].r, colors[index].g, colors[index].b);
        led_strip_refresh(strip);  // 刷新！让彩灯真的亮起来

        // 在屏幕上打印当前是什么颜色
        ESP_LOGI(TAG, "RGB LED: %s", colors[index].name);

        // ⏰ 步骤 B：等半秒钟
        // 让颜色保持 500 毫秒，这样人眼才能看清楚
        // 如果不等待，颜色切换太快，眼睛就看不清啦
        vTaskDelay(pdMS_TO_TICKS(RGB_BLINK_PERIOD_MS));

        //  步骤 C：关掉彩灯
        led_strip_clear(strip);  // 把灯关掉，制造"闪烁"的效果

        // ⏰ 步骤 D：再等一小会儿（100 毫秒）
        // 黑暗中也停一下，让闪烁感更明显
        vTaskDelay(pdMS_TO_TICKS(100));

        // 👉 步骤 E：指向下一个颜色
        // index + 1 就是下一个颜色
        // % COLOR_COUNT 意思是：到了最后一个颜色后，重新从第一个开始
        // 就像数数：0,1,2,3,4,5,6, 然后又回到 0,1,2...
        index = (index + 1) % COLOR_COUNT;
    }
    // 程序会永远在这个循环里运行，彩灯就会一直变色不停 ✨
}
