#include <Arduino.h>
#include "service/LVGLService.h"
#include "driver/WiFi_Singleton.h"
#include "service/MemoryMonitor.h"

MemoryMonitor memoryMonitor;

// ===== LVGL 动画演示 =====
static lv_obj_t *anim_arc = nullptr;
static lv_obj_t *anim_bar1 = nullptr;
static lv_obj_t *anim_bar2 = nullptr;
static lv_obj_t *anim_spinner = nullptr;
static lv_obj_t *gradient_obj = nullptr;

/**
 * 创建跨全屏的动画效果，用于观察屏幕素质和拼接效果
 */
void createDemoAnimation()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    // --- 1) 中心大圆弧旋转动画（跨越4个屏幕中心交汇处）---
    anim_arc = lv_arc_create(scr);
    lv_obj_set_size(anim_arc, 300, 300);
    lv_obj_center(anim_arc);
    lv_arc_set_rotation(anim_arc, 0);
    lv_arc_set_bg_angles(anim_arc, 0, 360);
    lv_arc_set_angles(anim_arc, 0, 270);
    lv_obj_remove_style(anim_arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(anim_arc, 20, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(anim_arc, lv_color_hex(0x00ff88), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(anim_arc, 20, LV_PART_MAIN);
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

    // --- 2) 四角各一个小 Spinner（验证各屏独立刷新）---
    const int spinner_size = 80;
    const int margin = 40;
    struct
    {
        int x;
        int y;
        uint32_t color;
    } corners[4] = {
        {margin, margin, 0xff4444},                                          // 左上
        {480 - margin - spinner_size, margin, 0x44ff44},                     // 右上
        {margin, 480 - margin - spinner_size, 0x4444ff},                     // 左下
        {480 - margin - spinner_size, 480 - margin - spinner_size, 0xffff44} // 右下
    };

    for (int i = 0; i < 4; i++)
    {
        lv_obj_t *spinner = lv_spinner_create(scr);
        lv_obj_set_size(spinner, spinner_size, spinner_size);
        lv_obj_set_pos(spinner, corners[i].x, corners[i].y);
        lv_spinner_set_anim_params(spinner, 1500, 200);
        lv_obj_set_style_arc_width(spinner, 8, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(corners[i].color), LV_PART_INDICATOR);
        lv_obj_set_style_arc_width(spinner, 8, LV_PART_MAIN);
        lv_obj_set_style_arc_color(spinner, lv_color_hex(0x222244), LV_PART_MAIN);
    }

    // --- 3) 水平渐变条（横跨左右两屏，检测拼接缝） ---
    anim_bar1 = lv_bar_create(scr);
    lv_obj_set_size(anim_bar1, 440, 20);
    lv_obj_align(anim_bar1, LV_ALIGN_CENTER, 0, -180);
    lv_bar_set_range(anim_bar1, 0, 100);
    lv_obj_set_style_bg_color(anim_bar1, lv_color_hex(0x333355), LV_PART_MAIN);
    lv_obj_set_style_bg_color(anim_bar1, lv_color_hex(0xff6600), LV_PART_INDICATOR);
    lv_obj_set_style_radius(anim_bar1, 10, LV_PART_MAIN);
    lv_obj_set_style_radius(anim_bar1, 10, LV_PART_INDICATOR);

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

    // --- 4) 垂直渐变条（纵跨上下两屏，检测拼接缝） ---
    anim_bar2 = lv_bar_create(scr);
    lv_obj_set_size(anim_bar2, 20, 440);
    lv_obj_align(anim_bar2, LV_ALIGN_CENTER, -180, 0);
    lv_bar_set_range(anim_bar2, 0, 100);
    lv_obj_set_style_bg_color(anim_bar2, lv_color_hex(0x333355), LV_PART_MAIN);
    lv_obj_set_style_bg_color(anim_bar2, lv_color_hex(0x00ccff), LV_PART_INDICATOR);
    lv_obj_set_style_radius(anim_bar2, 10, LV_PART_MAIN);
    lv_obj_set_style_radius(anim_bar2, 10, LV_PART_INDICATOR);

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
    lv_label_set_text(label, "480 x 480\nQuad Screen");
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);

    Serial.println("[DEMO] Animation demo created.");
}

void setup()
{
    Serial.begin(115200);
    delay(300);

    Serial.println("\n[BOOT] feature/ray/4_screen starting...");

    // 初始化 LVGL + 四屏
    LVGLService::getInstance().setup();

    // 3 秒后显示动画（先让用户看到方向标识）
    Serial.println("[BOOT] Direction labels shown. Animation starts in 3s...");
    delay(3000);

    // 创建动画演示
    createDemoAnimation();

    Serial.println("Initial Memory Usage:");
    memoryMonitor.printMemoryUsage();

    // WiFi 初始化
    WiFi_Singleton::getInstance().init();

    Serial.println("Memory Usage after WiFi connection:");
    memoryMonitor.printMemoryUsage();
}

void loop()
{
    LVGLService::getInstance().loop();
    // =================================================================
    // 🔧 【可调节 #3】主循环刷新间隔 — 控制帧率上限
    // =================================================================
    // delay 越大 → 帧率越低 → SPI 总线压力越小 → 越不容易丢包
    //   delay(16)   ~60 FPS 上一版
    //   delay(33)   ~30 FPS ✅ 当前默认 — 推荐
    //   delay(50)   ~20 FPS 保守
    //   delay(100)  ~10 FPS 最保守 (动画明显卡顿但最稳)
    delay(33); // ←←← 在这里调刷新间隔
    // =================================================================
}
