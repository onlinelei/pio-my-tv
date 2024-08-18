#ifndef ANIMATION_H
#define ANIMATION_H

#include "DisplayController.h"

class Animation {
public:
    Animation(DisplayController& display);
    void lodingPage();
    void runStarField(int numStars, int size, uint16_t color);

private:
    DisplayController& display;
    byte loadNum;
};

#endif // ANIMATION_H
