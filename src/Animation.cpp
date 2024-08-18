#include "Animation.h"

Animation::Animation(DisplayController& display)
    : display(display), loadNum(1) {}

void Animation::lodingPage() {
    while (loadNum < 194) {
        display.drawLoadingScreen(loadNum);
        delay(100);
        loadNum += 1;
    }
}

void Animation::runStarField() {
    display.updateStarField();
}
