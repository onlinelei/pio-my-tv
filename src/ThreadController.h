#ifndef THREAD_CONTROLLER_H
#define THREAD_CONTROLLER_H

#include <Thread.h>
#include <StaticThreadController.h>

class ThreadController {
public:
    ThreadController();
    void init();
    void run();

private:
    Thread reflash_time;
    Thread reflash_Banner;
    StaticThreadController<2> controller;

    static void reflashTime();
    static void reflashBanner();
};

#endif // THREAD_CONTROLLER_H