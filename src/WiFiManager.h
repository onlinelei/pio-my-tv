#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include <WiFi.h>

#ifndef WIFI_SSID
#define WIFI_SSID "your_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_PASSWORD"
#endif

class WiFiManager
{
public:
    static WiFiManager &getInstance();
    void init();
    void connect();

private:
    WiFiManager() {}
    ~WiFiManager() {}
    WiFiManager(const WiFiManager &) = delete;
    WiFiManager &operator=(const WiFiManager &) = delete;
};

#endif // WIFIMANAGER_H
