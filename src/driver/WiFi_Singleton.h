#ifndef WIFI_SINGLETON_H
#define WIFI_SINGLETON_H

#include <WiFi.h>
#include "service/ConfigManager.h"

#ifndef WIFI_SSID
#define WIFI_SSID "your_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_PASSWORD"
#endif

// WiFi 连接超时时间
#ifndef WIFI_CONNECT_TIMEOUT_MS
#define WIFI_CONNECT_TIMEOUT_MS 15000
#endif

class WiFi_Singleton
{
public:
    static WiFi_Singleton &getInstance()
    {
        static WiFi_Singleton instance;
        return instance;
    }

    /**
     * @brief 初始化 WiFi 连接
     *
     * 优先从 NVS (ConfigManager) 读取 SSID/密码，
     * 若 NVS 无配置则使用 platformio.ini 硬编码的 WIFI_SSID/WIFI_PASSWORD。
     * 带超时机制，避免死循环阻塞。
     */
    void init()
    {
        // 优先使用 NVS 配置
        String ssid = ConfigManager::getInstance().getWiFiSSID();
        String pwd = ConfigManager::getInstance().getWiFiPassword();

        // Fallback: NVS 无配置时使用编译时宏
        if (ssid.isEmpty())
        {
            ssid = WIFI_SSID;
            pwd = WIFI_PASSWORD;
            Serial.println("[WiFi] Using compile-time SSID (NVS empty)");
        }
        else
        {
            Serial.printf("[WiFi] Using NVS config: SSID=%s\n", ssid.c_str());
        }

        connectWithTimeout(ssid.c_str(), pwd.c_str());
    }

    bool isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }

    void reconnect()
    {
        if (!isConnected())
        {
            Serial.println("[WiFi] Reconnecting...");
            WiFi.disconnect();

            String ssid = ConfigManager::getInstance().getWiFiSSID();
            String pwd = ConfigManager::getInstance().getWiFiPassword();
            if (ssid.isEmpty())
            {
                ssid = WIFI_SSID;
                pwd = WIFI_PASSWORD;
            }
            connectWithTimeout(ssid.c_str(), pwd.c_str());
        }
    }

private:
    WiFi_Singleton() {}
    ~WiFi_Singleton() {}
    WiFi_Singleton(const WiFi_Singleton &) = delete;
    WiFi_Singleton &operator=(const WiFi_Singleton &) = delete;

    void connectWithTimeout(const char *ssid, const char *password)
    {
        WiFi.begin(ssid, password);

        unsigned long startMs = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
            if (millis() - startMs > WIFI_CONNECT_TIMEOUT_MS)
            {
                Serial.printf("\n[WiFi] Connection timeout after %dms\n",
                              WIFI_CONNECT_TIMEOUT_MS);
                WiFi.disconnect();
                return;
            }
        }
        Serial.println("");
        Serial.println("[WiFi] Connected");
        Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
    }
};

#endif // WIFI_SINGLETON_H
