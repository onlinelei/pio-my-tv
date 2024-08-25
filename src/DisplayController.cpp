#include <Arduino.h>

#include "DisplayController.h"
#include "PinController.h"

DisplayController::DisplayController()
    : tft(TFT_eSPI()), sprite(&tft)
{
}
DisplayController::~DisplayController()
{
}

void DisplayController::init() {
    // PinController::getInstance().tftInitBackLightPin(TFT_BL);
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

void DisplayController::drawLoadingScreen(byte loadNum)
{
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

void DisplayController::updateStarField(int numStars, int size, int speed, uint16_t color) {
    if (stars.size() != numStars){
        stars.resize(numStars); // 调整 stars 的大小
        for (int i = 0; i < numStars; i++) {
            stars[i].x = random(0, 240);
            stars[i].y = random(0, 240);
            stars[i].size = random(1, size);

            // 提取 RGB 分量
            uint8_t r = (color >> 11) & 0x1F;
            uint8_t g = (color >> 5) & 0x3F;
            uint8_t b = color & 0x1F;

            float factor = (float)random(10000) / 10000.0 * (3);
            // 调整亮度
            r = (uint8_t)(r * factor);
            g = (uint8_t)(g * factor);
            b = (uint8_t)(b * factor);

            // 重新组合颜色
            stars[i].color = ((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b & 0x1F);
            do {
                stars[i].speedX = random(-speed, speed);
            } while (stars[i].speedX == 0);
            do {
                stars[i].speedY = random(-speed, speed);
            } while (stars[i].speedY == 0);
        }
    }
    for (int i = 0; i < numStars; i++) {
        stars[i].x += stars[i].speedX;
        stars[i].y += stars[i].speedY;

        // 使用三元运算符简化位置更新逻辑
        stars[i].x = (stars[i].x < 0) ? 240 : (stars[i].x > 240) ? 0 : stars[i].x;
        stars[i].y = (stars[i].y < 0) ? 240 : (stars[i].y > 240) ? 0 : stars[i].y;
    }
    sprite.fillSprite(TFT_BLACK);
    for (int i = 0; i < numStars; i++) {
        if (stars[i].size < 2) {
            sprite.drawPixel(stars[i].x, stars[i].y, stars[i].color); // 绘制一个点
        } else {
            sprite.fillCircle(stars[i].x, stars[i].y, stars[i].size, stars[i].color); // 绘制一个圆
        }
    }
    sprite.pushSprite(0, 0); // 将缓冲区内容推送到TFT屏幕
}

void DisplayController::drawString(String text, int x, int y, uint16_t color)
{
    sprite.fillSprite(TFT_BLACK);
    sprite.setTextDatum(TL_DATUM);
    sprite.setTextColor(TFT_GREEN, TFT_BLACK);
    sprite.drawString(text, x, y, 2);
    sprite.setTextColor(color, TFT_BLACK);
    sprite.drawString(text, x, y + 10, 2);
    sprite.pushSprite(0, 0); // 将缓冲区内容推送到TFT屏幕
}
