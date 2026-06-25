#ifndef SETUP_MODE_H
#define SETUP_MODE_H

#include <Arduino.h>
#include "service/CaptivePortalService.h"
#include "driver/QuadPanel.h"
#include "ui/SetupUI.h"

/**
 * @brief 引导配网模式（单例）
 *
 * 设备未完成首次配置时激活：
 *   - 启动 Captive Portal（SoftAP + DNS 重定向 + WebServer）
 *   - 显示引导界面（WiFi 名称 + IP 地址）
 *   - loop() 中仅处理 HTTP 请求和 LVGL 动画刷新
 */
class SetupMode
{
public:
    static SetupMode &getInstance()
    {
        static SetupMode instance;
        return instance;
    }

    static constexpr const char *AP_NAME = "MyTV-Setup";

    void begin()
    {
        Serial.println("[SetupMode] No config found - entering Setup Mode");

        delay(1000);
        CaptivePortalService::getInstance().begin(AP_NAME);

        auto &portal = CaptivePortalService::getInstance();
        showSetupScreen(AP_NAME, portal.getIP());
        QuadPanel::getInstance().turnOnBacklight();

        Serial.printf("[SetupMode] Captive Portal at http://%s  (connect to WiFi '%s')\n",
                      portal.getIP().toString().c_str(), AP_NAME);
    }

    void loop()
    {
        CaptivePortalService::getInstance().loop();
        lv_timer_handler(); // 保持 LVGL 动画刷新
        delay(5);
    }

private:
    SetupMode() {}
    SetupMode(const SetupMode &) = delete;
    SetupMode &operator=(const SetupMode &) = delete;
};

#endif // SETUP_MODE_H
