#ifndef DEMO_ANIMATION_H
#define DEMO_ANIMATION_H

#include <lvgl.h>

/**
 * @brief 创建跨三屏的动画演示效果
 *
 * 适配 720×240 三屏横排布局，用于观察屏幕素质和拼接效果：
 * - 中心大圆弧旋转动画（位于三屏中心交汇区域）
 * - 左/中/右各一个 Spinner（验证各屏独立刷新）
 * - 水平渐变条（横跨三屏，检测拼接缝）
 * - 垂直渐变条（纵跨单屏高度）
 * - 中心文字标识坐标系
 */
void createDemoAnimation();

#endif // DEMO_ANIMATION_H
