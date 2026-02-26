#include <Arduino.h>
#include "service/LVGLService.h"
#include "ui/screens.h"
#include "ui/ui.h"
#include "driver/WiFi_Singleton.h"
#include "service/MemoryMonitor.h"

MemoryMonitor memoryMonitor;

void setup()
{
    Serial.begin(115200);
    LVGLService::getInstance().setup();

    // create_screen_main();
    // tick_screen_main();

    // create_screens();
    // tick_screen(0);
    ui_init();

    // 打印初始内存使用情况
    Serial.println("Initial Memory Usage:");
    memoryMonitor.printMemoryUsage();

    // 初始化 WiFi（示例代码）
    WiFi_Singleton::getInstance().init();

    // 打印 WiFi 连接后的内存使用情况
    Serial.println("Memory Usage after WiFi connection:");
    memoryMonitor.printMemoryUsage();
}

void loop()
{
    ui_tick();
    LVGLService::getInstance().loop();
}
