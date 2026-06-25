#ifndef BOOT_BUTTON_H
#define BOOT_BUTTON_H

#include <Arduino.h>
#include "service/ConfigManager.h"
#include "driver/QuadPanel.h"
#include "ui/SetupUI.h"

/**
 * @brief BOOT 按键长按检测服务（单例）
 *
 * ESP32-S3 的 GPIO 0 在开机时按住会进入 USB 下载模式，sketch 不运行，
 * 因此无法在 setup() 中检测。改为运行时在 loop() 中非阻塞检测：
 *   - 按住 BOOT 键超过 _holdMs → 触发恢复出厂设置
 *
 * GPIO0 刷新机制：
 *   I2S+APLL 操作后 GPIO0 可能被 GPIO 矩阵锁低，
 *   通过短暂切换 OUTPUT HIGH → INPUT_PULLUP 清除残留状态。
 */
class BootButton
{
public:
    static BootButton &getInstance()
    {
        static BootButton instance;
        return instance;
    }

    void begin()
    {
        pinMode(_pin, INPUT_PULLUP);
        Serial.printf("[BootButton] Initialized on GPIO%d, hold %ds to factory reset\n",
                      _pin, _holdMs / 1000);
    }

    /**
     * @brief 暂时屏蔽按键检测（音频测试等 I2S 操作时调用）
     */
    void suppressFor(unsigned long ms)
    {
        _suppressUntilMs = millis() + ms;
        _debounceCount = 0;
        _pressed = false;
        _pressStartMs = 0;
    }

    /**
     * @brief 在 loop() 中轮询调用，检测长按并触发恢复出厂
     */
    void loop()
    {
        // 屏蔽期内跳过（音频测试期间 GPIO0 不可靠）
        if (millis() < _suppressUntilMs) {
            return;
        }

        // ─── GPIO0 刷新：清除 I2S 操作后的引脚残留状态 ───
        // 将 GPIO0 短暂设为 OUTPUT HIGH，再切回 INPUT_PULLUP。
        // 这能清除 GPIO 矩阵中 I2S/APLL 可能残留的配置，
        // 而真正按下的 BOOT 键（物理接地）仍会读 LOW。
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, HIGH);
        delayMicroseconds(5);
        pinMode(_pin, INPUT_PULLUP);
        delayMicroseconds(10); // 等待上拉稳定

        bool rawLow = (digitalRead(_pin) == LOW);

        // 去抖：连续多次读到低电平才确认按下
        if (rawLow) {
            if (_debounceCount < 255) _debounceCount++;
        } else {
            _debounceCount = 0;
        }

        bool pressed = (_debounceCount >= _DEBOUNCE_COUNT);

        if (pressed)
        {
            if (!_pressed)
            {
                _pressed = true;
                _pressStartMs = millis();
                Serial.printf("[BootButton] Pressed (debounced), timing... (GPIO0=LOW, count=%d)\n",
                              _debounceCount);
            }
            else if (millis() - _pressStartMs > _holdMs)
            {
                _onLongPress();
            }
        }
        else
        {
            if (_pressed)
            {
                unsigned long held = millis() - _pressStartMs;
                Serial.printf("[BootButton] Released (held %lums, threshold %lums)\n",
                              held, _holdMs);
                _pressed = false;
                _pressStartMs = 0;
            }
        }
    }

    /**
     * @brief 串口命令触发的恢复出厂设置（不需要按 BOOT 键）
     */
    static void serialFactoryReset()
    {
        Serial.println("[BootButton] === Serial factory reset requested ===");
        Serial.println("[BootButton] Erasing NVS in 2s (send 'x' to cancel)...");
        delay(2000);
        // 检查是否取消
        while (Serial.available()) {
            if (Serial.read() == 'x') {
                Serial.println("[BootButton] Cancelled.");
                return;
            }
        }
        showFactoryResetScreen();
        QuadPanel::getInstance().turnOnBacklight();
        delay(1000);
        ConfigManager::getInstance().factoryReset();
    }

private:
    BootButton() {}
    BootButton(const BootButton &) = delete;
    BootButton &operator=(const BootButton &) = delete;

    static constexpr uint8_t _pin = 0;
    static constexpr unsigned long _holdMs = 10000;     // 10s 长按阈值
    static constexpr uint8_t _DEBOUNCE_COUNT = 50;      // ~50 次连续 LOW（~800ms）

    bool _pressed = false;
    unsigned long _pressStartMs = 0;
    uint8_t _debounceCount = 0;
    unsigned long _suppressUntilMs = 0;

    void _onLongPress()
    {
        // 触发前再次确认 GPIO0
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, HIGH);
        delayMicroseconds(5);
        pinMode(_pin, INPUT_PULLUP);
        delayMicroseconds(10);

        if (digitalRead(_pin) != LOW) {
            Serial.println("[BootButton] GPIO0 bounced HIGH at trigger — cancelled.");
            _pressed = false;
            _pressStartMs = 0;
            _debounceCount = 0;
            return;
        }

        Serial.printf("[BootButton] Factory reset triggered (held %lums)!\n",
                      millis() - _pressStartMs);
        showFactoryResetScreen();
        QuadPanel::getInstance().turnOnBacklight();
        delay(2000);
        ConfigManager::getInstance().factoryReset();
    }
};

#endif // BOOT_BUTTON_H
