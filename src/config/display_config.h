#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

/**
 * @file display_config.h
 * @brief 三屏横排拼接配置：引脚 / 位置映射 / 旋转角度
 *
 *  屏幕拓扑（三屏水平并排，从左到右）：
 *
 *      ┌──────────┬──────────┬──────────┐
 *      │   CS1    │   CS2    │   CS3    │
 *      │   左屏   │   中屏   │   右屏   │
 *      │  (0,0)   │ (240,0)  │ (480,0)  │
 *      └──────────┴──────────┴──────────┘
 *
 *  LVGL 大屏逻辑坐标系（左上为 (0,0)，右下为 (719,239)）
 */

// ---- 大屏 / 子屏分辨率 ----
#define BIG_SCREEN_W 720 // 3 × 240 = 720
#define BIG_SCREEN_H 240
#define SUB_SCREEN_W    240
#define SUB_SCREEN_H    240
#define PANEL_COUNT 3

// ---- SPI 共享总线引脚 ----
#define PIN_SPI_MOSI    18   // SDA
#define PIN_SPI_SCLK    16   // SCL
#define PIN_SPI_MISO    -1   // 未接
#define PIN_LCD_DC      17
#define PIN_LCD_RST     15
#define PIN_LCD_BL      4    // 硬件已上拉，软件可不驱动

// ---- 3 路独立 CS ----
#define PIN_LCD_CS1 38 // 左屏
#define PIN_LCD_CS2 39 // 中屏
#define PIN_LCD_CS3 2  // 右屏

// ---- SPI 频率 ----
// =================================================================
// 🔧 【可调节 #1】SPI 总线频率 — 遇黑屏/雪花屏先调这里！
// =================================================================
// 硬件极限：ESP32-S3 APB=80MHz，SPI 时钟最大 = APB/1 = 80MHz
// 有效分频：80M / 40M / 26.6M / 20M / 16M / 10M / 8M / 5M / 4M
// 实际瓶颈：多屏跳线走线的信号完整性，而非芯片本身
//
//   80000000UL  =  80 MHz   🔥 硬件极限，需优质走线+屏蔽
//   60000000UL  =  60 MHz   ⚠️ 非标分频，可能回退到 40MHz
//   40000000UL  =  40 MHz   ⚠️ 跳线极限，需短走线
//   33000000UL  =  33 MHz   ⚠️ 激进
//   27000000UL  =  27 MHz
//   20000000UL  =  20 MHz   中等
//   10000000UL  =  10 MHz   稳定
//    8000000UL  =   8 MHz   保守
//    5000000UL  =   5 MHz   最保守，几乎不会丢包
//    4000000UL  =   4 MHz   极低（刷新很慢但稳）
//
// 推荐通过 platformio.ini 的 -DLCD_SPI_FREQ_HZ=xxx 来配置，
// 如果未设置则使用下方默认值。修改后需重新 pio run -t upload
// =================================================================
#ifndef LCD_SPI_FREQ_HZ
#define LCD_SPI_FREQ_HZ 40000000UL
#endif
#define LCD_SPI_FREQ LCD_SPI_FREQ_HZ

// ---- 背光 PWM 配置 ----
#define BL_PWM_CHANNEL  0
#define BL_PWM_FREQ     5000     // 5kHz PWM
#define BL_PWM_RESOLUTION 8     // 8-bit (0-255)

/**
 * @brief 单块子屏配置项
 *  rotation: LovyanGFX setRotation() 入参，取值 0~3
 *      0 = 0°    1 = 90°    2 = 180°    3 = 270°
 *      4~7 为镜像模式
 *
 *  offset_x / offset_y: 该子屏左上角在 720×240 逻辑大屏中的坐标
 *
 *  brightness: 亮度修正值 (1-100)，100 = 最亮。
 *      由于 3 屏共享 BL 引脚，实际 PWM 使用 3 屏中的最大值。
 *      差值通过软件颜色缩放补偿（保证各屏亮度一致）。
 *
 *  ⚠️ rotation 需要点亮后根据实际显示效果调整
 */
struct PanelCfg {
    int      cs_pin;
    uint8_t  rotation;     // 先给默认值 0，运行后按需要改
    uint16_t offset_x;
    uint16_t offset_y;
    uint8_t  brightness;   // 1~100，默认 100
};

static const PanelCfg kPanelCfg[PANEL_COUNT] = {
    // CS1 → 左屏  rotation 根据实测调整
    {PIN_LCD_CS1, /*rotation=*/0, /*offset_x=*/0, /*offset_y=*/0, /*brightness=*/100},
    // CS2 → 中屏  rotation 根据实测调整
    {PIN_LCD_CS2, /*rotation=*/0, /*offset_x=*/SUB_SCREEN_W, /*offset_y=*/0, /*brightness=*/100},
    // CS3 → 右屏  rotation 根据实测调整
    {PIN_LCD_CS3, /*rotation=*/0, /*offset_x=*/SUB_SCREEN_W * 2, /*offset_y=*/0, /*brightness=*/100},
};

#endif // DISPLAY_CONFIG_H
