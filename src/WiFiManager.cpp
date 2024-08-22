#include <WiFi.h>
#include "WiFiManager.h"
#include "DisplayController.h"

WiFiManager &WiFiManager::getInstance()
{
    static WiFiManager instance;
    return instance;
}
void WiFiManager::init()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiManager::connect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        DisplayController::getInstance().drawString("Connecting to WiFi...", 10, 10, TFT_WHITE);
    }
    DisplayController::getInstance().drawString("WiFi connected", 10, 40, TFT_WHITE);
    DisplayController::getInstance().drawString("IP address: " + WiFi.localIP(), 10, 80, TFT_WHITE);
}
