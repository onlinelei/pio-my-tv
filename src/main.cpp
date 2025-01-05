#include <Arduino.h>

#include "service/LVGLService.h"
#include "service/OTAService.h"
#include "service/TaskManager.h"

void setup() {
    Serial.begin(115200);

    // 初始化LVGL
    LVGLService::getInstance().setup();

    // 初始化OTA服务
    OTAService::getInstance().begin("http://ota.okeng.top");

    // 启动任务管理器
    TaskManager::getInstance().begin();
}

void loop() {
    // 主循环为空，所有任务由FreeRTOS管理
    vTaskDelay(portMAX_DELAY);
}