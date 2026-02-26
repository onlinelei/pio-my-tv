#ifndef THREAD_CONTROLLER_H
#define THREAD_CONTROLLER_H

#include <Thread.h>
#include <StaticThreadController.h>

class ThreadController {
public:
    static ThreadController &getInstance()
    {
        static ThreadController instance;
        return instance;
    }
    void init();
    void run();

private:
    ThreadController();
    ~ThreadController();
    ThreadController(const ThreadController &) = delete;
    ThreadController &operator=(const ThreadController &) = delete;

    Thread initThread = Thread(staticInitThreadFunc);
    Thread reflashTime = Thread(staticReflashTimeFunc);
    Thread reflashBanner = Thread(staticReflashBannerFunc);

    StaticThreadController<3> controller;

    static void staticInitThreadFunc();
    static void staticReflashTimeFunc();
    static void staticReflashBannerFunc();
};

#endif // THREAD_CONTROLLER_H
