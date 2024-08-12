// Animation.cpp
#include "Animation.h"

Animation::Animation(DisplayController& display)
    : display(display), loadNum(1) {}

void Animation::run() {
    for (size_t i = 0; i < 100; i++) {
        display.drawLoadingScreen(loadNum, 0x0000);
        delay(30);
    }

    while (loadNum < 194) {
        display.drawLoadingScreen(loadNum, 0x0000);
        delay(100);
        loadNum += 1;
    }
}
