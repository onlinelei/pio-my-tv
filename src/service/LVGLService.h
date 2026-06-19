#ifndef LVGLSERVICE_H
#define LVGLSERVICE_H

#include <lvgl.h>
#include <esp_heap_caps.h>
#include "config/display_config.h"
#include "driver/QuadPanel.h"

// 启用 montserrat_20 字体（动画展示用）
LV_FONT_DECLARE(lv_font_montserrat_20);

class LVGLService
{
public:
    static LVGLService &getInstance()
    {
        static LVGLService instance;
        return instance;
    }

    void setup()
    {
        String LVGL_Arduino = "Hello Arduino! ";
        LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
        Serial.println(LVGL_Arduino);

        lv_init();
        lv_tick_set_cb(my_tick);

        // 1) 先初始化 3 屏硬件（LovyanGFX）
        QuadPanel::getInstance().init();

        // 2) 创建 LVGL 逻辑显示 720x240（三屏横排）
        lv_display_t *disp = lv_display_create(BIG_SCREEN_W, BIG_SCREEN_H);
        lv_display_set_flush_cb(disp, QuadPanel::lvglFlushCb);

        // 3) Partial render 模式：分配两块缓冲区，优先 PSRAM
        //    硬件极限：240 行 = 整屏高度（Full Buffer 模式）
        //    PSRAM 占用：buf = SCREEN_W × BUF_LINES × 2 bytes × 2块(双缓冲)
        //    240=全屏(691KB)  120=半屏(346KB)  60=默认(173KB)
        //    通过 platformio.ini 的 -DLVGL_BUF_LINES=xxx 配置
#ifndef LVGL_BUF_LINES
#define LVGL_BUF_LINES 60
#endif
        const size_t buf_pixels = BIG_SCREEN_W * LVGL_BUF_LINES;
        const size_t buf_bytes = buf_pixels * sizeof(lv_color_t);

        lv_color_t *buf1 = (lv_color_t *)heap_caps_malloc(buf_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        lv_color_t *buf2 = (lv_color_t *)heap_caps_malloc(buf_bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

        if (!buf1 || !buf2)
        {
            Serial.println("[LVGL] PSRAM alloc failed! Falling back to internal RAM.");
            if (buf1)
            {
                free(buf1);
                buf1 = nullptr;
            }
            if (buf2)
            {
                free(buf2);
                buf2 = nullptr;
            }
            buf1 = (lv_color_t *)heap_caps_malloc(buf_bytes, MALLOC_CAP_8BIT);
            buf2 = (lv_color_t *)heap_caps_malloc(buf_bytes, MALLOC_CAP_8BIT);
        }

        if (!buf1)
        {
            Serial.println("[LVGL] FATAL: Cannot allocate display buffer!");
            return;
        }

        lv_display_set_buffers(disp, buf1, buf2, buf_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
        lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_0);

        Serial.printf("[LVGL] Display created: %dx%d, buf=%uKB x2, PSRAM=%s\n",
                      BIG_SCREEN_W, BIG_SCREEN_H,
                      (unsigned)(buf_bytes / 1024),
                      heap_caps_get_free_size(MALLOC_CAP_SPIRAM) > 0 ? "YES" : "NO");

        Serial.println("[LVGL] Setup done.");
    }

    void loop()
    {
        lv_timer_handler();
    }

private:
    LVGLService() {}
    LVGLService(const LVGLService &) = delete;
    LVGLService &operator=(const LVGLService &) = delete;

    static uint32_t my_tick(void)
    {
        return millis();
    }
};

#endif // LVGLSERVICE_H
