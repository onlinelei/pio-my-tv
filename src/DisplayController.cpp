#include <Arduino.h>

#include "DisplayController.h"
#include "PinController.h"

DisplayController::DisplayController(PinController& pinController)
    : tft(TFT_eSPI()), sprite(&tft), pinController(pinController) {
    initStars();
}

void DisplayController::init() {
    pinController.tftInitBackLightPin(TFT_BL);
    Serial.println("Initializing TFT...");
    tft.init();
    Serial.println("TFT initialized.");
    tft.invertDisplay(1);
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    // 初始化TFT_eSprites
    sprite.setColorDepth(8);
    sprite.createSprite(240, 240);
}

void DisplayController::drawLoadingScreen(byte loadNum) {
    sprite.createSprite(200, 100);
    sprite.fillSprite(TFT_BLACK);
    sprite.drawRoundRect(0, 0, 200, 16, 8, TFT_WHITE);
    sprite.fillRoundRect(3, 3, loadNum, 10, 5, TFT_WHITE);
    sprite.setTextDatum(CC_DATUM);
    sprite.setTextColor(TFT_GREEN, TFT_BLACK);
    sprite.drawString("Connecting to WiFi......", 100, 40, 2);
    sprite.setTextColor(TFT_WHITE, TFT_BLACK);
    sprite.drawRightString("LOVE TV UI 2.0", 180, 60, 2);
    sprite.pushSprite(20, 120);
}

void DisplayController::drawStarField() {
    
    for (int i = 0; i < 100; i++) {
        sprite.fillCircle(stars[i].x, stars[i].y, stars[i].size, sprite.color565(stars[i].brightness, stars[i].brightness, stars[i].brightness));
    }
}

void DisplayController::updateStarField() {
    for (int i = 0; i < 100; i++) {
        stars[i].x += stars[i].speedX;
        stars[i].y += stars[i].speedY;

        if (stars[i].x < 0 || stars[i].x >= 240) {
            stars[i].x = random(0, 240);
            stars[i].y = random(0, 240);
            stars[i].speedX = random(-1, 2);
            stars[i].speedY = random(-1, 2);
        }
        if (stars[i].y < 0 || stars[i].y >= 240) {
            stars[i].x = random(0, 240);
            stars[i].y = random(0, 240);
            stars[i].speedX = random(-1, 2);
            stars[i].speedY = random(-1, 2);
        }
    }
    sprite.fillSprite(TFT_BLACK);
    drawStarField();
    sprite.pushSprite(0, 0); // 将缓冲区内容推送到TFT屏幕
}


void DisplayController::initStars() {
    for (int i = 0; i < 100; i++) {
        stars[i].x = random(0, 240);
        stars[i].y = random(0, 240);
        stars[i].size = random(1, 3);
        stars[i].brightness = random(128, 255);
        stars[i].speedX = random(-1, 2);
        stars[i].speedY = random(-1, 2);
    }
}
