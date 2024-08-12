#include <Arduino.h>
#include <Thread.h>
#include <StaticThreadController.h> //协程控制

#include "DisplayController.h"
#include "ButtonHandler.h"
#include "Animation.h"

#include "config/config.h"

#define LCD_BL_PIN 22
DisplayController display;
ButtonHandler buttons(15, 13);
Animation animation(display);

// ========== 线程控制 ==============
Thread reflash_time = Thread();
Thread reflash_Banner = Thread();
StaticThreadController<2> controller(&reflash_time, &reflash_Banner);

// ========== 回调函数 ==============
void reflashTime()
{
    // TODO 更新时间
}

void reflashBanner()
{
    // TODO 切换天气 or 空气质量
}
void rightsingleclick(Button2 &btn)
{
}
void rightdoubleclick(Button2 &btn)
{
}
void wifi_reset(Button2 &btn)
{
}
void leftsingleclick(Button2 &btn)
{
}
void leftdoubleclick(Button2 &btn)
{
}
void esp_reset(Button2 &btn)
{
}





// ==========  主程序 ==============

void setup()
{
    Serial.begin(115200);

    pinMode(LCD_BL_PIN, OUTPUT);
    analogWrite(LCD_BL_PIN, 255);

    display.init();
    buttons.init();

    animation.run();
    

    reflash_time.setInterval(300); // 设置所需间隔 300毫秒
    reflash_time.onRun(reflashTime);

    reflash_Banner.setInterval(2 * TMS); // 设置所需间隔 2秒
    reflash_Banner.onRun(reflashBanner);
}

void loop()
{
    Serial.println("My First PIO Project!");

    controller.run();
    delay(1000);
}