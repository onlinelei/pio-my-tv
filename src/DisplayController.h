#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

#define TFT_BL 22  // 默认值

#include <TFT_eSPI.h>
#include <vector> // 添加 std::vector 的头文件

#include "PinController.h"

class DisplayController {
public:
    DisplayController(PinController& pinController);
    void init();
    void drawLoadingScreen(byte loadNum);
    void updateStarField(int numStars, int size, int speed, uint16_t color);

private:
    TFT_eSPI tft;
    TFT_eSprite sprite; // 使用TFT_eSprite作为离屏缓冲区
    PinController& pinController; 
    struct Star {
        int x, y;
        int size;
        uint16_t color;
        int speedX, speedY;
    };
    std::vector<Star> stars;
};

#endif // DISPLAYCONTROLLER_H
