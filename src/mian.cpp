#include <Arduino.h>
#include <Thread.h>
#include <StaticThreadController.h> //协程控制

#include "DisplayController.h"
#include "ButtonHandler.h"
#include "Animation.h"
#include "PinController.h"
#include "ThreadController.h"

PinController pinController;
DisplayController display(pinController);
Animation animation(display);

ButtonHandler buttons(15, 13);
ThreadController threadController;

void setup()
{
    Serial.begin(115200);
    buttons.init();
    threadController.init();

    display.init();
    animation.lodingPage();
    animation.runStarField(); // 调用绘制星空的函数
}

void loop()
{
    Serial.println("My First PIO Project!");
    threadController.run();

    animation.runStarField(); // 每秒更新一次星空
    delay(50);
}