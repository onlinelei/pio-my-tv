#include "ButtonHandler.h"

ButtonHandler::ButtonHandler()
{
    // 构造函数中不需要初始化 buttonRight 和 buttonLeft
}
ButtonHandler::~ButtonHandler()
{
    // 构造函数中不需要初始化 buttonRight 和 buttonLeft
}

void ButtonHandler::init(int pinRight, int pinLeft)
{
    buttonRight = Button2(pinRight);
    buttonLeft = Button2(pinLeft);

    buttonRight.setDoubleClickTime(500);
    buttonRight.setLongClickTime(1000);
    buttonRight.setClickHandler(rightsingleclick);
    buttonRight.setDoubleClickHandler(rightdoubleclick);
    buttonRight.setLongClickHandler(wifi_reset);

    buttonLeft.setDoubleClickTime(500);
    buttonLeft.setLongClickTime(1000);
    buttonLeft.setClickHandler(leftsingleclick);
    buttonLeft.setDoubleClickHandler(leftdoubleclick);
    buttonLeft.setLongClickDetectedHandler(esp_reset);
}

void ButtonHandler::run()
{
    buttonRight.loop();
    buttonLeft.loop();
}

void ButtonHandler::rightsingleclick(Button2 &btn) {}
void ButtonHandler::rightdoubleclick(Button2 &btn) {}
void ButtonHandler::wifi_reset(Button2 &btn) {}
void ButtonHandler::leftsingleclick(Button2 &btn) {}
void ButtonHandler::leftdoubleclick(Button2 &btn) {}
void ButtonHandler::esp_reset(Button2 &btn) {}
