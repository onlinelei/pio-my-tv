#ifndef QUAD_PANEL_H
#define QUAD_PANEL_H

#include <Arduino.h>
#include <LovyanGFX.hpp>
#include <lvgl.h>
#include "config/display_config.h"

/**
 * @brief 单块子屏（继承 LovyanGFX 的 LGFX_Device）
 *
 * 3 块子屏共用 SPI host (SPI2_HOST)，通过 bus_shared = true 实现仲裁。
 * 每屏拥有独立 CS 引脚和可配置旋转角度。
 */
class LGFX_SubPanel : public lgfx::LGFX_Device {
public:
    LGFX_SubPanel(int cs_pin, bool useRst = false);
    void applyRotation(uint8_t rotation);

private:
    lgfx::Bus_SPI       _bus;
    lgfx::Panel_ST7789  _panel;
};

/**
 * @brief 三屏统一管理器（单例）
 *
 * 负责：
 * - 初始化 3 块 ST7789 子屏（水平并排）
 * - PWM 背光亮度控制 + 软件亮度补偿
 * - 提供 LVGL flush 回调（按脏区域自动派发到对应子屏）
 * - 运行时旋转参数调整
 */
class QuadPanel {
public:
    static QuadPanel& getInstance() {
        static QuadPanel instance;
        return instance;
    }

    /** 初始化所有屏幕（含背光、旋转、清屏、方向标识） */
    void init();

    /** 内容就绪后点亮背光（解决开机雪花屏） */
    void turnOnBacklight();

    /** 提供给 LVGL 的 flush 回调（static，内部通过单例转发） */
    static void lvglFlushCb(lv_display_t* disp,
                            const lv_area_t* area,
                            uint8_t* px_map);

    /** 运行时更改某块屏旋转角度（调试用） */
    void setRotation(uint8_t panelIdx, uint8_t rotation);

    /** 获取子屏 LovyanGFX 设备指针（高级用途） */
    LGFX_SubPanel* getPanel(uint8_t idx);

    /** 设置背光亮度 (1-100) */
    void setBrightness(uint8_t value);

private:
    QuadPanel() = default;
    QuadPanel(const QuadPanel&) = delete;
    QuadPanel& operator=(const QuadPanel&) = delete;

    /** 将 LVGL 脏区域映射到各子屏并推送像素 */
    void flushArea(const lv_area_t* area, uint8_t* px_map);

    /** 对像素应用亮度补偿（当 panel brightness < maxBrightness） */
    void applyBrightnessScale(uint16_t* pixels, uint32_t count, uint8_t brightness);

    LGFX_SubPanel* _panels[PANEL_COUNT] = { nullptr };
    bool           _inited = false;
    uint8_t _maxBrightness = 100; // 3 屏中最大亮度值
};

#endif // QUAD_PANEL_H
