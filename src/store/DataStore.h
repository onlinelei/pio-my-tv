#ifndef DATA_STORE_H
#define DATA_STORE_H

#include <Arduino.h>

class DataStore {
   private:
    static DataStore* instance;

    // 存储最新的数据
    long lastTimestamp;
    String lastMessage;

    DataStore() : lastTimestamp(0) {}

   public:
    static DataStore& getInstance() {
        if (instance == nullptr) {
            instance = new DataStore();
        }
        return *instance;
    }

    // 更新数据
    void updateTimestamp(long timestamp) {
        lastTimestamp = timestamp;
    }

    void updateMessage(const String& message) {
        lastMessage = message;
    }

    // 获取数据
    long getLastTimestamp() const {
        return lastTimestamp;
    }

    const String& getLastMessage() const {
        return lastMessage;
    }
};

DataStore* DataStore::instance = nullptr;

#endif