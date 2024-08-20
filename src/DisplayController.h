#ifndef DISPLAYCONTROLLER_H
#define DISPLAYCONTROLLER_H

#include <TFT_eSPI.h>
#include <vector>

#define TFT_BL 22 // 默认值

#include "PinController.h"

class DisplayController {
public:
    static DisplayController &getInstance()
    {
        static DisplayController instance;
        return instance;
    }

    void init();
    void drawLoadingScreen(byte loadNum);
    void updateStarField(int numStars, int size, int speed, uint16_t color);

private:
    DisplayController();
    ~DisplayController();
    DisplayController(const DisplayController &) = delete;
    DisplayController &operator=(const DisplayController &) = delete;

    TFT_eSPI tft;
    TFT_eSprite sprite; // 使用TFT_eSprite作为离屏缓冲区
    struct Star {
        int x, y;
        int size;
        uint16_t color;
        int speedX, speedY;
    };
    std::vector<Star> stars;
};

#endif // DISPLAYCONTROLLER_H
