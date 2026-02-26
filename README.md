# ESP32-S3 + ST7789 Arduino Demo Examples

ESP32-S3 开发板 Arduino 示例程序集，适用于 1.69 英寸 ST7789 显示屏 (240x280)。

## 硬件配置

| 引脚 | 功能 |
|------|------|
| GPIO 13 | LCD 背光 (BL) |
| GPIO 39 | LCD MOSI |
| GPIO 38 | LCD CLK |
| GPIO 10 | LCD DC |
| GPIO 3  | LCD RST |
| GPIO 14 | LCD CS |
| GPIO 0  | 按键 |
| GPIO 48 | WS2812 RGB LED |

- Flash: 16MB
- 分区方案: `app3M_fat9M_16MB`

## 示例列表

| 序号 | 名称 | 功能描述 |
|------|------|----------|
| 01 | HelloWorld | 基础屏幕测试，随机位置显示 Hello World |
| 02 | ColorTest | 黑客帝国代码雨效果 (ASCII 字符下落) |
| 03 | SimpleWatch | GFX 时钟，WiFi + NTP 时间同步 |
| 04 | GFX_WIFIAnalyzer | WiFi 信号分析器，频道图形显示 |
| 05 | GIF_Player | GIF 播放器，AP 模式网页上传 |
| 06 | LVGL_Time | LVGL 时钟界面，WiFi + NTP 同步 |
| 07 | LVGL_LED | LVGL 界面 + WS2812 RGB 控制 |

## 依赖库

### ESP32 板包自带
- `WiFi.h`
- `WebServer.h`
- `FFat.h`
- `HWCDC.h`

### 需要安装
| 库名 | 用途 | 使用的示例 |
|------|------|-----------|
| Arduino_GFX_Library | 显示驱动 | 全部 |
| lvgl (8.x) | 图形界面 | 06, 07 |
| Adafruit_NeoPixel | WS2812 驱动 | 07 |
| AnimatedGIF | GIF 解码 | 05 |

## 编译上传

```bash
arduino-cli compile --jobs 8 -b esp32:esp32:esp32s3:FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB -e -u -p COM121 .
```

## WiFi 配置

需要 WiFi 的示例 (03, 04, 06)，请修改 `pin_config.h` 中的配置：

```cpp
#define WIFI_SSID     "your_ssid"
#define WIFI_PASSWORD "your_password"
```

## 注意事项

1. LVGL 示例需要在项目目录放置 `lv_conf.h` 配置文件
2. 05_GIF_Player 使用 AP 模式，连接 "GIF_Player" 热点后访问 192.168.4.1 上传 GIF
3. 07_LVGL_LED 按键操作：单击切换颜色，双击开关 LED
