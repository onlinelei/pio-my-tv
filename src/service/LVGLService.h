#ifndef LVGLSERVICE_H
#define LVGLSERVICE_H

#include <lvgl.h>
#include "../driver/TFT_Singleton.h"

class LVGLService
{
public:
    static LVGLService &getInstance()
    {
        static LVGLService instance;
        return instance;
    }

    void init()
    {
        lv_init();
        String LVGL_Arduino = "Hello Arduino! ";
        LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();

        Serial.println(LVGL_Arduino);
        Serial.println("I am LVGL_Arduino");

        TFT_eSPI &tft = TFT_Singleton::getInstance().getTFT();
        tft.begin();        /* TFT init */
        tft.setRotation(4); /* Landscape orientation, flipped */

        lv_disp_draw_buf_init(&draw_buf, buf, NULL, TFT_WIDTH * TFT_HEIGHT / 10);

        /*Initialize the display*/
        static lv_disp_drv_t disp_drv;
        lv_disp_drv_init(&disp_drv);
        /*Change the following line to your display resolution*/
        disp_drv.hor_res = TFT_WIDTH;
        disp_drv.ver_res = TFT_HEIGHT;
        disp_drv.flush_cb = my_disp_flush;
        disp_drv.draw_buf = &draw_buf;
        lv_disp_drv_register(&disp_drv);

        /*Initialize the (dummy) input device driver*/
        static lv_indev_drv_t indev_drv;
        lv_indev_drv_init(&indev_drv);
        indev_drv.type = LV_INDEV_TYPE_POINTER;
        lv_indev_drv_register(&indev_drv);

        lv_obj_t *label = lv_label_create(lv_scr_act());
        lv_label_set_text(label, "Hello Ardino and LVGL!");
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    }

    void push()
    {
        lv_timer_handler();
    }

private:
    LVGLService() {}
    ~LVGLService() {}
    LVGLService(const LVGLService &) = delete;
    LVGLService &operator=(const LVGLService &) = delete;

    static void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
    {
        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);

        TFT_eSPI &tft = TFT_Singleton::getInstance().getTFT();
        tft.startWrite();
        tft.setAddrWindow(area->x1, area->y1, w, h);
        tft.pushColors((uint16_t *)&color_p->full, w * h, true);
        tft.endWrite();

        lv_disp_flush_ready(disp_drv);
    }

    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf[TFT_WIDTH * TFT_HEIGHT / 10];
};

lv_disp_draw_buf_t LVGLService::draw_buf;
lv_color_t LVGLService::buf[TFT_WIDTH * TFT_HEIGHT / 10];

#endif // LVGLSERVICE_H
