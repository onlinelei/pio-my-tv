
#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

#include <TFT_eSPI.h>

class DisplayController {
public:
    DisplayController();
    void init();
    void drawLoadingScreen(byte loadNum, uint16_t bgColor);

private:
    TFT_eSPI tft;
    TFT_eSprite clk;
    uint16_t bgColor, whiteColor, greenColor;
    void createColors();
};

#endif // DISPLAYCONTROLLER_H