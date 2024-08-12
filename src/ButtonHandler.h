// ButtonHandler.h
#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include <Button2.h>

class ButtonHandler {
public:
    ButtonHandler(int pinRight, int pinLeft);
    void init();
    void update();

private:
    Button2 buttonRight;
    Button2 buttonLeft;

    static void rightsingleclick(Button2 &btn);
    static void rightdoubleclick(Button2 &btn);
    static void wifi_reset(Button2 &btn);
    static void leftsingleclick(Button2 &btn);
    static void leftdoubleclick(Button2 &btn);
    static void esp_reset(Button2 &btn);
};

#endif // BUTTONHANDLER_H
