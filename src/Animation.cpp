
#include "Animation.h"

Animation::Animation()
    : loadNum(1), m_numStars(300), m_size(3), m_color(TFT_RED), m_count(100), m_speed(5) {}
Animation::~Animation()
{
}

void Animation::lodingPage() {
    while (loadNum < 194) {
        DisplayController::getInstance().drawLoadingScreen(loadNum);
        delay(100);
        loadNum += 1;
    }
}

void Animation::runStarField(int numStars, int size, int speed, uint16_t color) {
    DisplayController::getInstance().updateStarField(numStars, size, speed, color);
}

void Animation::runStarFieldAuto()
{
    uint16_t colors[] = {
        TFT_NAVY, TFT_DARKGREEN, TFT_DARKCYAN, TFT_MAROON, TFT_PURPLE,
        TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE, TFT_GREEN, TFT_CYAN,
        TFT_RED, TFT_MAGENTA, TFT_YELLOW, TFT_WHITE, TFT_ORANGE, TFT_GREENYELLOW,
        TFT_PINK
    };

    if (m_count <= 0)
    {
        m_count = 20;
        m_numStars = random(100) + 300;
        m_color = colors[random(17)];
    }
    runStarField(m_numStars, m_size, m_speed, m_color); // 调用绘制星空的函数
    m_count -= 1;
}
