#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include "config/version.h"
#include <ArduinoJson.h>
#include <vector>

class OTAService
{
private:
    String firmwareVersion;
    String serverUrl;

    static OTAService *instance;

    // 私有构造函数
    OTAService()
    {
        firmwareVersion = FIRMWARE_VERSION;
    }

    // 比较版本号
    bool isNewerVersion(const String &serverVersion)
    {
        // 将版本号按点分割
        std::vector<int> current;
        std::vector<int> server;

        // 分割当前版本号
        String temp = firmwareVersion;
        while (temp.length() > 0)
        {
            int dot = temp.indexOf('.');
            if (dot == -1)
            {
                current.push_back(temp.toInt());
                break;
            }
            current.push_back(temp.substring(0, dot).toInt());
            temp = temp.substring(dot + 1);
        }

        // 分割服务器版本号
        temp = serverVersion;
        while (temp.length() > 0)
        {
            int dot = temp.indexOf('.');
            if (dot == -1)
            {
                server.push_back(temp.toInt());
                break;
            }
            server.push_back(temp.substring(0, dot).toInt());
            temp = temp.substring(dot + 1);
        }

        // 取较短的长度进行比较
        size_t len = min(current.size(), server.size());

        // 逐位比较版本号
        for (size_t i = 0; i < len; i++)
        {
            if (server[i] > current[i])
            {
                return true;
            }
            if (server[i] < current[i])
            {
                return false;
            }
        }

        // 如果前面的位数都相同，比较版本号长度
        return server.size() > current.size();
    }

public:
    static OTAService &getInstance()
    {
        if (instance == nullptr)
        {
            instance = new OTAService();
        }
        return *instance;
    }

    void begin(const char *url = nullptr)
    {
        // 确保版本号已设置
        if (firmwareVersion.isEmpty())
        {
            firmwareVersion = FIRMWARE_VERSION;
        }

        // 如果提供了URL，则设置服务器地址
        if (url != nullptr)
        {
            setServerUrl(url);
        }

        Serial.printf("OTA Service initialized with version: %s\n", firmwareVersion.c_str());
        if (!serverUrl.isEmpty())
        {
            Serial.printf("OTA Server URL: %s\n", serverUrl.c_str());
        }
    }

    bool checkForUpdates()
    {
        Serial.println("\n=== Starting OTA Update Check ===");
        Serial.printf("Current firmware version: %s\n", firmwareVersion.c_str());

        // 1. 检查WiFi连接
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("Error: WiFi not connected!");
            return false;
        }

        // 2. 打印系统信息
        Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());
        Serial.printf("Sketch Size: %d bytes\n", ESP.getSketchSize());
        Serial.printf("Free Sketch Space: %d bytes\n", ESP.getFreeSketchSpace());

        // 3. 设置更新配置
        httpUpdate.setLedPin(LED_BUILTIN, LOW);
        httpUpdate.rebootOnUpdate(true);
        httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);

        // 4. 获取版本信息
        HTTPClient http;
        http.begin(serverUrl + "version.json");

        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK)
        {
            Serial.printf("Version check failed, error: %d\n", httpCode);
            http.end();
            return false;
        }

        String payload = http.getString();
        http.end();

        // 解析JSON
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error)
        {
            Serial.printf("JSON parsing failed: %s\n", error.c_str());
            return false;
        }

        // 获取服务器版本
        const char *serverVersion = doc["version"];
        Serial.printf("Server version: %s\n", serverVersion);

        // 检查是否需要更新
        if (!isNewerVersion(serverVersion))
        {
            Serial.println("Current firmware is up to date");
            return false;
        }

        Serial.println("New version available, starting update...");

        // 5. 开始更新
        Serial.printf("Updating from: %s\n", (serverUrl + "firmware.bin").c_str());

        WiFiClient client;
        t_httpUpdate_return ret = httpUpdate.update(client, serverUrl + "firmware.bin");

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            Serial.printf("Update failed! Error (%d): %s\n",
                          httpUpdate.getLastError(),
                          httpUpdate.getLastErrorString().c_str());
            return false;

        case HTTP_UPDATE_NO_UPDATES:
            Serial.println("No updates needed");
            return false;

        case HTTP_UPDATE_OK:
            Serial.println("Update successful! Rebooting...");
            delay(1000); // 等待日志输出
            return true;
        }

        return false;
    }

    // 设置服务器URL
    void setServerUrl(const char *url)
    {
        serverUrl = url;
        if (!serverUrl.endsWith("/"))
        {
            serverUrl += "/";
        }
        Serial.printf("OTA Server URL updated: %s\n", serverUrl.c_str());
    }
};

OTAService *OTAService::instance = nullptr;

#endif // OTA_SERVICE_H