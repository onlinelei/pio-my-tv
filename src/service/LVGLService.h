#ifndef LVGLSERVICE_H
#define LVGLSERVICE_H

#include <lvgl.h>
#include "ui/ui.h"

class LVGLService
{
private:
    // 显示相关配置
    static constexpr uint32_t DRAW_BUF_SIZE = (TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8));
    static constexpr uint8_t TFT_ROTATION = LV_DISPLAY_ROTATION_0;

    // 显示缓冲区
    uint32_t draw_buf[DRAW_BUF_SIZE / 4];

    // 私有构造函数（单例模式）
    LVGLService() = default;

    // 禁用拷贝构造和赋值操作
    LVGLService(const LVGLService &) = delete;
    LVGLService &operator=(const LVGLService &) = delete;

public:
    // 单例模式获取实例
    static LVGLService &getInstance()
    {
        static LVGLService instance;
        return instance;
    }

    // LVGL初始化设置
    void setup()
    {
        // 打印LVGL版本信息
        String LVGL_Arduino = "Hello Arduino! ";
        LVGL_Arduino += String('V') + lv_version_major() + "." +
                        lv_version_minor() + "." + lv_version_patch();
        Serial.println(LVGL_Arduino);

        // 初始化LVGL
        lv_init();
        lv_tick_set_cb(my_tick);

        // 创建并配置显示设备
        lv_display_t *disp = lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, sizeof(draw_buf));
        lv_display_set_rotation(disp, (lv_display_rotation_t)TFT_ROTATION);

        // 初始化UI
        ui_init();
        Serial.println("Setup done");
    }

    // LVGL主循环
    void loop()
    {
        ui_tick();          // 更新UI状态
        lv_timer_handler(); // 处理LVGL的定时任务
    }

private:
    // 显示刷新回调函数
    static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
    {
        lv_display_flush_ready(disp);
    }

    // LVGL时钟回调函数
    static uint32_t my_tick(void)
    {
        return millis();
    }
};

#endif // LVGLSERVICE_H
