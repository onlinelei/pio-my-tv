#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "store/DataStore.h"

// MQTT消息结构体
struct MQTTMessage {
    long timestamp;  // Unix时间戳
    String command;  // 可选的命令字段
    String data;     // 可选的数据字段

    // 从JSON字符串解析
    bool fromJson(const String &jsonString) {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, jsonString);

        if (error) {
            Serial.println("JSON解析失败");
            return false;
        }

        // 解析时间戳（必需字段）
        if (!doc.containsKey("timestamp")) {
            Serial.println("缺少timestamp字段");
            return false;
        }
        timestamp = doc["timestamp"].as<long>();

        // 解析可选字段
        command = doc.containsKey("command") ? doc["command"].as<String>() : "";
        data = doc.containsKey("data") ? doc["data"].as<String>() : "";

        return true;
    }

    // 转换为JSON字符串
    String toJson() const {
        StaticJsonDocument<200> doc;
        doc["timestamp"] = timestamp;
        if (command.length() > 0) doc["command"] = command;
        if (data.length() > 0) doc["data"] = data;

        String output;
        serializeJson(doc, output);
        return output;
    }
};

class MQTTService {
   private:
    static MQTTService *instance;
    WiFiClient espClient;
    PubSubClient client;

    // MQTT配置
    const char *mqtt_server = "okeng.top";
    const int mqtt_port = 1883;
    const char *mqtt_user = "ray";
    const char *mqtt_password = "123456";

    // 重连延时
    unsigned long lastReconnectAttempt = 0;
    const unsigned long RECONNECT_DELAY = 5000;  // 5秒

    MQTTService() : client(espClient) {
        client.setServer(mqtt_server, mqtt_port);
        client.setCallback([this](char *topic, byte *payload, unsigned int length) {
            this->callback(topic, payload, length);
        });
    }

    // MQTT消息回调
    void callback(char *topic, byte *payload, unsigned int length) {
        String message;
        for (int i = 0; i < length; i++) {
            message += (char)payload[i];
        }

        Serial.printf("收到消息 [%s]: %s\n", topic, message.c_str());

        MQTTMessage mqttMsg;
        if (mqttMsg.fromJson(message)) {
            // 设置时区信息（只需要在系统启动时设置一次）
            static bool timeZoneSet = false;
            if (!timeZoneSet) {
                setenv("TZ", "CST-8", 1);  // 设置为东八区
                tzset();
                timeZoneSet = true;
            }

            // 更新系统时间
            struct timeval tv;
            tv.tv_sec = mqttMsg.timestamp;
            tv.tv_usec = 0;
            settimeofday(&tv, NULL);

            // 只更新数据存储
            DataStore::getInstance().updateTimestamp(mqttMsg.timestamp);
            if (mqttMsg.data.length() > 0) {
                DataStore::getInstance().updateMessage(mqttMsg.data);
            }
        }
    }

    // 重连
    bool reconnect() {
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println("MQTT Connected");
            // 订阅所有主题
            client.subscribe("#");
            return true;
        }
        return false;
    }

   public:
    static MQTTService &getInstance() {
        if (instance == nullptr) {
            instance = new MQTTService();
        }
        return *instance;
    }

    void loop() {
        if (!client.connected()) {
            unsigned long now = millis();
            if (now - lastReconnectAttempt > RECONNECT_DELAY) {
                lastReconnectAttempt = now;
                if (reconnect()) {
                    lastReconnectAttempt = 0;
                }
            }
        } else {
            client.loop();
        }
    }

    bool publish(const char *topic, const char *message) {
        return client.publish(topic, message);
    }
};

MQTTService *MQTTService::instance = nullptr;
#endif  // MQTT_SERVICE_H