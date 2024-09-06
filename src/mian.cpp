#include <Arduino.h>
#include "service/LVGLService.h"

void setup()
{
    Serial.begin(115200);
    LVGLService::getInstance().setup();
}

void loop()
{
    LVGLService::getInstance().loop();
    // delay(1); // 添加延迟以避免过度占用 CPU
}
