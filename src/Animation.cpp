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

void Animation::runStarField(int numStars, int size, uint16_t color) {
    display.updateStarField(numStars, size, color); 
}
