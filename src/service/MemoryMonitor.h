#ifndef MEMORYMONITOR_H
#define MEMORYMONITOR_H

#include <Arduino.h>
#include <esp_heap_caps.h>
#include <esp_spi_flash.h>
#include <esp_system.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <FS.h>

class MemoryMonitor
{
private:
    // 常量定义
    static const int BUFFER_SIZE = 128;
    static const int TABLE_WIDTH = 49;
    static const int CONTENT_WIDTH = 47;
    static const int COLUMN_WIDTH = 22;

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

    // 打印表格行
    void printRow(const char *name, const char *value)
    {
        char buffer[BUFFER_SIZE];
        snprintf(buffer, sizeof(buffer), "║ %*s | %-*s ║",
                 COLUMN_WIDTH, name, COLUMN_WIDTH, value);
        Serial.println(buffer);
    }

    // 打印表头
    void printHeader(const char *title)
    {
        Serial.println("╔═════════════════════════════════════════════════╗");
        printEmptyRow();
        printCenteredTitle(title);
        printEmptyRow();
    }

    // 打印分隔符
    void printSeparator(const char *title)
    {
        printEmptyRow();
        Serial.println("╠═════════════════════════════════════════════════╣");
        printEmptyRow();
        printCenteredTitle(title);
        printEmptyRow();
    }

    // 打印页脚
    void printFooter()
    {
        printEmptyRow();
        Serial.println("╚═════════════════════════════════════════════════╝");
    }

    // 打印空行
    void printEmptyRow()
    {
        Serial.println("║                                                 ║");
    }

    // 打印居中标题
    void printCenteredTitle(const char *title)
    {
        char buffer[BUFFER_SIZE];
        int titleLength = strlen(title);
        int leftPadding = (TABLE_WIDTH - titleLength) / 2;
        int rightPadding = TABLE_WIDTH - titleLength - leftPadding;
        snprintf(buffer, sizeof(buffer), "║%*s%s%*s║", leftPadding, "", title, rightPadding, "");
        Serial.println(buffer);
    }

    // 打印内存信息
    void printHeapInfo()
    {
        auto freeHeap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        auto totalHeap = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
        auto minFreeHeap = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
        auto usedHeap = totalHeap - freeHeap;

        printRow("Total Heap", formatSize(totalHeap).c_str());
        printRow("Used Heap", formatSize(usedHeap).c_str());
        printRow("Free Heap", formatSize(freeHeap).c_str());
        printRow("Min Free Ever", formatSize(minFreeHeap).c_str());
        printRow("Memory Usage", formatPercentage(usedHeap, totalHeap).c_str());
    }

    // 打印IRAM信息
    void printIRAMInfo()
    {
        auto freeIram = heap_caps_get_free_size(MALLOC_CAP_EXEC);
        auto totalIram = heap_caps_get_total_size(MALLOC_CAP_EXEC);
        auto usedIram = totalIram - freeIram;

        printRow("Total IRAM", formatSize(totalIram).c_str());
        printRow("Free IRAM", formatSize(freeIram).c_str());
        printRow("Memory Usage", formatPercentage(usedIram, totalIram).c_str());
    }

    // 打印内存碎片信息
    void printFragmentationInfo()
    {
        multi_heap_info_t info;
        heap_caps_get_info(&info, MALLOC_CAP_INTERNAL);

        printRow("Largest Free Block", formatSize(info.largest_free_block).c_str());
        printRow("Total Blocks", String(info.total_blocks).c_str());
        printRow("Allocated Blocks", String(info.allocated_blocks).c_str());
        printRow("Free Blocks", String(info.free_blocks).c_str());
        printRow("Block Usage", formatPercentage(info.allocated_blocks, info.total_blocks).c_str());
    }

    // 打印系统内存详情
    void printSystemMemoryInfo()
    {
        auto total8bit = heap_caps_get_total_size(MALLOC_CAP_8BIT);
        auto free8bit = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        auto used8bit = total8bit - free8bit;
        auto totalDMA = heap_caps_get_total_size(MALLOC_CAP_DMA);
        auto freeDMA = heap_caps_get_free_size(MALLOC_CAP_DMA);

        printRow("Total 8-bit RAM", formatSize(total8bit).c_str());
        printRow("Free 8-bit RAM", formatSize(free8bit).c_str());
        printRow("Memory Usage", formatPercentage(used8bit, total8bit).c_str());
        printRow("Total DMA Memory", formatSize(totalDMA).c_str());
        printRow("Free DMA Memory", formatSize(freeDMA).c_str());
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

        printFileList();
        LittleFS.end();
    }

    // 打印文件列表
    void printFileList()
    {
        fs::File root = LittleFS.open("/");
        fs::File file = root.openNextFile();
        int fileCount = 0;

        printSeparator("Files in LittleFS");
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

public:
    MemoryMonitor() {}

    void printMemoryUsage()
    {
        printHeader("Esp32 Info");

        printSeparator("Heap Memory Usage");
        printHeapInfo();

        printSeparator("IRAM Memory Usage");
        printIRAMInfo();

        printSeparator("Memory Fragmentation");
        printFragmentationInfo();

        printSeparator("System Memory Details");
        printSystemMemoryInfo();

        printSeparator("Flash and System Info");
        printFlashInfo();

        printSeparator("Flash File System (LittleFS)");
        printFileSystemInfo();

        printSeparator("CPU Information");
        printCPUInfo();

        printSeparator("WiFi Information");
        printWiFiInfo();

        printSeparator("Additional System Info");
        printAdditionalSystemInfo();

        printFooter();
    }
};

#endif // MEMORYMONITOR_H