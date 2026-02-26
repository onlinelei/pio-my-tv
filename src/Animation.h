#ifndef ANIMATION_H
#define ANIMATION_H

#include "DisplayController.h"

class Animation {
public:
    static Animation &getInstance()
    {
        static Animation instance;
        return instance;
    }
    void lodingPage();
    void runStarField(int numStars, int size, int speed, uint16_t color);
    void runStarFieldAuto();

private:
    Animation();
    ~Animation();
    Animation(const Animation &) = delete;
    Animation &operator=(const Animation &) = delete;

    byte loadNum;
    int m_numStars;
    int m_size;
    uint16_t m_color;
    int m_count;
    int m_speed;
};

#endif // ANIMATION_H
