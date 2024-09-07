#include <Arduino.h>
#include "service/LVGLService.h"
#include "ui/screens.h"
#include "ui/ui.h"

void setup()
{
    Serial.begin(115200);
    LVGLService::getInstance().setup();

    // create_screen_main();
    // tick_screen_main();

    // create_screens();
    // tick_screen(0);
    ui_init();
}

void loop()
{
    ui_tick();
    LVGLService::getInstance().loop();
    // delay(1); // 添加延迟以避免过度占用 CPU
}
