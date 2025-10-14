# Vostro ESP32-S3 RGB LED

基于 ESP32-S3 的 RGB LED 交替闪烁示例程序。

## 硬件平台

- **开发板**: ESP32-S3-DEV-KIT-N8R8
- **LED 引脚**: GPIO38（RGB LED）
- **目标芯片**: ESP32-S3

## 功能说明

程序通过 GPIO 控制 RGB LED，循环切换 7 种颜色：

| 顺序 | 颜色 |
|------|------|
| 1 |  红色 (RED) |
| 2 | 🟢 绿色 (GREEN) |
| 3 | 🔵 蓝色 (BLUE) |
| 4 | 🟡 黄色 (YELLOW) |
| 5 | 🩵 青色 (CYAN) |
| 6 | 🟣 品红 (MAGENTA) |
| 7 | ⚪ 白色 (WHITE) |

每种颜色点亮 500ms，熄灭 100ms 后切换到下一种颜色。

## 项目结构

vostro-esp32/ 
├── CMakeLists.txt # 项目根构建配置 
├── main/ 
│ ├── CMakeLists.txt # 组件构建配置 
│ └── main.c # 主程序（RGB LED 闪烁）
└── README.md # 项目说明

## 编译与烧录

```bash
# 设置目标芯片
idf.py set-target esp32s3

# 编译
idf.py build

# 烧录并打开串口监视器（替换 PORT 为实际串口）
idf.py -p PORT flash monitor
```

依赖
ESP-IDF v5.x
FreeRTOS（ESP-IDF 内置）
GPIO 驱动（ESP-IDF 内置）
无需任何外部组件依赖。
