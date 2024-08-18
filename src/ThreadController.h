#ifndef THREAD_CONTROLLER_H
#define THREAD_CONTROLLER_H

#include <Thread.h>
#include <StaticThreadController.h>
#include "DisplayController.h"
#include "ButtonHandler.h"
#include "Animation.h"
#include "PinController.h"

class ThreadController {
public:
    ThreadController(PinController& pinController, DisplayController& display, Animation& animation, ButtonHandler& buttons);
    void init();
    void run();

private:
    PinController& pinController;
    DisplayController& display;
    Animation& animation;
    ButtonHandler& buttons;

    Thread initThread;
    Thread reflashTime;
    Thread reflashBanner;

    StaticThreadController<3> controller;

    void initThreadFunc();
    void reflashTimeFunc();
    void reflashBannerFunc();
};

#endif // THREAD_CONTROLLER_H
