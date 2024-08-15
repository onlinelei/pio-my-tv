#include <Arduino.h>
#include <Thread.h>
#include <StaticThreadController.h> //协程控制

#include "DisplayController.h"
#include "ButtonHandler.h"
#include "Animation.h"
#include "ThreadController.h"

DisplayController display;
ButtonHandler buttons(15, 13);
Animation animation(display);
ThreadController threadController;

void setup()
{
    Serial.begin(115200);

    display.init();
    buttons.init();
    animation.run();
    threadController.init();

}

void loop()
{
    Serial.println("My First PIO Project!");

    threadController.run();
    delay(1000);
}