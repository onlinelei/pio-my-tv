#ifndef ANIMATION_H
#define ANIMATION_H

#include "DisplayController.h"

class Animation {
public:
    Animation(DisplayController& display);
    void lodingPage();
    void runStarField(int numStars, int size, int speed, uint16_t color);
    void runStarFieldAuto(int* numStars, int* size, int* speed, uint16_t* color, int* count);

private:
    DisplayController& display;
    byte loadNum;
};

#endif // ANIMATION_H
