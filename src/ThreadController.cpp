#include "ThreadController.h"

ThreadController::ThreadController()
    : controller(&reflash_time, &reflash_Banner) {
}

void ThreadController::init() {
    reflash_time.setInterval(300); // 设置所需间隔 300毫秒
    reflash_time.onRun(reflashTime);

    reflash_Banner.setInterval(2 * 1000); // 设置所需间隔 2秒
    reflash_Banner.onRun(reflashBanner);
}

void ThreadController::run() {
    controller.run();
}

void ThreadController::reflashTime() {
    // TODO 更新时间
}

void ThreadController::reflashBanner() {
    // TODO 切换天气 or 空气质量
}