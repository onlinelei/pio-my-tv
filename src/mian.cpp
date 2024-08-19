#include <Arduino.h>
#include <Thread.h>
#include <StaticThreadController.h> //协程控制

#include "ThreadController.h"


static PinController pinController;
static ButtonHandler buttons(15, 13);
static DisplayController display(pinController);
static Animation animation(display);

ThreadController threadController(pinController, display, animation, buttons);

int tempNumStars = 300;
int tempSize = 3;
uint16_t tempColor = TFT_RED;
int tempCount = 100;
int speed = 5;

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
    // threadController.run();

    // animation.lodingPage();
    animation.runStarFieldAuto(&tempNumStars, &tempSize, &speed, &tempColor, &tempCount);

    delay(20);
}