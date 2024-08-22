#include <Arduino.h>

#include "ThreadController.h"
#include "ButtonHandler.h"
#include "Animation.h"
#include "WiFiManager.h"

ThreadController::ThreadController()
    : initThread(ThreadController::staticInitThreadFunc, 100),
      reflashTime(ThreadController::staticReflashTimeFunc, 20),
      reflashBanner(ThreadController::staticReflashBannerFunc, 2000),
      controller(&initThread, &reflashTime, &reflashBanner)
{
}

ThreadController::~ThreadController()
{
}

void ThreadController::init()
{
    // 初始化代码已经在构造函数中完成
}

void ThreadController::run()
{
    controller.run();
}

void ThreadController::staticInitThreadFunc()
{
    // ButtonHandler::getInstance().init(3, 4);
}

void ThreadController::staticReflashTimeFunc()
{
    // WiFiManager::getInstance().connect();
    Animation::getInstance().runStarFieldAuto();
}

void ThreadController::staticReflashBannerFunc()
{
}
