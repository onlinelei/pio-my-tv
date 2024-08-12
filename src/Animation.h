// Animation.h
#ifndef ANIMATION_H
#define ANIMATION_H

#include "DisplayController.h"

class Animation {
public:
    Animation(DisplayController& display);
    void run();

private:
    DisplayController& display;
    byte loadNum;
};

#endif // ANIMATION_H
