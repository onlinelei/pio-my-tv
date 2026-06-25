#ifndef OTA_SERVICE_H
#define OTA_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>
#include <esp_ota_ops.h>
#include <MD5Builder.h>

// ---- 默认值（通过 platformio.ini build_flags 覆盖） ----
#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.0.0"
#endif

#ifndef OTA_MANIFEST_URL
#define OTA_MANIFEST_URL "http://localhost/ota/manifest.json"
#endif

#ifndef OTA_CHECK_INTERVAL_MS
#define OTA_CHECK_INTERVAL_MS 300000  // 默认 5 分钟
#endif

/**
 * @brief OTA 远程升级服务（单例）
 *
 * 功能：
 * - 定期从远程 manifest.json 检查固件版本
 * - 发现新版本时下载并通过 MD5 校验后刷入
 * - 利用 ESP32 A/B 双分区机制，升级失败自动回滚
 *
 * manifest.json 格式：
 * {
 *   "version": "1.1.0",
 *   "url": "firmware_1.1.0.bin",
 *   "md5": "d41d8cd98f00b204e9800998ecf8427e"
 * }
 *
 * 安全机制：
 * - L1: MD5 完整性校验（防传输损坏）
 * - L2: A/B 双分区 + 回滚保护（防变砖）
 */
class OTAService
{
public:
    static OTAService &getInstance()
    {
        static OTAService instance;
        return instance;
    }

    /**
     * 启动 OTA 服务
     * 1. 确认当前固件有效（取消回滚）
     * 2. 创建后台任务定期检查更新
     */
    void begin()
    {
        // 启动时确认当前 OTA 分区有效，防止回滚
        markCurrentFirmwareValid();

        Serial.printf("[OTA] Firmware: v%s\n", FIRMWARE_VERSION);
        Serial.printf("[OTA] Manifest: %s\n", OTA_MANIFEST_URL);
        Serial.printf("[OTA] Check interval: %lu ms\n", (unsigned long)OTA_CHECK_INTERVAL_MS);

        // 创建后台 FreeRTOS 任务
        xTaskCreatePinnedToCore(
            otaTaskEntry,
            "OTA_Task",
            8192,          // 栈大小
            this,          // 参数
            1,             // 优先级（低）
            &_taskHandle,
            0              // 固定在 Core 0（Core 1 留给 LVGL）
        );
        Serial.println("[OTA] Background task started on Core 0.");
    }

    /** 手动触发一次检查（调试用） */
    void triggerCheck()
    {
        _forceCheck = true;
    }

    /** 获取当前固件版本 */
    const char *getVersion() const
    {
        return FIRMWARE_VERSION;
    }

private:
    OTAService() = default;
    OTAService(const OTAService &) = delete;
    OTAService &operator=(const OTAService &) = delete;

    TaskHandle_t _taskHandle = nullptr;
    bool _forceCheck = false;
    bool _updating = false;

    /**
     * 标记当前固件有效，取消自动回滚
     * 如果从 OTA 升级后首次启动，此调用确认新固件正常工作
     * 如果不调用且看门狗超时，ESP32 会自动回滚到旧固件
     */
    void markCurrentFirmwareValid()
    {
        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_ota_img_states_t state;

        if (esp_ota_get_state_partition(running, &state) == ESP_OK)
        {
            if (state == ESP_OTA_IMG_PENDING_VERIFY)
            {
                esp_ota_mark_app_valid_cancel_rollback();
                Serial.printf("[OTA] Firmware v%s validated, rollback cancelled.\n", FIRMWARE_VERSION);
            }
            else
            {
                Serial.printf("[OTA] Partition state: %d (no pending rollback).\n", state);
            }
        }
    }

    /** FreeRTOS 任务入口（static） */
    static void otaTaskEntry(void *param)
    {
        auto *self = static_cast<OTAService *>(param);
        self->otaTaskLoop();
    }

    /** 后台任务主循环 */
    void otaTaskLoop()
    {
        // 启动后等待 WiFi 就绪
        vTaskDelay(pdMS_TO_TICKS(10000));

        while (true)
        {
            if (WiFi.status() == WL_CONNECTED && !_updating)
            {
                if (_forceCheck || true) // 每次间隔到了都检查
                {
                    _forceCheck = false;
                    checkAndUpdate();
                }
            }
            vTaskDelay(pdMS_TO_TICKS(OTA_CHECK_INTERVAL_MS));
        }
    }

    /**
     * 核心逻辑：检查 manifest → 比较版本 → 下载固件 → 刷入
     */
    void checkAndUpdate()
    {
        Serial.println("[OTA] Checking for updates...");

        // 1. 下载 manifest.json
        HTTPClient http;
        http.setTimeout(10000); // 10s 超时
        http.begin(OTA_MANIFEST_URL);

        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK)
        {
            Serial.printf("[OTA] Manifest fetch failed, HTTP %d\n", httpCode);
            http.end();
            return;
        }

        // 2. 解析 JSON
        String payload = http.getString();
        http.end();

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (err)
        {
            Serial.printf("[OTA] JSON parse error: %s\n", err.c_str());
            return;
        }

        const char *remoteVersion = doc["version"] | "unknown";
        const char *firmwareUrl = doc["url"] | "";
        const char *expectedMd5 = doc["md5"] | "";

        Serial.printf("[OTA] Remote: v%s (current: v%s)\n", remoteVersion, FIRMWARE_VERSION);

        // 3. 比较版本号
        if (strcmp(remoteVersion, FIRMWARE_VERSION) == 0)
        {
            Serial.println("[OTA] Already up to date.");
            return;
        }

        // 4. 验证 URL 和 MD5
        if (strlen(firmwareUrl) == 0)
        {
            Serial.println("[OTA] Error: manifest missing 'url' field.");
            return;
        }
        if (strlen(expectedMd5) == 0)
        {
            Serial.println("[OTA] Warning: manifest missing 'md5', skipping update for safety.");
            return;
        }

        Serial.println("[OTA] ========================================");
        Serial.printf("[OTA] New version available: v%s → v%s\n", FIRMWARE_VERSION, remoteVersion);
        Serial.printf("[OTA] Firmware URL: %s\n", firmwareUrl);
        Serial.printf("[OTA] Expected MD5: %s\n", expectedMd5);
        Serial.println("[OTA] ========================================");

        // 5. 构建完整的固件下载地址
        //    如果 url 是相对路径，拼接 manifest 的 base URL
        String fullUrl;
        if (firmwareUrl[0] == 'h') // http:// or https://
        {
            fullUrl = String(firmwareUrl);
        }
        else
        {
            // 从 manifest URL 提取 base path
            String baseUrl = String(OTA_MANIFEST_URL);
            int lastSlash = baseUrl.lastIndexOf('/');
            fullUrl = baseUrl.substring(0, lastSlash + 1) + String(firmwareUrl);
        }

        // 6. 下载并刷入固件
        _updating = true;
        performUpdate(fullUrl, String(expectedMd5));
        _updating = false;
    }

    /**
     * 执行固件下载和刷入
     * 手动下载 + Update 库写入 OTA 分区，支持 MD5 校验
     */
    void performUpdate(const String &url, const String &expectedMd5)
    {
        Serial.println("[OTA] Downloading firmware...");

        HTTPClient http;
        http.setTimeout(30000); // 30s 超时（固件可能较大）
        http.begin(url);

        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK)
        {
            Serial.printf("[OTA] Download failed, HTTP %d\n", httpCode);
            http.end();
            return;
        }

        int contentLength = http.getSize();
        if (contentLength <= 0)
        {
            Serial.println("[OTA] Error: unknown content length.");
            http.end();
            return;
        }

        Serial.printf("[OTA] Firmware size: %d bytes\n", contentLength);

        // 初始化 Update 库（写入 OTA 分区）
        if (!Update.begin(contentLength, U_FLASH))
        {
            Serial.printf("[OTA] Update.begin failed: %s\n", Update.errorString());
            http.end();
            return;
        }

        // 设置预期 MD5，Update 库会在写入完成后自动校验
        if (expectedMd5.length() > 0)
        {
            Update.setMD5(expectedMd5.c_str());
        }

        // 流式写入（边下载边写，不占额外内存）
        WiFiClient *stream = http.getStreamPtr();
        size_t written = Update.writeStream(*stream);
        http.end();

        Serial.printf("[OTA] Written: %u / %d bytes\n", written, contentLength);

        if (written != (size_t)contentLength)
        {
            Serial.printf("[OTA] Error: incomplete write (%u != %d)\n", written, contentLength);
            Update.abort();
            return;
        }

        // 完成写入 + MD5 校验
        if (!Update.end(true))
        {
            Serial.printf("[OTA] Update.end failed: %s\n", Update.errorString());
            Update.abort();
            return;
        }

        // MD5 校验通过 + 写入成功
        if (!Update.isFinished())
        {
            Serial.println("[OTA] Error: update not finished.");
            return;
        }

        Serial.println("[OTA] ========================================");
        Serial.println("[OTA] Update SUCCESS! MD5 verified.");
        Serial.println("[OTA] Rebooting in 3 seconds...");
        Serial.println("[OTA] ========================================");
        delay(3000);
        ESP.restart();
    }
};

#endif // OTA_SERVICE_H
