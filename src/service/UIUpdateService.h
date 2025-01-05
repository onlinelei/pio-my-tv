#ifndef UI_UPDATE_SERVICE_H
#define UI_UPDATE_SERVICE_H

#include <Arduino.h>

#include "store/DataStore.h"
#include "ui/vars.h"

class UIUpdateService {
   private:
    static UIUpdateService* instance;
    UIUpdateService() = default;

    // 更新时间显示
    void updateDateTime() {
        time_t now;
        time(&now);
        struct tm* timeinfo = localtime(&now);

        char dateStr[16];
        char timeStr[16];

        strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", timeinfo);
        strftime(timeStr, sizeof(timeStr), "%H:%M:%S", timeinfo);

        set_var_page_1_var_date(dateStr);
        set_var_page_1_var_time(timeStr);
    }

    // 更新消息显示
    void updateMessage() {
        const String& lastMessage = DataStore::getInstance().getLastMessage();
        if (lastMessage.length() > 0) {
            set_var_page_1_var_message(lastMessage.c_str());
        }
    }

    // 这里可以添加其他UI更新方法
    // void updateOtherFields() { ... }

   public:
    static UIUpdateService& getInstance() {
        if (instance == nullptr) {
            instance = new UIUpdateService();
        }
        return *instance;
    }

    // 更新所有UI数据
    void updateAll() {
        updateDateTime();
        updateMessage();
        // updateOtherFields();  // 未来可能的其他更新
    }
};

UIUpdateService* UIUpdateService::instance = nullptr;

#endif  // UI_UPDATE_SERVICE_H