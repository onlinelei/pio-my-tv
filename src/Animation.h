#ifndef ANIMATION_H
#define ANIMATION_H

#include "DisplayController.h"

class Animation {
public:
    Animation(DisplayController& display);
    void lodingPage();
    void runStarField(); // 新增运行星空的函数

private:
    DisplayController& display;
    byte loadNum;
};

#endif // ANIMATION_H
