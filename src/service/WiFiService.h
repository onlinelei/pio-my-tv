#ifndef WIFI_SERVICE_H
#define WIFI_SERVICE_H

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"

class WiFiService {
   private:
    static WiFiService *instance;

    const char *ssid = WIFI_SSID;
    const char *password = WIFI_PASSWORD;
    bool wifiInitialized = false;
    bool wifiConnected = false;

    WiFiService() = default;

   public:
    static WiFiService &getInstance() {
        if (instance == nullptr) {
            instance = new WiFiService();
        }
        return *instance;
    }

    void loop() {
        if (!wifiInitialized) {
            Serial.println("Initializing WiFi...");
            WiFi.mode(WIFI_STA);
            wifiInitialized = true;
        }

        if (WiFi.status() != WL_CONNECTED) {
            if (wifiConnected) {
                wifiConnected = false;
                Serial.println("WiFi Connection Lost!");
            }
            WiFi.begin(ssid, password);
        } else {
            if (!wifiConnected) {
                wifiConnected = true;
                Serial.println("WiFi Connected!");
                Serial.println(WiFi.localIP());
            }
        }
    }

    bool isConnected() const {
        return wifiConnected;
    }
};

WiFiService *WiFiService::instance = nullptr;

#endif  // WIFI_SERVICE_H