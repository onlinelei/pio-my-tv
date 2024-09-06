/*Using LVGL with Arduino requires some extra steps:
 *Be sure to read the docs here: https://docs.lvgl.io/master/get-started/platforms/arduino.html  */
#include "service/LVGLService.h"

// ============== Main program ==============

// ============== LVGL setup ==============
void setup()
{
    Serial.begin(115200);

    LVGLService::getInstance().init();
    Serial.println("Setup done");
}

void loop()
{
    LVGLService::getInstance().push();
    delay(5);
}
