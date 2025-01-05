#ifndef SYSTEMINFOMONITOR_H
#define SYSTEMINFOMONITOR_H

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <esp_spi_flash.h>
#include <esp_system.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <FS.h>

class SystemInfoMonitor
{
private:
    static SystemInfoMonitor *instance;

    // 私有构造函数
    SystemInfoMonitor() {}

    // 常量定义
    static const int BUFFER_SIZE = 128;
    static const int TABLE_WIDTH = 49;
    static const int CONTENT_WIDTH = 47;
    static const int COLUMN_WIDTH = 22;

    boolean linPrint = false;

    // 格式化辅助函数
    static String formatSize(size_t bytes, int decimals = 1)
    {
        return String(bytes / 1024.0, decimals) + " KB";
    }

    static String formatPercentage(float value, float total, int decimals = 1)
    {
        return String((value * 100.0) / total, decimals) + "%";
    }

    // WiFi信号强度转换
    static int convertRSSIToQuality(int rssi)
    {
        if (rssi <= -100)
            return 0;
        if (rssi >= -50)
            return 100;
        return 2 * (rssi + 100);
    }
    // 格式化内存大小（转换为KB）
    String formatKB(uint32_t bytes)
    {
        float kb = bytes / 1024.0f;
        char buf[16];
        snprintf(buf, sizeof(buf), "%.2f KB", kb);
        return String(buf);
    }

    // 格式化内存大小（带小数点的KB）
    String formatKBWithDecimal(uint32_t bytes)
    {
        float kb = bytes / 1024.0;
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f KB", kb);
        return String(buf);
    }

    // 打印表格行
    void printRow(const char *name, const char *value)
    {
        char buffer[BUFFER_SIZE];
        if (linPrint)
        {
            snprintf(buffer, sizeof(buffer), " %*s | %-*s ", COLUMN_WIDTH, name, COLUMN_WIDTH, value);
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "║ %*s | %-*s ║", COLUMN_WIDTH, name, COLUMN_WIDTH, value);
        }
        Serial.println(buffer);
    }

    // 打印表头
    void printHeader(const char *title)
    {
        if (linPrint)
        {
            printEmptyRow();
            printCenteredTitle(title);
        }
        else
        {
            Serial.println("╔═════════════════════════════════════════════════╗");
            printEmptyRow();
            printCenteredTitle(title);
            printEmptyRow();
        }
    }

    // 打印分隔符
    void printSeparator(const char *title)
    {
        if (linPrint)
        {
            printEmptyRow();
            printCenteredTitle(title);
        }
        else
        {
            printEmptyRow();
            Serial.println("╠═════════════════════════════════════════════════╣");
            printEmptyRow();
            printCenteredTitle(title);
            printEmptyRow();
        }
    }

    // 打印页脚
    void printFooter()
    {
        if (linPrint)
        {
            printCenteredTitle("End of Report");
        }
        else
        {
            printEmptyRow();
            Serial.println("╚═════════════════════════════════════════════════╝");
        }
    }

    // 打印空行
    void printEmptyRow()
    {
        if (linPrint)
        {
            printCenteredTitle("");
        }
        else
        {
            Serial.println("║                                                 ║");
        }
    }

    // 打印居中标题
    void printCenteredTitle(const char *title)
    {
        char buffer[BUFFER_SIZE];
        int titleLength = strlen(title);
        int leftPadding = (TABLE_WIDTH - titleLength) / 2;
        int rightPadding = TABLE_WIDTH - titleLength - leftPadding;
        if (linPrint)
        {
            snprintf(buffer, sizeof(buffer), "%*s%s%*s", leftPadding, "", title, rightPadding, "");
        }
        else
        {
            snprintf(buffer, sizeof(buffer), "║%*s%s%*s║", leftPadding, "", title, rightPadding, "");
        }
        Serial.println(buffer);
    }

    // 打印系统内存详情
    void printSystemMemoryInfo()
    {
        // DRAM和Heap信息
        uint32_t dramTotal = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        uint32_t dramFree = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        uint32_t dramUsed = dramTotal - dramFree;

        uint32_t heapTotal = ESP.getHeapSize(); // 这实际上是DRAM中的堆区域
        uint32_t heapFree = ESP.getFreeHeap();
        uint32_t heapUsed = heapTotal - heapFree;

        printRow("DRAM Total", formatKB(dramTotal).c_str());
        printRow("DRAM Used", (formatKB(dramUsed) + " (" + formatPercentage(dramUsed, dramTotal) + ") ").c_str());
        printRow("-Heap Total", formatKB(heapTotal).c_str()); // Heap是DRAM的一部分
        printRow("--Heap Used", (formatKB(heapUsed) + " (" + formatPercentage(heapUsed, heapTotal) + ") ").c_str());
        printRow("--Heap Free", formatKB(heapFree).c_str());
        printRow("--Min Free", formatKB(ESP.getMinFreeHeap()).c_str());

        // IRAM信息
        uint32_t iramTotal = heap_caps_get_total_size(MALLOC_CAP_EXEC | MALLOC_CAP_32BIT);
        uint32_t iramFree = heap_caps_get_free_size(MALLOC_CAP_EXEC | MALLOC_CAP_32BIT);
        uint32_t iramUsed = iramTotal - iramFree;
        printRow("IRAM Total", "128 KB");
        printRow("IRAM Free", formatKB(iramFree).c_str());
        printRow("IRAM Used", (formatKB(iramUsed) + " (" + formatPercentage(iramUsed, iramTotal) + ")").c_str());

        // RTC RAM信息
        uint32_t rtcTotal = heap_caps_get_total_size(MALLOC_CAP_RTCRAM);
        uint32_t rtcFree = heap_caps_get_free_size(MALLOC_CAP_RTCRAM);
        uint32_t rtcUsed = rtcTotal - rtcFree;
        printRow("RTC RAM Total", formatKB(rtcTotal).c_str());
        printRow("RTC RAM Free", formatKBWithDecimal(rtcFree).c_str());
        printRow("RTC RAM Used", (formatKB(rtcUsed) + " (" + formatPercentage(rtcUsed, rtcTotal) + ")").c_str());
    }

    // 打印Flash信息
    void printFlashInfo()
    {
        float flashSize = ESP.getFlashChipSize() / 1024.0 / 1024.0;
        float flashSpeed = ESP.getFlashChipSpeed() / 1000000.0;
        size_t sketchSize = ESP.getSketchSize();
        size_t freeSketchSpace = ESP.getFreeSketchSpace();
        size_t totalSketchSpace = sketchSize + freeSketchSpace;

        printRow("Flash Size", (String(flashSize, 1) + " MB").c_str());
        printRow("Flash Speed", (String(flashSpeed, 1) + " MHz").c_str());
        printRow("Sketch Size", formatSize(sketchSize).c_str());
        printRow("Free Flash Space", formatSize(freeSketchSpace).c_str());
        printRow("Flash Usage", formatPercentage(sketchSize, totalSketchSpace).c_str());
    }

    // 打印文件系统信息
    void printFileSystemInfo()
    {
        if (!LittleFS.begin(true))
            return;

        size_t totalBytes = LittleFS.totalBytes();
        size_t usedBytes = LittleFS.usedBytes();

        printRow("Total Space", formatSize(totalBytes).c_str());
        printRow("Used Space", formatSize(usedBytes).c_str());
        printRow("Free Space", formatSize(totalBytes - usedBytes).c_str());
        printRow("Usage", formatPercentage(usedBytes, totalBytes).c_str());

        LittleFS.end();
    }

    // 打印文件列表
    void printFileList()
    {
        if (!LittleFS.begin(true))
            return;

        fs::File root = LittleFS.open("/");
        fs::File file = root.openNextFile();
        int fileCount = 0;

        while (file)
        {
            printRow(file.name(), formatSize(file.size()).c_str());
            file = root.openNextFile();
            fileCount++;
        }

        if (fileCount == 0)
        {
            printRow("Status", "No files found");
        }
    }

    // 打印CPU信息
    void printCPUInfo()
    {
        printRow("CPU Frequency", (String(ESP.getCpuFreqMHz()) + " MHz").c_str());
        printRow("CPU Cores", "2");
    }

    // 打印WiFi信息
    void printWiFiInfo()
    {
        if (!WiFi.isConnected())
        {
            printRow("WiFi Status", "Not Connected");
            return;
        }

        // 基本信息
        printRow("SSID", WiFi.SSID().c_str());
        printRow("Channel", String(WiFi.channel()).c_str());

        // 信号强度
        int rssi = WiFi.RSSI();
        int quality = convertRSSIToQuality(rssi);
        printRow("Signal Strength",
                 (String(quality) + "% (" + String(rssi) + " dBm)").c_str());

        // 网络信息
        printRow("Local IP", WiFi.localIP().toString().c_str());
        printRow("Subnet Mask", WiFi.subnetMask().toString().c_str());
        printRow("Gateway IP", WiFi.gatewayIP().toString().c_str());
        printRow("MAC Address", WiFi.macAddress().c_str());

        // DNS信息
        IPAddress dns1 = WiFi.dnsIP(0);
        IPAddress dns2 = WiFi.dnsIP(1);
        if (dns1)
            printRow("Primary DNS", dns1.toString().c_str());
        if (dns2)
            printRow("Secondary DNS", dns2.toString().c_str());
    }

    // 打印附加系统信息
    void printAdditionalSystemInfo()
    {
        float temperature = temperatureRead();
        printRow("Chip Temperature", (String(temperature, 1) + " deg C").c_str());

        esp_reset_reason_t resetReason = esp_reset_reason();
        printRow("Reset Reason", String(resetReason).c_str());

        uint64_t chipId = ESP.getEfuseMac();
        printRow("Chip ID",
                 (String((uint32_t)(chipId >> 32), HEX) +
                  String((uint32_t)chipId, HEX))
                     .c_str());

        unsigned long uptime = millis() / 1000;
        printRow("Uptime", (String(uptime) + " seconds").c_str());
    }

    // 格式化百分比，保留一位小数
    String formatPercentage(uint32_t used, uint32_t total)
    {
        float percentage = (used * 100.0f) / total;
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1f%%", percentage);
        return String(buf);
    }

public:
    static SystemInfoMonitor &getInstance()
    {
        if (instance == nullptr)
        {
            instance = new SystemInfoMonitor();
        }
        return *instance;
    }

    // 删除拷贝构造函数和赋值运算符
    SystemInfoMonitor(const SystemInfoMonitor &) = delete;
    SystemInfoMonitor &operator=(const SystemInfoMonitor &) = delete;

    void printMemoryUsage(bool linPrint)
    {
        this->linPrint = linPrint;
        printHeader("Esp32 Info");

        printSeparator("System Memory Details");
        printSystemMemoryInfo();

        printSeparator("Flash and System Info");
        printFlashInfo();

        printSeparator("Flash File System (LittleFS)");
        printFileSystemInfo();

        printSeparator("Files in LittleFS");
        printFileList();

        printSeparator("CPU Information");
        printCPUInfo();

        printSeparator("WiFi Information");
        printWiFiInfo();

        printSeparator("Additional System Info");
        printAdditionalSystemInfo();

        printFooter();
    }
};

// 初始化静态成员
SystemInfoMonitor *SystemInfoMonitor::instance = nullptr;

#endif // SYSTEMINFOMONITOR_H