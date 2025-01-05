#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

#include "LVGLService.h"
#include "MQTTService.h"
#include "OTAService.h"
#include "SystemInfoMonitor.h"
#include "TaskBase.h"
#include "UIUpdateService.h"
#include "WiFiService.h"
#include "store/DataStore.h"
#include "ui/vars.h"

class TaskManager
{
private:
 static TaskManager* instance;
 TaskManager() = default;

 // 使用 enum 定义任务参数，提高可读性
 enum TaskParams {
     // 任务栈大小
     STACK_SIZE = 4096,

     // 任务核心 0 和 1 分别对应 Core 0 和 Core 1
     GUI_CORE_0 = 0,
     GUI_CORE_1 = 1,

     // 任务优先级 数字越大优先级越高
     PRIORITY_1 = 1,
     PRIORITY_2 = 2
 };

 void createTask(const char* name,
                 uint32_t delay_ms,
                 uint32_t stack = STACK_SIZE,
                 uint8_t priority = PRIORITY_1,
                 int8_t core = GUI_CORE_0,
                 std::function<void()> func = nullptr) {
     auto task = new TaskBase(delay_ms, func);
     task->createTask(name, stack, priority, core);
 }

public:
 static TaskManager& getInstance() {
     if (instance == nullptr) {
         instance = new TaskManager();
     }
     return *instance;
 }

 void begin() {
     // GUI任务，刷新UI - 5ms, Core 1
     createTask("GuiTask", 5, STACK_SIZE, PRIORITY_2, GUI_CORE_1, []() {
         LVGLService::getInstance().loop();
     });

     // UI更新任务，更新UI数据 - 10ms
     createTask("UIValueUpdateTask", 10, STACK_SIZE, PRIORITY_2, GUI_CORE_1, []() {
         UIUpdateService::getInstance().updateAll();
     });

     // WiFi任务 - 5s
     createTask("WiFiTask", 5000, STACK_SIZE, PRIORITY_2, GUI_CORE_0, []() {
         WiFiService::getInstance().loop();
     });

     // MQTT任务 - 10ms
     createTask("MQTTTask", 10, STACK_SIZE, PRIORITY_1, GUI_CORE_0, []() {
         if (WiFiService::getInstance().isConnected()) {
             MQTTService::getInstance().loop();
         }
     });

     // OTA任务 - 60s
     createTask("OTATask", 60000, STACK_SIZE, PRIORITY_1, GUI_CORE_0, []() {
         if (WiFiService::getInstance().isConnected()) {
             OTAService::getInstance().checkForUpdates();
         }
     });

     // 系统监控任务 - 5min
     createTask("MonitorTask", 5 * 60000, STACK_SIZE, PRIORITY_1, GUI_CORE_0, []() {
         SystemInfoMonitor::getInstance().printMemoryUsage(true);
     });
 }
};

TaskManager* TaskManager::instance = nullptr;

#endif // TASK_MANAGER_H