#include <Arduino.h>
#include "service/LVGLService.h"
#include "ui/screens.h"
#include "ui/ui.h"

void setup()
{
    Serial.begin(115200);
    LVGLService::getInstance().setup();
    ui_init();

    // create_screen_main();
    // tick_screen_main();

    // create_screens();
    // tick_screen(0);
}

void loop()
{

    ui_tick();
    LVGLService::getInstance().loop();
}
