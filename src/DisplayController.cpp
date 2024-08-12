#include "DisplayController.h"
#include <Arduino.h>

#include "config/TFTConfig.h"


DisplayController::DisplayController()
    : tft(TFT_eSPI()), clk(&tft) {}

void DisplayController::init() {
    Serial.println("Initializing TFT...");
    tft.init();
    Serial.println("TFT initialized.");
    tft.invertDisplay(1);
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    createColors();
    tft.setTextColor(whiteColor, bgColor);
}

void DisplayController::createColors() {
    bgColor = tft.color565(0, 0, 0);
    whiteColor = tft.color565(245, 246, 247);
    greenColor = tft.color565(0, 255, 0);
}

void DisplayController::drawLoadingScreen(byte loadNum, uint16_t bgColor) {
    clk.setColorDepth(8);
    clk.createSprite(200, 100);
    clk.fillSprite(bgColor);
    clk.drawRoundRect(0, 0, 200, 16, 8, whiteColor);
    clk.fillRoundRect(3, 3, loadNum, 10, 5, whiteColor);
    clk.setTextDatum(CC_DATUM);
    clk.setTextColor(greenColor, bgColor);
    clk.drawString("Connecting to WiFi......", 100, 40, 2);
    clk.setTextColor(whiteColor, bgColor);
    clk.drawRightString("LOVE TV UI 2.0", 180, 60, 2);
    clk.pushSprite(20, 120);
    clk.deleteSprite();
}
