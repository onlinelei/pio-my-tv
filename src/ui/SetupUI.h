#ifndef SETUP_UI_H
#define SETUP_UI_H

#include <Arduino.h>
#include <lvgl.h>
#include <IPAddress.h>

/**
 * @brief 设备引导配置 LVGL 界面
 *
 * 在 Captive Portal 模式下，720x240 三屏显示：
 * - WiFi 热点名称
 * - 配置地址 (192.168.4.1)
 * - 连接引导提示
 */

/**
 * @brief 显示引导配网界面
 * @param apName WiFi 热点名称
 * @param ip AP IP 地址
 */
void showSetupScreen(const char *apName, IPAddress ip);

/**
 * @brief 显示"配置已保存，正在重启..."提示
 */
void showSetupSavedScreen();

/**
 * @brief 显示恢复出厂设置提示
 */
void showFactoryResetScreen();

#endif // SETUP_UI_H
