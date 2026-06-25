#ifndef CAPTIVE_PORTAL_SERVICE_H
#define CAPTIVE_PORTAL_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <map>
#include "service/ConfigManager.h"
#include "service/ConfigPortal_html.h"

/**
 * @brief Captive Portal 配网服务（单例）
 *
 * 首次启动时创建 WiFi 热点 + DNS 重定向 + Web 配置页面。
 * 用户手机连接热点后，打开任意网页自动跳转到配置界面。
 * 配置保存后设备自动重启进入正常模式。
 */
class CaptivePortalService
{
public:
    static CaptivePortalService &getInstance()
    {
        static CaptivePortalService instance;
        return instance;
    }

    /**
     * @brief 启动 Captive Portal
     * @param apName WiFi 热点名称
     */
    void begin(const char *apName = "MyTV-Setup")
    {
        Serial.println("[Portal] Starting Captive Portal...");

        // 1) 启动 SoftAP
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apName);
        _apIP = WiFi.softAPIP();
        Serial.printf("[Portal] AP started: %s (IP: %s)\n",
                      apName, _apIP.toString().c_str());

        // 2) 启动 DNS 服务器，所有域名解析到 AP IP（Captive Portal 核心）
        _dns = new DNSServer();
        _dns->setErrorReplyCode(DNSReplyCode::NoError);
        _dns->start(53, "*", _apIP);
        Serial.println("[Portal] DNS captive portal active");

        // 3) 启动 Web 服务器
        _server = new WebServer(80);
        setupRoutes();
        _server->begin();
        Serial.println("[Portal] Web server started on port 80");

        _running = true;
    }

    /**
     * @brief 在 loop() 中调用，处理 DNS 和 HTTP 请求
     */
    void loop()
    {
        if (!_running)
            return;
        _dns->processNextRequest();
        _server->handleClient();
    }

    /**
     * @brief 获取 AP IP 地址
     */
    IPAddress getIP() const { return _apIP; }

    /**
     * @brief 获取 AP 名称
     */
    String getAPName() const { return _apName; }

    /**
     * @brief 是否正在运行
     */
    bool isRunning() const { return _running; }

private:
    CaptivePortalService() {}
    ~CaptivePortalService()
    {
        if (_dns) { _dns->stop(); delete _dns; }
        if (_server) { _server->stop(); delete _server; }
    }
    CaptivePortalService(const CaptivePortalService &) = delete;
    CaptivePortalService &operator=(const CaptivePortalService &) = delete;

    void setupRoutes()
    {
        // ---- 配置页面 ----
        _server->on("/", HTTP_GET, [this]()
        {
            _server->send_P(200, "text/html", CONFIG_PORTAL_HTML);
        });

        // ---- WiFi 扫描 ----
        _server->on("/scan", HTTP_GET, [this]()
        {
            Serial.println("[Portal] Scanning WiFi networks...");
            int n = WiFi.scanNetworks();
            JsonDocument doc;
            JsonArray arr = doc.to<JsonArray>();

            // 去重：同一 SSID 只保留信号最强的
            std::map<String, int> seen; // ssid -> index in arr
            for (int i = 0; i < n; i++)
            {
                String ssid = WiFi.SSID(i);
                if (ssid.isEmpty())
                    continue;

                int rssi = WiFi.RSSI(i);
                if (seen.find(ssid) != seen.end())
                {
                    // 已存在，更新为更强的信号
                    int idx = seen[ssid];
                    if (rssi > arr[idx]["rssi"].as<int>())
                    {
                        arr[idx]["rssi"] = rssi;
                    }
                }
                else
                {
                    seen[ssid] = arr.size();
                    JsonObject obj = arr.add<JsonObject>();
                    obj["ssid"] = ssid;
                    obj["rssi"] = rssi;
                }
            }
            WiFi.scanDelete();

            String json;
            serializeJson(doc, json);
            _server->send(200, "application/json", json);
            Serial.printf("[Portal] Found %d unique networks\n", (int)seen.size());
        });

        // ---- 保存配置 ----
        _server->on("/save", HTTP_POST, [this]()
        {
            if (!_server->hasArg("plain"))
            {
                _server->send(400, "text/plain", "Missing body");
                return;
            }

            String body = _server->arg("plain");
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, body);
            if (err)
            {
                _server->send(400, "text/plain", String("JSON error: ") + err.c_str());
                return;
            }

            String ssid = doc["ssid"] | "";
            String password = doc["password"] | "";
            String deviceName = doc["deviceName"] | "MyTV";
            uint8_t brightness = doc["brightness"] | 80;
            int8_t timezone = doc["timezone"] | 8;
            bool otaEnabled = doc["otaEnabled"] | true;

            if (ssid.isEmpty())
            {
                _server->send(400, "text/plain", "WiFi SSID is required");
                return;
            }

            Serial.println("[Portal] Saving configuration:");
            Serial.printf("  SSID     : %s\n", ssid.c_str());
            Serial.printf("  Device   : %s\n", deviceName.c_str());
            Serial.printf("  Brightness: %d%%\n", brightness);
            Serial.printf("  Timezone : UTC%+d\n", timezone);
            Serial.printf("  OTA      : %s\n", otaEnabled ? "Enabled" : "Disabled");

            ConfigManager::getInstance().saveAll(
                ssid, password, deviceName, brightness, timezone, otaEnabled);

            _server->send(200, "application/json",
                          "{\"status\":\"ok\",\"message\":\"Saved! Restarting...\"}");

            // 延迟 1 秒让 HTTP 响应发送完毕，然后重启
            delay(1000);
            Serial.println("[Portal] Restarting device...");
            ESP.restart();
        });

        // ---- 状态查询 ----
        _server->on("/status", HTTP_GET, [this]()
        {
            auto &cfg = ConfigManager::getInstance();
            JsonDocument doc;
            doc["configured"] = cfg.isConfigured();
            doc["deviceName"] = cfg.getDeviceName();
            doc["brightness"] = cfg.getBrightness();
            doc["timezone"] = cfg.getTimezone();
            doc["otaEnabled"] = cfg.isOtaEnabled();

            String json;
            serializeJson(doc, json);
            _server->send(200, "application/json", json);
        });

        // ---- Captive Portal 重定向（兼容各平台检测） ----
        // Android
        _server->on("/generate_204", HTTP_GET, [this]() { redirect(); });
        // Apple
        _server->on("/hotspot-detect.html", HTTP_GET, [this]() { redirect(); });
        // Windows
        _server->on("/connecttest.txt", HTTP_GET, [this]() { redirect(); });
        _server->on("/ncsi.txt", HTTP_GET, [this]() { redirect(); });
        // Firefox
        _server->on("/canonical.html", HTTP_GET, [this]() { redirect(); });
        // Fallback: 所有未知路径重定向到首页
        _server->onNotFound([this]() { redirect(); });
    }

    void redirect()
    {
        _server->sendHeader("Location", "http://" + _apIP.toString() + "/");
        _server->send(302, "text/plain", "");
    }

    DNSServer *_dns = nullptr;
    WebServer *_server = nullptr;
    IPAddress _apIP;
    String _apName = "MyTV-Setup";
    bool _running = false;
};

#endif // CAPTIVE_PORTAL_SERVICE_H
