#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

#define TFT_BL 22  // 默认值

#include <TFT_eSPI.h>

#include "PinController.h"

class DisplayController {
public:
    DisplayController(PinController& pinController);
    void init();
    void drawLoadingScreen(byte loadNum, uint16_t bgColor);

private:
    TFT_eSPI tft;
    TFT_eSprite clk;
    PinController& pinController; 
    uint16_t bgColor, whiteColor, greenColor;
    void createColors();
};

#endif // DISPLAYCONTROLLER_H