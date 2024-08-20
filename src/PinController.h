#ifndef PINCONTROLLER_H
#define PINCONTROLLER_H

#include <Arduino.h>

class PinController {
public:
    static PinController &getInstance()
    {
        static PinController instance;
        return instance;
    }

    void init();
    void tftInitBackLightPin(uint16_t pin);

private:
    PinController();
    ~PinController();
    PinController(const PinController &) = delete;            // 禁用拷贝构造函数
    PinController &operator=(const PinController &) = delete; // 禁用赋值操作符
};

#endif // PINCONTROLLER_H