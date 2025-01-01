#include <Arduino.h>
#include "service/LVGLService.h"
#include "ui/screens.h"
#include "ui/ui.h"
#include "service/WiFiService.h"

WiFiService wifiService;

void setup()
{
    Serial.begin(115200);
    LVGLService::getInstance().setup();
    ui_init();
    wifiService.init();

    // create_screen_main();
    // tick_screen_main();

    // create_screens();
    // tick_screen(0);
}

void loop()
{
    // static uint32_t last_update = 0;
    // uint32_t now = millis();

    // // 每1000ms (1秒) 更新一次
    // if (now - last_update >= 1000)
    // {
    //     // 获取text_show_area_ray对象
    //     lv_obj_t *text_area = objects.text_show_area_ray;
    //     if (text_area != NULL)
    //     {
    //         // 获取当前文本
    //         const char *current_text = lv_textarea_get_text(text_area);

    //         // 计算当前文本中的换行符数量
    //         int line_count = 0;
    //         for (const char *p = current_text; *p; p++)
    //         {
    //             if (*p == '\n')
    //                 line_count++;
    //         }

    //         // 如果超过10行，删除最早的一行
    //         if (line_count >= 10)
    //         {
    //             // 找到第一个换行符后的位置
    //             const char *first_newline = strchr(current_text, '\n');
    //             if (first_newline != NULL)
    //             {
    //                 // 将第一个换行符后的所有文本移动到开头
    //                 lv_textarea_set_text(text_area, first_newline + 1);
    //             }
    //         }

    //         // 生成随机数 (0-999)
    //         int random_num = random(1000);

    //         // 创建新文本 (添加随机数和换行符)
    //         char new_text[32];
    //         snprintf(new_text, sizeof(new_text), "%d\n", random_num);

    //         // 将新文本追加到文本区域
    //         lv_textarea_add_text(text_area, new_text);
    //     }

    //     // 更新标签
    //     lv_obj_t *label = objects.label_ray;
    //     if (label != NULL)
    //     {
    //         // 生成随机数 (0-999)
    //         int random_num = random(1000);

    //         // 创建新文本
    //         char label_text[32];
    //         snprintf(label_text, sizeof(label_text), "Random: %d", random_num);

    //         // 设置标签文本
    //         lv_label_set_text(label, label_text);
    //     }

    //     last_update = now;
    // }

    // // 每隔10秒切换到下一个屏幕
    // static uint32_t last_screen_change = 0;
    // static ScreensEnum current_screen = SCREEN_ID_MAIN;

    // if (now - last_screen_change >= 10000) // 10000ms = 10秒
    // {
    //     // 循环切换屏幕
    //     current_screen = (ScreensEnum)(((int)current_screen % 3) + 1);

    //     // 加载新屏幕
    //     loadScreen(current_screen);

    //     last_screen_change = now;
    // }

    ui_tick();
    LVGLService::getInstance().loop();
}
