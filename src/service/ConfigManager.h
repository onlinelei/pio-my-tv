#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

/**
 * @brief 设备配置管理服务（单例）
 *
 * 基于 ESP32 Preferences (NVS) 持久化存储用户配置。
 * NVS namespace: "mytv_config"
 *
 * 首次启动时 NVS 为空 → isConfigured() 返回 false → 进入引导模式。
 * 用户通过 Captive Portal 配置后调用 save 系列方法写入 NVS。
 * 恢复出厂设置调用 factoryReset() 擦除所有配置并重启。
 */
class ConfigManager
{
public:
    static ConfigManager &getInstance()
    {
        static ConfigManager instance;
        return instance;
    }

    // ---- 初始化 ----

    void begin()
    {
        _prefs.begin("mytv_config", false); // false = 读写模式
        Serial.printf("[Config] NVS loaded. configured=%s\n",
                      isConfigured() ? "true" : "false");
    }

    // ---- 判断是否已完成首次配置 ----

    bool isConfigured()
    {
        return _prefs.getBool("configured", false);
    }

    // ---- WiFi ----

    String getWiFiSSID()
    {
        return _prefs.getString("wifi_ssid", "");
    }

    String getWiFiPassword()
    {
        return _prefs.getString("wifi_pwd", "");
    }

    void setWiFi(const String &ssid, const String &password)
    {
        _prefs.putString("wifi_ssid", ssid);
        _prefs.putString("wifi_pwd", password);
    }

    // ---- 设备名称 ----

    String getDeviceName()
    {
        return _prefs.getString("dev_name", "MyTV");
    }

    void setDeviceName(const String &name)
    {
        _prefs.putString("dev_name", name);
    }

    // ---- 屏幕亮度 (1-100) ----

    uint8_t getBrightness()
    {
        return _prefs.getUChar("brightness", 80); // 默认 80%
    }

    void setBrightness(uint8_t val)
    {
        _prefs.putUChar("brightness", constrain(val, 1, 100));
    }

    // ---- 时区 (UTC offset in hours, -12 ~ +14) ----

    int8_t getTimezone()
    {
        return _prefs.getChar("timezone", 8); // 默认 UTC+8 (中国)
    }

    void setTimezone(int8_t tz)
    {
        _prefs.putChar("timezone", tz);
    }

    // ---- OTA 自动更新 ----

    bool isOtaEnabled()
    {
        return _prefs.getBool("ota_enabled", true); // 默认开启
    }

    void setOtaEnabled(bool enabled)
    {
        _prefs.putBool("ota_enabled", enabled);
    }

    // ---- 保存所有配置（Captive Portal 提交时调用） ----

    void saveAll(const String &ssid, const String &password,
                 const String &deviceName, uint8_t brightness,
                 int8_t timezone, bool otaEnabled)
    {
        setWiFi(ssid, password);
        setDeviceName(deviceName);
        setBrightness(brightness);
        setTimezone(timezone);
        setOtaEnabled(otaEnabled);
        _prefs.putBool("configured", true);

        // 立即读回验证
        bool ok = isConfigured() && getWiFiSSID() == ssid;
        Serial.printf("[Config] saveAll complete. configured=%s, ssid=%s, ok=%s\n",
                      isConfigured() ? "true" : "false",
                      getWiFiSSID().c_str(),
                      ok ? "YES" : "NO *** VERIFY FAILED ***");
    }

    // ---- 恢复出厂设置 ----

    void factoryReset()
    {
        Serial.println("[Config] Factory reset - erasing NVS...");
        _prefs.clear();
        _prefs.end();
        Serial.println("[Config] NVS erased. Restarting...");
        delay(500);
        ESP.restart();
    }

    // ---- 调试打印 ----

    void printConfig()
    {
        Serial.println("========== Device Configuration ==========");
        Serial.printf("  Configured : %s\n", isConfigured() ? "Yes" : "No");
        Serial.printf("  WiFi SSID  : %s\n", getWiFiSSID().c_str());
        Serial.printf("  WiFi PWD   : %s\n", getWiFiPassword().length() > 0 ? "********" : "(empty)");
        Serial.printf("  Device Name: %s\n", getDeviceName().c_str());
        Serial.printf("  Brightness : %d%%\n", getBrightness());
        Serial.printf("  Timezone   : UTC%+d\n", getTimezone());
        Serial.printf("  OTA Enabled: %s\n", isOtaEnabled() ? "Yes" : "No");
        Serial.println("==========================================");
    }

private:
    ConfigManager() {}
    ~ConfigManager() {}
    ConfigManager(const ConfigManager &) = delete;
    ConfigManager &operator=(const ConfigManager &) = delete;

    Preferences _prefs;
};

#endif // CONFIG_MANAGER_H
