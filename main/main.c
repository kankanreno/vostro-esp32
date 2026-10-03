// ============================================================
//  ESP32-S3 温湿度计
//  功能：每 2 秒读取 DHT22 传感器的温度和湿度，
//        读取时板载绿灯闪一下作为提示
//
//  硬件接线：
//    WS2812 彩灯 → GPIO21（板载，不用接线）
//    DHT22 模块  → + 接 3.3V，OUT 接 GPIO2，- 接 GND
// ============================================================

// ===== 第 1 步：准备工具包 =====
// 写程序就像做手工，先要把工具准备好。
// 每一行 #include 就是告诉电脑："我要用这个工具！"

#include "freertos/FreeRTOS.h"   // 任务管理（让程序能"等一等"）
#include "freertos/task.h"       // 延时工具（vTaskDelay）
#include "esp_log.h"             // 打印工具（在屏幕上显示文字）
#include "esp_rom_sys.h"         // 精确延时工具（微秒级等待）
#include "esp_timer.h"           // 硬件计时器（精确计时，像秒表一样）
#include "driver/gpio.h"         // GPIO 工具（控制引脚的高低电平）
#include "led_strip.h"           // LED 灯带工具（控制 WS2812 彩灯）

// TAG 是日志的"标签"，方便在一堆文字中找到我们自己的消息
static const char *TAG = "main";

// ===== 第 2 步：告诉程序硬件接在哪里 =====

// WS2812 彩灯插在开发板的 21 号引脚（板载，出厂就焊好了）
#define RGB_LED_GPIO        GPIO_NUM_21

// DHT22 温湿度传感器的数据线插在 2 号引脚
// （就像告诉机器人："传感器插在 2 号插座上"）
#define DHT22_GPIO          GPIO_NUM_2

// 每 2000 毫秒（2 秒）读一次温湿度
// （DHT22 比较"慢热"，问太快它会不理你）
#define DHT22_PERIOD_MS     2000
#define LED_FLASH_MS        100    // 所有颜色闪烁的统一时长

// ===== 第 3 步：写一个"秒表"工具 =====
// esp_timer_get_time() 是 ESP32 内置的硬件秒表，
// 它返回从开机到现在过了多少微秒（1 秒 = 100 万微秒）
// 我们把它包成一个简单的函数，方便后面使用

static inline uint32_t now_us(void) {
    return (uint32_t)esp_timer_get_time();
}

// ===== 第 4 步：DHT22 通信工具函数 =====

// 等待引脚变成指定的电平（高或低），如果超时还没变就返回 false
// 就像等朋友回消息，等了太久就不等了
static bool dht_wait_level(gpio_num_t pin, bool level, int timeout_us) {
    uint32_t start = now_us();          // 按下秒表，开始计时
    while (gpio_get_level(pin) != level) {  // 引脚还没变成想要的电平？
        if (now_us() - start > (uint32_t)timeout_us) return false;  // 超时了，不等了
    }
    return true;  // 等到了！
}

// ===== 第 5 步：读取 DHT22 温湿度的核心函数 =====
//
// DHT22 通信就像两个人打电报：
//   1. ESP32 说："我要数据！"（拉低引脚 1.1 毫秒）
//   2. DHT22 说："收到！"（拉低 80 微秒，再拉高 80 微秒）
//   3. DHT22 开始发数据：40 个 bit（0 或 1）
//      - 每个 bit 先拉低 50 微秒
//      - 然后拉高：短（26 微秒）= 0，长（70 微秒）= 1
//   4. ESP32 把 40 个 bit 拼成 5 个字节：
//      湿度高8位 + 湿度低8位 + 温度高8位 + 温度低8位 + 校验和
//
//  怎么判断 0 还是 1？
//   测量整个 bit 的时长：
//   0 bit = 50 + 26 = 76 微秒
//   1 bit = 50 + 70 = 120 微秒
//   超过 90 微秒就是 1，否则是 0

static esp_err_t dht22_read(gpio_num_t pin, float *humidity, float *temperature) {
    uint8_t data[5] = {0};  // 5 个字节的"收件箱"，先全部填 0

    // --- 第 1 阶段：ESP32 发送起始信号 ---
    // 把引脚设为输出模式，拉低 1.1 毫秒，告诉传感器"我要数据！"
    gpio_set_direction(pin, GPIO_MODE_OUTPUT);
    gpio_set_level(pin, 0);             // 拉低（说"喂！"）
    esp_rom_delay_us(1100);             // 等 1.1 毫秒
    gpio_set_level(pin, 1);             // 拉高（松开）
    esp_rom_delay_us(30);               // 等 30 微秒

    // --- 第 2 阶段：等待传感器应答 ---
    // 切换为输入模式，听传感器说话
    gpio_set_direction(pin, GPIO_MODE_INPUT);

    // 传感器应该先拉低 80 微秒，再拉高 80 微秒（说"收到！"）
    if (!dht_wait_level(pin, 0, 100)) return ESP_ERR_TIMEOUT;  // 等它拉低
    if (!dht_wait_level(pin, 1, 100)) return ESP_ERR_TIMEOUT;  // 等它拉高

    // --- 第 3 阶段：读取 40 个 bit 的数据 ---
    for (int i = 0; i < 40; i++) {
        // 每个 bit 都以"拉低 50 微秒"开始，先等它拉低
        if (!dht_wait_level(pin, 0, 100)) return ESP_ERR_TIMEOUT;

        // 按下秒表！测量从这个 bit 开始到下一个 bit 开始的总时长
        uint32_t start = now_us();
        if (!dht_wait_level(pin, 1, 100)) return ESP_ERR_TIMEOUT;  // 等拉高
        if (!dht_wait_level(pin, 0, 100)) return ESP_ERR_TIMEOUT;  // 等下一个 bit 的拉低
        int period_us = (int)(now_us() - start);  // 总时长

        // 把数据放进"收件箱"
        data[i / 8] <<= 1;          // 先左移一位（腾出最右边的位置）
        if (period_us > 90) {       // 总时长 > 90 微秒 = 1 bit（~120us）
            data[i / 8] |= 1;       // 最右边填 1
        }
        // 否则总时长 <= 90 微秒 = 0 bit（~76us），最右边保持 0
    }

    // --- 第 4 阶段：校验数据 ---
    // 前 4 个字节加起来，应该等于第 5 个字节（校验和）
    // 就像快递单上的"合计"，对不上就说明数据传输出错了
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) return ESP_ERR_INVALID_CRC;

    // --- 第 5 阶段：把原始数据变成温度和湿度 ---
    // 湿度 = (第1字节 << 8 | 第2字节) / 10.0
    // 温度 = (第3字节 << 8 | 第4字节) / 10.0
    // （DHT22 的精度是 0.1，所以要除以 10）
    uint16_t raw_h = ((uint16_t)data[0] << 8) | data[1];  // 湿度（无符号，不会是负数）
    int16_t  raw_t = ((uint16_t)data[2] << 8) | data[3];  // 温度（有符号，冬天可以是负数）

    *humidity    = raw_h / 10.0f;   // 比如 603 → 60.3%
    *temperature = raw_t / 10.0f;   // 比如 285 → 28.5°C

    return ESP_OK;  // 成功！
}

// ===== 第 6 步：主程序（程序的起点）=====
// 开发板一通电，就从这里开始运行！

void app_main(void) {
    ESP_LOGI(TAG, "Starting...");

    // ----- 初始化 WS2812 彩灯 -----
    // 就像玩新玩具前要先"打开开关"

    led_strip_handle_t strip;  // 创建一个"遥控器"

    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_LED_GPIO,   // 灯在 21 号引脚
        .max_leds = 1,                     // 只有 1 颗灯
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,  // WS2812 的颜色顺序是 绿-红-蓝
        .led_model = LED_MODEL_WS2812,    // 灯的型号
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,  // 通信速度：每秒 1000 万次
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &strip));
    ESP_LOGI(TAG, "WS2812 LED initialized on GPIO%d", RGB_LED_GPIO);

    // ----- 初始化 DHT22 的引脚 -----
    // 设置 2 号引脚为输出模式，并打开内部上拉电阻
    // （上拉电阻就像一根"弹簧"，默认把引脚拉到高电平）
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << DHT22_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    // ----- 主循环：永远不停地读取温湿度 -----
    while (1) {
        // ️ 读取温湿度
        float humidity, temperature;
        esp_err_t ret = dht22_read(DHT22_GPIO, &humidity, &temperature);
        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "DHT22 -> 温度: %.1f °C, 湿度: %.1f %%", temperature, humidity);

            if (temperature >= 32.0f) {
                ESP_LOGW(TAG, "️ 温度过高: %.1f °C！", temperature);
                led_strip_set_pixel(strip, 0, 60, 60, 0);  // 黄色
                led_strip_refresh(strip);
                vTaskDelay(pdMS_TO_TICKS(LED_FLASH_MS));
            } else {
                led_strip_set_pixel(strip, 0, 60, 0, 0);  // 绿色
                led_strip_refresh(strip);
                vTaskDelay(pdMS_TO_TICKS(LED_FLASH_MS));
            }
        } else {
            ESP_LOGE(TAG, "DHT22 读取失败 (err=0x%x)，请检查接线", ret);
            // 闪红灯表示失败
            led_strip_set_pixel(strip, 0, 0, 60, 0);  // 红色
            led_strip_refresh(strip);
            vTaskDelay(pdMS_TO_TICKS(LED_FLASH_MS));
        }

        // 关掉彩灯
        led_strip_clear(strip);

        // ⏰ 等 2 秒再读下一次
        vTaskDelay(pdMS_TO_TICKS(DHT22_PERIOD_MS));
    }
    // 程序会永远在这个循环里运行
}
