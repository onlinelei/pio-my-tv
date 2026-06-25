#include "SetupUI.h"
#include "config/display_config.h"
#include <Arduino.h>

LV_FONT_DECLARE(lv_font_montserrat_20);
LV_FONT_DECLARE(chinese_18);

// 使用 PingFang SC 合并字体（含中文+ASCII）
#define FONT_CN (&chinese_18)

// ---- 辅助：清除屏幕 ----
static void clearScreen()
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_clean(scr);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x0a0a1a), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);
}

// ---- 辅助：创建居中文字 ----
static lv_obj_t *createLabel(lv_obj_t *parent, const char *text,
                             int32_t y_offset, uint32_t color, int32_t font_size = -1)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(color), LV_PART_MAIN);
    if (font_size > 0)
    {
        // 使用内建字体缩放（montserrat_20 为基础）
        // LVGL v9 不直接支持运行时 font size，用 montserrat_20 + 缩放
    }
    lv_obj_align(label, LV_ALIGN_CENTER, 0, y_offset);
    return label;
}

void showSetupScreen(const char *apName, IPAddress ip)
{
    clearScreen();
    lv_obj_t *scr = lv_scr_act();

    // ---- 左侧区域 (0~240): WiFi 图标 + "SETUP" ----
    lv_obj_t *arc = lv_arc_create(scr);
    lv_obj_set_size(arc, 120, 120);
    lv_obj_align(arc, LV_ALIGN_CENTER, -240, 0);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_angles(arc, 0, 270);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_set_style_arc_width(arc, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x4a9eff), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, 10, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x1a2a4a), LV_PART_MAIN);

    // 缓慢旋转动画
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, arc);
    lv_anim_set_exec_cb(&a, [](void *obj, int32_t v)
                        { lv_arc_set_rotation((lv_obj_t *)obj, (uint16_t)v); });
    lv_anim_set_values(&a, 0, 360);
    lv_anim_set_duration(&a, 5000);
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);
    lv_anim_start(&a);

    // ---- 中间区域 (240~480): 主信息 ----
    // 标题
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "MyTV Setup");
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, FONT_CN, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -60);

    // WiFi 名称
    char buf[80];
    snprintf(buf, sizeof(buf), "WiFi: %s", apName);
    lv_obj_t *ssidLabel = lv_label_create(scr);
    lv_label_set_text(ssidLabel, buf);
    lv_obj_set_style_text_color(ssidLabel, lv_color_hex(0x4a9eff), LV_PART_MAIN);
    lv_obj_set_style_text_font(ssidLabel, FONT_CN, LV_PART_MAIN);
    lv_obj_align(ssidLabel, LV_ALIGN_CENTER, 0, -20);

    // IP 地址
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    lv_obj_t *ipLabel = lv_label_create(scr);
    lv_label_set_text(ipLabel, buf);
    lv_obj_set_style_text_color(ipLabel, lv_color_hex(0x88cc88), LV_PART_MAIN);
    lv_obj_set_style_text_font(ipLabel, FONT_CN, LV_PART_MAIN);
    lv_obj_align(ipLabel, LV_ALIGN_CENTER, 0, 20);

    // 手动访问提示（如未自动跳转）
    char urlBuf[80];
    snprintf(urlBuf, sizeof(urlBuf), "如未自动跳转 访问 %d.%d.%d.%d",
             ip[0], ip[1], ip[2], ip[3]);
    lv_obj_t *hintLabel = lv_label_create(scr);
    lv_label_set_text(hintLabel, urlBuf);
    lv_obj_set_style_text_color(hintLabel, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(hintLabel, FONT_CN, LV_PART_MAIN);
    lv_obj_align(hintLabel, LV_ALIGN_CENTER, 0, 60);

    // ---- 右侧区域 (480~720): 状态指示 ----
    // 三个闪烁的点表示等待状态
    for (int i = 0; i < 3; i++)
    {
        lv_obj_t *dot = lv_obj_create(scr);
        lv_obj_set_size(dot, 16, 16);
        lv_obj_set_style_bg_color(dot, lv_color_hex(0x4a9eff), LV_PART_MAIN);
        lv_obj_set_style_border_width(dot, 0, LV_PART_MAIN);
        lv_obj_align(dot, LV_ALIGN_CENTER, 240 + (i - 1) * 30, 0);
        lv_obj_set_style_radius(dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);

        // 依次闪烁动画
        lv_anim_t dotAnim;
        lv_anim_init(&dotAnim);
        lv_anim_set_var(&dotAnim, dot);
        lv_anim_set_exec_cb(&dotAnim, [](void *obj, int32_t v)
                            { lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, LV_PART_MAIN); });
        lv_anim_set_values(&dotAnim, LV_OPA_30, LV_OPA_COVER);
        lv_anim_set_duration(&dotAnim, 600);
        lv_anim_set_delay(&dotAnim, i * 200);
        lv_anim_set_repeat_count(&dotAnim, LV_ANIM_REPEAT_INFINITE);
        lv_anim_set_playback_duration(&dotAnim, 600);
        lv_anim_start(&dotAnim);
    }
}

void showSetupSavedScreen()
{
    clearScreen();
    lv_obj_t *scr = lv_scr_act();

    // 勾号圆圈
    lv_obj_t *circle = lv_obj_create(scr);
    lv_obj_set_size(circle, 100, 100);
    lv_obj_align(circle, LV_ALIGN_CENTER, -240, 0);
    lv_obj_set_style_bg_color(circle, lv_color_hex(0x1a3a1a), LV_PART_MAIN);
    lv_obj_set_style_border_color(circle, lv_color_hex(0x4caf50), LV_PART_MAIN);
    lv_obj_set_style_border_width(circle, 3, LV_PART_MAIN);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);

    lv_obj_t *check = lv_label_create(circle);
    lv_label_set_text(check, LV_SYMBOL_OK);
    lv_obj_set_style_text_color(check, lv_color_hex(0x4caf50), LV_PART_MAIN);
    lv_obj_center(check);

    // 文字
    lv_obj_t *msg = lv_label_create(scr);
    lv_label_set_text(msg, "配置已保存!");
    lv_obj_set_style_text_color(msg, lv_color_hex(0x4caf50), LV_PART_MAIN);
    lv_obj_set_style_text_font(msg, FONT_CN, LV_PART_MAIN);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *msg2 = lv_label_create(scr);
    lv_label_set_text(msg2, "正在重启...");
    lv_obj_set_style_text_color(msg2, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(msg2, FONT_CN, LV_PART_MAIN);
    lv_obj_align(msg2, LV_ALIGN_CENTER, 0, 20);
}

void showFactoryResetScreen()
{
    clearScreen();
    lv_obj_t *scr = lv_scr_act();

    // 警告图标
    lv_obj_t *warn = lv_label_create(scr);
    lv_label_set_text(warn, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_color(warn, lv_color_hex(0xff6600), LV_PART_MAIN);
    lv_obj_align(warn, LV_ALIGN_CENTER, -240, 0);

    // 文字
    lv_obj_t *msg = lv_label_create(scr);
    lv_label_set_text(msg, "恢复出厂设置");
    lv_obj_set_style_text_color(msg, lv_color_hex(0xff6600), LV_PART_MAIN);
    lv_obj_set_style_text_font(msg, FONT_CN, LV_PART_MAIN);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *msg2 = lv_label_create(scr);
    lv_label_set_text(msg2, "正在擦除配置并重启...");
    lv_obj_set_style_text_color(msg2, lv_color_hex(0x888888), LV_PART_MAIN);
    lv_obj_set_style_text_font(msg2, FONT_CN, LV_PART_MAIN);
    lv_obj_align(msg2, LV_ALIGN_CENTER, 0, 20);
}
