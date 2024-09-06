#ifndef WIFI_SINGLETON_H
#define WIFI_SINGLETON_H

#include <WiFi.h>

#ifndef WIFI_SSID
#define WIFI_SSID "your_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your_PASSWORD"
#endif

class WiFi_Singleton
{
public:
    static WiFi_Singleton &getInstance()
    {
        static WiFi_Singleton instance;
        return instance;
    }

    void init()
    {
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }

    bool isConnected()
    {
        return WiFi.status() == WL_CONNECTED;
    }

    void reconnect()
    {
        if (!isConnected())
        {
            Serial.println("Reconnecting to WiFi...");
            WiFi.disconnect();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            while (WiFi.status() != WL_CONNECTED)
            {
                delay(500);
                Serial.print(".");
            }
            Serial.println("");
            Serial.println("WiFi reconnected");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
        }
    }

private:
    WiFi_Singleton() {}
    ~WiFi_Singleton() {}
    WiFi_Singleton(const WiFi_Singleton &) = delete;
    WiFi_Singleton &operator=(const WiFi_Singleton &) = delete;
};

#endif // WIFI_SINGLETON_H
