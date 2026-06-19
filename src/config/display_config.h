#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

/**
 * @file display_config.h
 * @brief 四屏拼接配置：引脚 / 象限映射 / 旋转角度
 *
 *  屏幕拓扑（数学象限规则，原点在大屏中心）：
 *
 *      ┌──────────┬──────────┐
 *      │  CS1     │   CS2    │
 *      │ 第二象限 │ 第一象限 │
 *      │ (左上)   │  (右上)  │
 *      ├──────────┼──────────┤
 *      │  CS3     │   CS4    │
 *      │ 第三象限 │ 第四象限 │
 *      │ (左下)   │  (右下)  │
 *      └──────────┴──────────┘
 *
 *  LVGL 大屏逻辑坐标系（左上为 (0,0)，右下为 (479,479)）
 */

// ---- 大屏 / 子屏分辨率 ----
#define BIG_SCREEN_W    480
#define BIG_SCREEN_H    480
#define SUB_SCREEN_W    240
#define SUB_SCREEN_H    240
#define PANEL_COUNT     4

// ---- SPI 共享总线引脚 ----
#define PIN_SPI_MOSI    18   // SDA
#define PIN_SPI_SCLK    16   // SCL
#define PIN_SPI_MISO    -1   // 未接
#define PIN_LCD_DC      17
#define PIN_LCD_RST     15
#define PIN_LCD_BL      4    // 硬件已上拉，软件可不驱动

// ---- 4 路独立 CS ----
#define PIN_LCD_CS1     38   // 左上 - 第二象限
#define PIN_LCD_CS2     39   // 右上 - 第一象限
#define PIN_LCD_CS3     2    // 左下 - 第三象限
#define PIN_LCD_CS4     1    // 右下 - 第四象限

// ---- SPI 频率 ----
// =================================================================
// 🔧 【可调节 #1】SPI 总线频率 — 遇黑屏/雪花屏先调这里！
// =================================================================
// 多屏并联走线较长 + 跳线/杭接，频率越高越容易丢包 → 黑屏/雪花屏。
// 遇不稳定时从高到低逐个尝试，取连续跑30分钟不闪烁的最高频率：
//
//   40000000UL  =  40 MHz   ⚠️ 极限、需短走线+好 PCB
//   33000000UL  =  33 MHz   ⚠️ 激进
//   27000000UL  =  27 MHz
//   20000000UL  =  20 MHz   上一版默认 (不够稳)
//   10000000UL  =  10 MHz   ✅ 当前默认，代码在这
//    8000000UL  =   8 MHz   保守
//    5000000UL  =   5 MHz   最保守，几乎不会丢包
//    4000000UL  =   4 MHz   极低（刷新很慢但稳）
//
// 修改后需重新 pio run -t upload
#define LCD_SPI_FREQ 10000000UL // ←←← 在这里调频率
// =================================================================

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
 *  offset_x / offset_y: 该子屏左上角在 480×480 逻辑大屏中的坐标
 *
 *  brightness: 亮度修正值 (1-100)，100 = 最亮。
 *      由于 4 屏共享 BL 引脚，实际 PWM 使用 4 屏中的最大值。
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
    // CS1 → 第二象限（左上） UP朝右→左旋90°→ rotation=3
    {PIN_LCD_CS1, /*rotation=*/0, /*offset_x=*/0, /*offset_y=*/0, /*brightness=*/100},
    // CS2 → 第一象限（右上） UP朝左→右旋90°→ rotation=1
    {PIN_LCD_CS2, /*rotation=*/0, /*offset_x=*/SUB_SCREEN_W, /*offset_y=*/0, /*brightness=*/100},
    // CS3 → 第三象限（左下） UP朝右→左旋90°→ rotation=3
    {PIN_LCD_CS3, /*rotation=*/0, /*offset_x=*/0, /*offset_y=*/SUB_SCREEN_H, /*brightness=*/100},
    // CS4 → 第四象限（右下） UP朝左→右旋90°→ rotation=1
    {PIN_LCD_CS4, /*rotation=*/0, /*offset_x=*/SUB_SCREEN_W, /*offset_y=*/SUB_SCREEN_H, /*brightness=*/100},
};

#endif // DISPLAY_CONFIG_H
