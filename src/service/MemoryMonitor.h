#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <esp_chip_info.h>
#include <esp_idf_version.h>
#include <esp_system.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <FS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/**
 * @brief 极客风系统状态打印工具
 *  使用 ANSI 颜色 + Unicode 进度条，输出芯片 / 内存 / Flash / 网络 / 系统状态
 *  若终端不支持 ANSI，可在初始化时调用 disableColor()
 */
class MemoryMonitor
{
private:
    // 默认开启 ANSI 彩色；platformio.ini 加 build_flags = -DMM_NO_COLOR 则全局关闭
#if defined(MM_NO_COLOR)
    bool _color = false;
#else
    bool _color = true;
#endif

    // ---- ANSI 颜色（不支持时无副作用，纯文本） ----
    const char *C_RESET = "\033[0m";
    const char *C_TITLE = "\033[1;36m";   // bold cyan
    const char *C_SECTION = "\033[1;35m"; // bold magenta
    const char *C_LABEL = "\033[0;90m";   // dark gray
    const char *C_VALUE = "\033[0;97m";   // bright white
    const char *C_OK = "\033[0;32m";      // green
    const char *C_WARN = "\033[0;33m";    // yellow
    const char *C_BAD = "\033[0;31m";     // red
    const char *C_DIM = "\033[0;90m";

    const char *col(const char *c) { return _color ? c : ""; }

    // ---- 数值格式化 ----
    static String formatKB(size_t bytes)
    {
        return String(bytes / 1024.0f, 1) + " KB";
    }
    static String formatMB(size_t bytes)
    {
        return String(bytes / 1024.0f / 1024.0f, 2) + " MB";
    }

    // ---- Unicode 进度条 ----
    String makeBar(float percent, int width = 20)
    {
        if (percent < 0)
            percent = 0;
        if (percent > 100)
            percent = 100;
        int filled = (int)(percent * width / 100.0f + 0.5f);
        const char *color = (percent < 60) ? C_OK : (percent < 85) ? C_WARN
                                                                   : C_BAD;
        String bar = "[";
        bar += col(color);
        for (int i = 0; i < filled; i++)
            bar += "█";
        bar += col(C_DIM);
        for (int i = filled; i < width; i++)
            bar += "░";
        bar += col(C_RESET);
        bar += "]";
        return bar;
    }

    // ---- 复位原因可读化 ----
    static const char *resetReasonStr(esp_reset_reason_t r)
    {
        switch (r)
        {
        case ESP_RST_UNKNOWN:
            return "Unknown (USB-CDC?)";
        case ESP_RST_POWERON:
            return "Power-on";
        case ESP_RST_EXT:
            return "External Pin";
        case ESP_RST_SW:
            return "Software";
        case ESP_RST_PANIC:
            return "Exception/Panic";
        case ESP_RST_INT_WDT:
            return "Interrupt WDT";
        case ESP_RST_TASK_WDT:
            return "Task WDT";
        case ESP_RST_WDT:
            return "Other WDT";
        case ESP_RST_DEEPSLEEP:
            return "Deep-sleep Wake";
        case ESP_RST_BROWNOUT:
            return "Brownout";
        case ESP_RST_SDIO:
            return "SDIO";
#ifdef ESP_RST_USB
        case ESP_RST_USB:
            return "USB Peripheral";
#endif
#ifdef ESP_RST_JTAG
        case ESP_RST_JTAG:
            return "JTAG";
#endif
        default:
            return "Unknown";
        }
    }

    // ---- 芯片型号 ----
    static const char *chipModelStr(esp_chip_model_t m)
    {
        switch (m)
        {
        case CHIP_ESP32:
            return "ESP32";
        case CHIP_ESP32S2:
            return "ESP32-S2";
        case CHIP_ESP32S3:
            return "ESP32-S3";
        case CHIP_ESP32C3:
            return "ESP32-C3";
        case CHIP_ESP32H2:
            return "ESP32-H2";
        default:
            return "Unknown";
        }
    }

    // ---- WiFi 信号格 ----
    static String rssiBars(int rssi)
    {
        int bars = 0;
        if (rssi >= -50)
            bars = 5;
        else if (rssi >= -60)
            bars = 4;
        else if (rssi >= -70)
            bars = 3;
        else if (rssi >= -80)
            bars = 2;
        else if (rssi >= -90)
            bars = 1;
        String s = "";
        for (int i = 0; i < 5; i++)
            s += (i < bars) ? "▮" : "▯";
        return s;
    }

    // ---- Uptime 人性化 ----
    static String formatUptime(unsigned long sec)
    {
        char b[32];
        if (sec < 60)
            snprintf(b, sizeof(b), "%lu s", sec);
        else if (sec < 3600)
            snprintf(b, sizeof(b), "%lu m %lu s", sec / 60, sec % 60);
        else if (sec < 86400)
            snprintf(b, sizeof(b), "%lu h %lu m", sec / 3600, (sec % 3600) / 60);
        else
            snprintf(b, sizeof(b), "%lu d %lu h", sec / 86400, (sec % 86400) / 3600);
        return String(b);
    }

    // ---- 输出辅助 ----
    void section(const char *name)
    {
        Serial.println();
        Serial.printf("%s▶ %s%s\n", col(C_SECTION), name, col(C_RESET));
    }

    void kv(const char *key, const String &value)
    {
        Serial.printf("  %s%-12s%s │ %s%s%s\n",
                      col(C_LABEL), key, col(C_RESET),
                      col(C_VALUE), value.c_str(), col(C_RESET));
    }
    void kv(const char *key, const char *value) { kv(key, String(value)); }

    void barLine(const char *key, float percent, const String &detail)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), " %5.1f%% │ ", percent);
        kv(key, makeBar(percent) + buf + detail);
    }

    // ===================== 各分区 =====================

    void printBanner()
    {
        Serial.println();
        Serial.printf("%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n",
                      col(C_TITLE), col(C_RESET));
        Serial.printf("%s  ░▒▓ ESP32-S3  ::  myTV Quad Display Console ▓▒░%s\n",
                      col(C_TITLE), col(C_RESET));
        Serial.printf("%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n",
                      col(C_TITLE), col(C_RESET));
    }

    void printChipInfo()
    {
        section("CHIP");
        esp_chip_info_t info;
        esp_chip_info(&info);

        kv("Model", String(chipModelStr(info.model)) + " rev " + String(info.revision));
        kv("CPU", String(info.cores) + " × Xtensa LX7 @ " + String(ESP.getCpuFreqMHz()) + " MHz");

        String f;
        if (info.features & CHIP_FEATURE_WIFI_BGN)
            f += "WiFi · ";
        if (info.features & CHIP_FEATURE_BT)
            f += "BT · ";
        if (info.features & CHIP_FEATURE_BLE)
            f += "BLE · ";
        if (info.features & CHIP_FEATURE_EMB_FLASH)
            f += "Emb-Flash · ";
        if (info.features & CHIP_FEATURE_EMB_PSRAM)
            f += "Emb-PSRAM · ";
        if (f.endsWith(" · "))
            f = f.substring(0, f.length() - 3);
        kv("Features", f);

        kv("SDK", esp_get_idf_version());
        kv("Arduino", String("v") + String(ESP_ARDUINO_VERSION_MAJOR) + "." +
                          String(ESP_ARDUINO_VERSION_MINOR) + "." +
                          String(ESP_ARDUINO_VERSION_PATCH));

        uint64_t chipId = ESP.getEfuseMac();
        char idBuf[24];
        snprintf(idBuf, sizeof(idBuf), "0x%04X%08X",
                 (uint16_t)(chipId >> 32), (uint32_t)chipId);
        kv("Chip ID", idBuf);
    }

    void printMemoryInfo()
    {
        section("MEMORY");

        // Internal Heap
        size_t heapTotal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        size_t heapFree = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        size_t heapUsed = heapTotal - heapFree;
        size_t heapMin = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        float heapPct = heapTotal ? heapUsed * 100.0f / heapTotal : 0;
        barLine("Heap", heapPct, formatKB(heapUsed) + " / " + formatKB(heapTotal));

        // PSRAM
        size_t psTotal = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
        if (psTotal > 0)
        {
            size_t psFree = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
            size_t psUsed = psTotal - psFree;
            float psPct = psUsed * 100.0f / psTotal;
            barLine("PSRAM", psPct, formatKB(psUsed) + " / " + formatMB(psTotal));
        }
        else
        {
            kv("PSRAM", "Not available");
        }

        // IRAM
        size_t iramTotal = heap_caps_get_total_size(MALLOC_CAP_EXEC);
        size_t iramFree = heap_caps_get_free_size(MALLOC_CAP_EXEC);
        size_t iramUsed = iramTotal - iramFree;
        float iramPct = iramTotal ? iramUsed * 100.0f / iramTotal : 0;
        barLine("IRAM", iramPct, formatKB(iramUsed) + " / " + formatKB(iramTotal));

        // DMA
        size_t dmaTotal = heap_caps_get_total_size(MALLOC_CAP_DMA);
        size_t dmaFree = heap_caps_get_free_size(MALLOC_CAP_DMA);
        kv("DMA Free", formatKB(dmaFree) + " / " + formatKB(dmaTotal));

        // Min heap & frag
        kv("Min-Heap", formatKB(heapMin) + " (lowest ever)");
        multi_heap_info_t mi;
        heap_caps_get_info(&mi, MALLOC_CAP_INTERNAL);
        kv("Frag", String(mi.free_blocks) + " free blks · largest " + formatKB(mi.largest_free_block));
    }

    void printFlashInfo()
    {
        section("FLASH");

        // Sketch：用运行分区的总大小作为分母（ESP.getFreeSketchSpace() 返回的是另一个 OTA 分区全尺寸，作分母会造成比例偏小）
        size_t sketchSize = ESP.getSketchSize();
        size_t partitionSize = 0;
        const esp_partition_t *running = esp_ota_get_running_partition();
        if (running)
        {
            partitionSize = running->size;
        }
        else
        {
            partitionSize = sketchSize + ESP.getFreeSketchSpace();
        }
        if (partitionSize == 0)
            partitionSize = 1; // 防除零
        float sketchPct = sketchSize * 100.0f / partitionSize;
        barLine("Sketch", sketchPct, formatKB(sketchSize) + " / " + formatKB(partitionSize));

        const char *mode = "?";
        // 避开 Arduino-ESP32 v2.0.17 在 ESP32-S3 上 ESP.getFlashChipMode() 的 bug（REG_READ 宏展开出错误地址导致 LoadProhibited）
        // 改用编译时 sdkconfig 宏推断实际使用的 Flash 模式
#if defined(CONFIG_ESPTOOLPY_FLASHMODE_QIO)
        mode = "QIO";
#elif defined(CONFIG_ESPTOOLPY_FLASHMODE_QOUT)
        mode = "QOUT";
#elif defined(CONFIG_ESPTOOLPY_FLASHMODE_DIO)
        mode = "DIO";
#elif defined(CONFIG_ESPTOOLPY_FLASHMODE_DOUT)
        mode = "DOUT";
#endif
        char chipBuf[80];
        snprintf(chipBuf, sizeof(chipBuf), "%s @ %u MHz %s",
                 formatMB(ESP.getFlashChipSize()).c_str(),
                 (unsigned)(ESP.getFlashChipSpeed() / 1000000), mode);
        kv("Chip", chipBuf);

        // LittleFS：只挂载一次，begin(false) 不自动格式化
        // 避免首次启动在 SPIFFS 分区上强制格式化 ~3.5MB 触发任务看门狗
        static bool s_lfsTried = false;
        static bool s_lfsOk = false;
        if (!s_lfsTried)
        {
            s_lfsTried = true;
            s_lfsOk = LittleFS.begin(false);
        }
        if (s_lfsOk)
        {
            size_t fsTotal = LittleFS.totalBytes();
            size_t fsUsed = LittleFS.usedBytes();
            float fsPct = fsTotal ? fsUsed * 100.0f / fsTotal : 0;
            barLine("LittleFS", fsPct, formatKB(fsUsed) + " / " + formatKB(fsTotal));

            // Files (最多列出 5 个)
            // 用嵌套作用域强制 fs::File 在此处析构，避免与 LittleFS 全局状态生命周期冲突
            String files;
            int count = 0;
            {
                fs::File root = LittleFS.open("/");
                if (root)
                {
                    fs::File file = root.openNextFile();
                    while (file)
                    {
                        if (count > 0)
                            files += ", ";
                        files += String(file.name()) + "(" + formatKB(file.size()) + ")";
                        file = root.openNextFile();
                        count++;
                        if (count >= 5)
                        {
                            files += " …";
                            break;
                        }
                    }
                }
            }
            kv("Files", count == 0 ? "(empty)" : files);
            // 不调 LittleFS.end()：保持挂载，避免析构访问已释放对象
        }
        else
        {
            kv("LittleFS", "Not mounted");
        }
    }

    void printNetworkInfo()
    {
        section("NETWORK");
        if (!WiFi.isConnected())
        {
            kv("WiFi", String(col(C_BAD)) + "● Disconnected" + col(C_RESET));
            kv("MAC", WiFi.macAddress()); // 即使未连接也能拿到
            return;
        }
        kv("WiFi", String(col(C_OK)) + "● Connected" + col(C_RESET));

        int rssi = WiFi.RSSI();
        char b[80];
        snprintf(b, sizeof(b), "%s  ch %d · %s %d dBm",
                 WiFi.SSID().c_str(), WiFi.channel(),
                 rssiBars(rssi).c_str(), rssi);
        kv("SSID", b);
        kv("IP", WiFi.localIP().toString() + " / " + WiFi.subnetMask().toString());
        kv("Gateway", WiFi.gatewayIP().toString());
        kv("MAC", WiFi.macAddress());

        IPAddress dns1 = WiFi.dnsIP(0);
        if (dns1)
            kv("DNS", dns1.toString());
    }

    void printSystemInfo()
    {
        section("SYSTEM");
        kv("Uptime", formatUptime(millis() / 1000));
        kv("Reset", resetReasonStr(esp_reset_reason()));

        // 温度（部分批次 S3 不支持，读到 NaN 会显示 nan）
        float t = temperatureRead();
        if (!isnan(t))
        {
            kv("Temp", String(t, 1) + " °C");
        }

        kv("Tasks", String(uxTaskGetNumberOfTasks()) + " active (FreeRTOS)");
    }

public:
    MemoryMonitor() {}

    void disableColor() { _color = false; }
    void enableColor() { _color = true; }

    void printMemoryUsage()
    {
        printBanner();
        printChipInfo();
        printMemoryInfo();
        printFlashInfo();
        printNetworkInfo();
        printSystemInfo();
        Serial.println();
        Serial.printf("%s━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━%s\n\n",
                      col(C_TITLE), col(C_RESET));
    }
};

#endif // MEMORYMONITOR_H
