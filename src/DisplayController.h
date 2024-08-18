#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

#define TFT_BL 22  // 默认值

#include <TFT_eSPI.h>

#include "PinController.h"

class DisplayController {
public:
    DisplayController(PinController& pinController);
    void init();
    void drawLoadingScreen(byte loadNum);
    void drawStarField(); // 新增绘制星空的函数
    void updateStarField(); // 新增更新星空的函数

private:
    TFT_eSPI tft;
    TFT_eSprite sprite; // 使用TFT_eSprite作为离屏缓冲区
    PinController& pinController; 
    struct Star {
        int x, y;
        int size;
        int brightness;
        int speedX, speedY;
    };
    Star stars[100];
    void initStars();
};

#endif // DISPLAYCONTROLLER_H
