#include "ThreadController.h"

ThreadController::ThreadController(PinController& pinController, DisplayController& display, Animation& animation, ButtonHandler& buttons)
    : pinController(pinController),
      display(display),
      animation(animation),
      buttons(buttons),
      controller(&initThread, &reflashTime, &reflashBanner) {
}

void ThreadController::init() {
    // initThread.setInterval(100); // 设置初始化任务的间隔时间
    // initThread.onRun([this]() { initThreadFunc(); }); // 使用 lambda 表达式

    // reflashTime.setInterval(300); // 设置所需间隔 300毫秒
    // reflashTime.onRun([this]() { reflashTimeFunc(); }); // 使用 lambda 表达式

    // reflashBanner.setInterval(2 * 1000); // 设置所需间隔 2秒
    // reflashBanner.onRun([this]() { reflashBannerFunc(); }); // 使用 lambda 表达式
}

void ThreadController::run() {
    controller.run();
}

void ThreadController::initThreadFunc() {
    // 初始化线程的实现
    animation.lodingPage();
}

void ThreadController::reflashTimeFunc() {
    // reflashTime 线程的实现
}

void ThreadController::reflashBannerFunc() {
    // reflashBanner 线程的实现
}
