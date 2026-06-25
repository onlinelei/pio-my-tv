#ifndef NORMAL_MODE_H
#define NORMAL_MODE_H

#include <Arduino.h>
#include "service/LVGLService.h"
#include "service/ConfigManager.h"
#include "service/OTAService.h"
#include "service/MemoryMonitor.h"
#include "driver/WiFi_Singleton.h"
#include "driver/QuadPanel.h"
#include "demo/DemoAnimation.h"

/**
 * @brief 正常运行模式（单例）
 *
 * 设备已完成首次配置时激活：
 *   - 启动 DemoAnimation 演示 + WiFi + OTA 服务
 *   - loop() 中运行 LVGL 主循环
 */
class NormalMode
{
public:
    static NormalMode &getInstance()
    {
        static NormalMode instance;
        return instance;
    }

    void begin()
    {
        Serial.println("[NormalMode] Device configured - entering Normal Mode");

        delay(3000);
        createDemoAnimation();
        QuadPanel::getInstance().turnOnBacklight();

        _memMonitor.printMemoryUsage();

        WiFi_Singleton::getInstance().init();

        if (WiFi_Singleton::getInstance().isConnected() &&
            ConfigManager::getInstance().isOtaEnabled())
        {
            OTAService::getInstance().begin();
        }
    }

    void loop()
    {
        LVGLService::getInstance().loop();

#ifndef LOOP_DELAY_MS
#define LOOP_DELAY_MS 16
#endif
        delay(LOOP_DELAY_MS);
    }

private:
    NormalMode() {}
    NormalMode(const NormalMode &) = delete;
    NormalMode &operator=(const NormalMode &) = delete;

    MemoryMonitor _memMonitor;
};

#endif // NORMAL_MODE_H
