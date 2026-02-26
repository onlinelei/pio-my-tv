#include "PinController.h"

PinController::PinController() {}

PinController::~PinController() {}

void PinController::init() {}

void PinController::tftInitBackLightPin(uint16_t pin)
{
    pinMode(pin, OUTPUT);
    analogWrite(pin, 255);
}