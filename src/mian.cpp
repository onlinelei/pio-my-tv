#include <Arduino.h>
#include "service/LVGLService.h"
#include "service/ConfigManager.h"
#include "service/BootButton.h"
#include "service/SetupMode.h"
#include "service/NormalMode.h"

static bool _setupMode = false;

void setup()
{
    Serial.begin(115200);
    delay(300);
    Serial.println("\n[BOOT] MyTV starting...");

    // 硬件初始化
    LVGLService::getInstance().setup();
    BootButton::getInstance().begin();

    // 配置管理
    ConfigManager::getInstance().begin();
    ConfigManager::getInstance().printConfig();

    // 选择运行模式
    if (!ConfigManager::getInstance().isConfigured())
    {
        _setupMode = true;
        SetupMode::getInstance().begin();
    }
    else
    {
        NormalMode::getInstance().begin();
    }
}

void loop()
{
    BootButton::getInstance().loop();

    if (_setupMode)
        SetupMode::getInstance().loop();
    else
        NormalMode::getInstance().loop();
}
