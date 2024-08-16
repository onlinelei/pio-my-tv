
#ifndef PINCONTROLLER_H
#define PINCONTROLLER_H

#include <Arduino.h>

class PinController {
public:
    PinController();
    void init();
    void tftInitBackLightPin(uint16_t pin); 

private:
    
};

#endif // PINCONTROLLER_H