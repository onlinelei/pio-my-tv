#include <Arduino.h>
#include <Thread.h>
#include <StaticThreadController.h> //协程控制

#include "ThreadController.h"


static PinController pinController;
static ButtonHandler buttons(15, 13);
static DisplayController display(pinController);
static Animation animation(display);

ThreadController threadController(pinController, display, animation, buttons);

uint16_t colors[] = {
        TFT_BLACK, TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON, TFT_PURPLE,
        TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE, TFT_GREEN, TFT_CYAN,
        TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE, TFT_ORANGE, TFT_GREENYELLOW,
        TFT_PINK
    };
int forSize = 100;
int size = 300;
uint16_t color = TFT_DARKGREEN;

void setup()
{
    Serial.begin(115200);

    pinController.init();
    buttons.init();
    display.init();
    threadController.init();
    
}

void loop()
{
    threadController.run();

    // animation.lodingPage();
    if(forSize <= 0) {
        forSize = 100;
        size = random(100) + 300;
        color = colors[random(16)];
    }

    animation.runStarField(size, 3, color); // 调用绘制星空的函数    
    forSize -= 1;
    delay(50);
}