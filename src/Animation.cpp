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

void Animation::runStarField(int numStars, int size, int speed, uint16_t color) {
    display.updateStarField(numStars, size, speed, color); 
}

void Animation::runStarFieldAuto(int* numStars, int* size, int* speed, uint16_t* color, int* count){
    uint16_t colors[] = {
        TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON, TFT_PURPLE,
        TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE, TFT_GREEN, TFT_CYAN,
        TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE, TFT_ORANGE, TFT_GREENYELLOW,
        TFT_PINK
    };

    if(*count <= 0) {
        *count = 20;
        *numStars = random(100) + 300;
        *color = colors[random(17)];
    }
    runStarField(*numStars, *size, *speed, *color); // 调用绘制星空的函数    
    *count -= 1;
}
