#include "DemoAnimation.h"
#include "config/display_config.h"
#include <Arduino.h>

// 启用 montserrat_20 字体
LV_FONT_DECLARE(lv_font_montserrat_20);

// ===== 动画对象句柄 =====
static lv_obj_t *anim_arc = nullptr;
static lv_obj_t *anim_bar1 = nullptr;
static lv_obj_t *anim_bar2 = nullptr;

void createDemoAnimation()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    // --- 1) 中心大圆弧旋转动画（位于 720x240 中心点）---
    anim_arc = lv_arc_create(scr);
    lv_obj_set_size(anim_arc, 220, 220);
    lv_obj_center(anim_arc);
    lv_arc_set_rotation(anim_arc, 0);
    lv_arc_set_bg_angles(anim_arc, 0, 360);
    lv_arc_set_angles(anim_arc, 0, 270);
    lv_obj_remove_style(anim_arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(anim_arc, 16, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(anim_arc, lv_color_hex(0x00ff88), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(anim_arc, 16, LV_PART_MAIN);
    lv_obj_set_style_arc_color(anim_arc, lv_color_hex(0x333355), LV_PART_MAIN);

    // 圆弧旋转动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, anim_arc);
    lv_anim_set_exec_cb(&a, [](void *obj, int32_t v)
                        { lv_arc_set_rotation((lv_obj_t *)obj, (uint16_t)v); });
    lv_anim_set_values(&a, 0, 360);
    lv_anim_set_duration(&a, 3000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // --- 2) 左/中/右各一个 Spinner（验证各屏独立刷新）---
    const int spinner_size = 70;
    const int margin = 20;
    struct
    {
        int x;
        int y;
        uint32_t color;
    } regions[3] = {
        {margin, margin, 0xff4444},                                   // 左屏区域
        {BIG_SCREEN_W / 2 - spinner_size / 2, margin, 0x44ff44},     // 中屏区域（水平居中）
        {BIG_SCREEN_W - margin - spinner_size, margin, 0x4444ff}     // 右屏区域
    };

    for (int i = 0; i < 3; i++)
    {
        lv_obj_t *spinner = lv_spinner_create(scr);
        lv_obj_set_size(spinner, spinner_size, spinner_size);
        lv_obj_set_pos(spinner, regions[i].x, regions[i].y);
        lv_spinner_set_anim_params(spinner, 1500, 200);
        lv_obj_set_style_arc_width(spinner, 8, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(regions[i].color), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner, 8, LV_PART_MAIN);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(0x222244), LV_PART_MAIN);
    }

    // --- 3) 水平渐变条（横跨三屏，检测拼接缝） ---
    anim_bar1 = lv_bar_create(scr);
    lv_obj_set_size(anim_bar1, BIG_SCREEN_W - 40, 16);
    lv_obj_align(anim_bar1, LV_ALIGN_CENTER, 0, -90);
    lv_bar_set_range(anim_bar1, 0, 100);
    lv_obj_set_style_bg_color(anim_bar1, lv_color_hex(0x333355), LV_PART_MAIN);
    lv_obj_set_style_bg_color(anim_bar1, lv_color_hex(0xff6600), LV_PART_INDICATOR);
    lv_obj_set_style_radius(anim_bar1, 8, LV_PART_MAIN);
    lv_obj_set_style_radius(anim_bar1, 8, LV_PART_INDICATOR);

    lv_anim_t ab;
    lv_anim_init(&ab);
    lv_anim_set_var(&ab, anim_bar1);
    lv_anim_set_exec_cb(&ab, [](void *obj, int32_t v)
                        { lv_bar_set_value((lv_obj_t *)obj, v, LV_ANIM_OFF); });
    lv_anim_set_values(&ab, 0, 100);
    lv_anim_set_duration(&ab, 2000);
    lv_anim_set_repeat_count(&ab, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_duration(&ab, 2000);
    lv_anim_start(&ab);

    // --- 4) 垂直渐变条（纵跨单屏高度，检测拼接缝） ---
    anim_bar2 = lv_bar_create(scr);
    lv_obj_set_size(anim_bar2, 16, BIG_SCREEN_H - 40);
    lv_obj_align(anim_bar2, LV_ALIGN_CENTER, -280, 0);
    lv_bar_set_range(anim_bar2, 0, 100);
    lv_obj_set_style_bg_color(anim_bar2, lv_color_hex(0x333355), LV_PART_MAIN);
    lv_obj_set_style_bg_color(anim_bar2, lv_color_hex(0x00ccff), LV_PART_INDICATOR);
    lv_obj_set_style_radius(anim_bar2, 8, LV_PART_MAIN);
    lv_obj_set_style_radius(anim_bar2, 8, LV_PART_INDICATOR);

    lv_anim_t av;
    lv_anim_init(&av);
    lv_anim_set_var(&av, anim_bar2);
    lv_anim_set_exec_cb(&av, [](void *obj, int32_t v)
                        { lv_bar_set_value((lv_obj_t *)obj, v, LV_ANIM_OFF); });
    lv_anim_set_values(&av, 0, 100);
    lv_anim_set_duration(&av, 3000);
    lv_anim_set_repeat_count(&av, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_playback_duration(&av, 3000);
    lv_anim_start(&av);

    // --- 5) 中心文字（标识坐标系） ---
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "720 x 240\nTriple Screen");
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 30);

    Serial.println("[DEMO] Triple-screen animation demo created.");
}
