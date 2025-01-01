#ifndef LVGLSERVICE_H
#define LVGLSERVICE_H

#include <lvgl.h>
#include <TFT_eSPI.h>

#define DRAW_BUF_SIZE (TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];
#define TFT_ROTATION LV_DISPLAY_ROTATION_0

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

        /*Set a tick source so that LVGL will know how much time elapsed. */
        lv_tick_set_cb(my_tick);

        lv_display_t *disp;
        /*TFT_eSPI can be enabled lv_conf.h to initialize the display in a simple way*/
        disp = lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, sizeof(draw_buf));
        lv_display_set_rotation(disp, TFT_ROTATION);

        Serial.println("Setup done");
    }

    void loop()
    {
        lv_timer_handler(); /* let the GUI do its work */
    }

private:
    LVGLService() {}
    LVGLService(const LVGLService &) = delete;
    LVGLService &operator=(const LVGLService &) = delete;

    static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
    {
        lv_display_flush_ready(disp);
    }

    static uint32_t my_tick(void)
    {
        return millis();
    }
};

#endif // LVGLSERVICE_H
