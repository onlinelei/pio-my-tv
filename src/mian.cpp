#include <Arduino.h>
#include <Thread.h>
#include <StaticThreadController.h> //协程控制

#include "ThreadController.h"
#include "PinController.h"
#include "ButtonHandler.h"
#include "DisplayController.h"
#include "Animation.h"
#include "WiFiManager.h"

void setup()
{
    Serial.begin(115200);
    PinController::getInstance().init();
    ButtonHandler::getInstance().init(15, 13);
    DisplayController::getInstance().init();
    ThreadController::getInstance().init();
    WiFiManager::getInstance().init();
}

void loop()
{
    ThreadController::getInstance().run();
    // animation.lodingPage();
}