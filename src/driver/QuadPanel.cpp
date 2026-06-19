#include "QuadPanel.h"

// ========================= LGFX_SubPanel =========================

LGFX_SubPanel::LGFX_SubPanel(int cs_pin, bool useRst) {
    {   // ---- SPI Bus 配置 ----
        auto cfg = _bus.config();
        cfg.spi_host    = SPI2_HOST;
        cfg.spi_mode    = 0;
        cfg.freq_write  = LCD_SPI_FREQ;
        cfg.freq_read   = 16000000;
        cfg.spi_3wire   = false;
        cfg.use_lock    = true;             // 多屏共享 bus 必需
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        cfg.pin_sclk    = PIN_SPI_SCLK;
        cfg.pin_mosi    = PIN_SPI_MOSI;
        cfg.pin_miso    = PIN_SPI_MISO;
        cfg.pin_dc      = PIN_LCD_DC;
        _bus.config(cfg);
        _panel.setBus(&_bus);
    }
    {   // ---- Panel 配置 ----
        auto cfg = _panel.config();
        cfg.pin_cs           = cs_pin;
        cfg.pin_rst          = useRst ? PIN_LCD_RST : -1;
        cfg.pin_busy         = -1;
        cfg.memory_width     = SUB_SCREEN_W;
        cfg.memory_height    = 320;              // ST7789 实际显存高 = 320
        cfg.panel_width      = SUB_SCREEN_W;
        cfg.panel_height     = SUB_SCREEN_H;
        cfg.offset_x         = 0;
        cfg.offset_y         = 0;
        cfg.offset_rotation  = 0;
        cfg.dummy_read_pixel = 8;
        cfg.dummy_read_bits  = 1;
        cfg.readable         = false;
        cfg.invert           = true;        // ST7789 需要反色
        cfg.rgb_order        = false;
        cfg.dlen_16bit       = false;
        cfg.bus_shared       = true;        // 关键：声明 bus 共用
        _panel.config(cfg);
    }
    setPanel(&_panel);
}

void LGFX_SubPanel::applyRotation(uint8_t rotation) {
    setRotation(rotation);
}

// =========================== QuadPanel ===========================

void QuadPanel::init() {
    if (_inited) return;

    // ===== 计算最大亮度，设置 PWM =====
    _maxBrightness = 0;
    for (int i = 0; i < PANEL_COUNT; ++i) {
        if (kPanelCfg[i].brightness > _maxBrightness) {
            _maxBrightness = kPanelCfg[i].brightness;
        }
    }
    if (_maxBrightness == 0) _maxBrightness = 100;

    // BL 使用 PWM（LOW 有效 → duty 反转：亮度100% = duty 0%）
    // 初始亮度设为 0（关闭），避免 SPI 初始化期间出现雪花屏
    ledcSetup(BL_PWM_CHANNEL, BL_PWM_FREQ, BL_PWM_RESOLUTION);
    ledcAttachPin(PIN_LCD_BL, BL_PWM_CHANNEL);
    ledcWrite(BL_PWM_CHANNEL, 255); // 255 = 完全关闭背光 (LOW有效)
    Serial.println("[QuadPanel] Backlight OFF during init.");

    // ===== 手动统一复位所有屏幕（共享 RST 线） =====
    pinMode(PIN_LCD_RST, OUTPUT);
    digitalWrite(PIN_LCD_RST, HIGH);
    delay(10);
    digitalWrite(PIN_LCD_RST, LOW);
    delay(20);
    digitalWrite(PIN_LCD_RST, HIGH);
    delay(150);
    Serial.println("[QuadPanel] All panels reset done.");

    // ===== 逐个初始化（pin_rst=-1，不再单独复位） =====
    for (int i = 0; i < PANEL_COUNT; ++i) {
        Serial.printf("[QuadPanel] Init panel %d (CS=IO%d)...\n", i, kPanelCfg[i].cs_pin);
        _panels[i] = new LGFX_SubPanel(kPanelCfg[i].cs_pin, false);
        _panels[i]->init();
        _panels[i]->setRotation(kPanelCfg[i].rotation);
        _panels[i]->setSwapBytes(true);
        _panels[i]->fillScreen(TFT_BLACK);
        Serial.printf("[QuadPanel] Panel %d init OK.\n", i);
    }

    // ===== 方向标识：每屏显示编号 + UP箭头 + 旋转值 =====
    const uint16_t debugColor[PANEL_COUNT] = {
        0xF800, // Red    - 左屏
        0x07E0, // Green  - 中屏
        0x001F  // Blue   - 右屏
    };
    const char *posLabel[PANEL_COUNT] = {
        "L", // Left
        "C", // Center
        "R"  // Right
    };

    for (int i = 0; i < PANEL_COUNT; ++i) {
        auto* p = _panels[i];
        p->fillScreen(debugColor[i]);

        p->setTextColor(TFT_WHITE);
        p->setTextSize(3);

        // 顶部：UP 箭头（标识"上方"方向）
        p->setCursor(90, 10);
        p->print("^ UP ^");

        // 中间：大编号
        p->setTextSize(6);
        p->setCursor(90, 80);
        p->printf("%d", i + 1);

        // 下方：位置 + rotation 信息
        p->setTextSize(2);
        p->setCursor(50, 160);
        p->printf("CS%d %s R%d", i + 1, posLabel[i], kPanelCfg[i].rotation);

        // 底部：亮度
        p->setCursor(50, 190);
        p->printf("Bright: %d", kPanelCfg[i].brightness);

        // 底部箭头标识"下方"
        p->setCursor(90, 220);
        p->print("v DN v");

        Serial.printf("[QuadPanel] Panel %d: %s, R%d, B%d\n",
                      i, posLabel[i], kPanelCfg[i].rotation, kPanelCfg[i].brightness);
    }

    _inited = true;
    Serial.printf("[QuadPanel] All %d panels initialized. MaxBrightness=%d (backlight still OFF)\n",
                  PANEL_COUNT, _maxBrightness);
}

void QuadPanel::turnOnBacklight()
{
    setBrightness(_maxBrightness);
    Serial.println("[QuadPanel] Backlight ON — content ready.");
}

void QuadPanel::setBrightness(uint8_t value) {
    if (value < 1) value = 1;
    if (value > 100) value = 100;
    // BL 是 LOW 有效：value=100 → duty=0, value=1 → duty=252
    uint8_t duty = 255 - (uint16_t)value * 255 / 100;
    ledcWrite(BL_PWM_CHANNEL, duty);
    Serial.printf("[QuadPanel] Brightness set to %d%% (PWM duty=%d/255)\n", value, duty);
}

void QuadPanel::setRotation(uint8_t panelIdx, uint8_t rotation) {
    if (panelIdx >= PANEL_COUNT || _panels[panelIdx] == nullptr) return;
    _panels[panelIdx]->setRotation(rotation);
    Serial.printf("[QuadPanel] Panel %d rotation set to %d\n", panelIdx, rotation);
}

LGFX_SubPanel* QuadPanel::getPanel(uint8_t idx) {
    if (idx >= PANEL_COUNT) return nullptr;
    return _panels[idx];
}

/* static */
void QuadPanel::lvglFlushCb(lv_display_t* disp,
                            const lv_area_t* area,
                            uint8_t* px_map) {
    QuadPanel::getInstance().flushArea(area, px_map);
    lv_display_flush_ready(disp);
}

void QuadPanel::applyBrightnessScale(uint16_t* pixels, uint32_t count, uint8_t brightness) {
    if (brightness >= _maxBrightness) return;  // 不需要衰减

    // 缩放因子：panel_brightness / max_brightness
    // 使用定点数（*256）避免浮点
    const uint32_t scale = (uint32_t)brightness * 256 / _maxBrightness;

    for (uint32_t i = 0; i < count; ++i) {
        uint16_t px = pixels[i];
        // 提取 RGB565 分量
        uint32_t r = (px >> 11) & 0x1F;
        uint32_t g = (px >> 5)  & 0x3F;
        uint32_t b =  px        & 0x1F;
        // 应用缩放
        r = (r * scale) >> 8;
        g = (g * scale) >> 8;
        b = (b * scale) >> 8;
        // 重组
        pixels[i] = (r << 11) | (g << 5) | b;
    }
}

void QuadPanel::flushArea(const lv_area_t* area, uint8_t* px_map) {
    if (!_inited) return;

    const int32_t aw = area->x2 - area->x1 + 1;
    auto* src = reinterpret_cast<uint16_t*>(px_map);

    for (int i = 0; i < PANEL_COUNT; ++i) {
        const auto& cfg = kPanelCfg[i];

        // 本子屏在大屏中的矩形区域
        const int32_t sx1 = cfg.offset_x;
        const int32_t sy1 = cfg.offset_y;
        const int32_t sx2 = sx1 + SUB_SCREEN_W - 1;
        const int32_t sy2 = sy1 + SUB_SCREEN_H - 1;

        // 求 area 与本子屏区域的交集
        const int32_t ix1 = max((int32_t)area->x1, sx1);
        const int32_t iy1 = max((int32_t)area->y1, sy1);
        const int32_t ix2 = min((int32_t)area->x2, sx2);
        const int32_t iy2 = min((int32_t)area->y2, sy2);

        if (ix1 > ix2 || iy1 > iy2) continue;

        const int32_t w = ix2 - ix1 + 1;
        const int32_t h = iy2 - iy1 + 1;

        // 转换到子屏本地坐标
        const int32_t lx = ix1 - sx1;
        const int32_t ly = iy1 - sy1;

        // 像素起始指针
        uint16_t* line0 = src + (iy1 - area->y1) * aw + (ix1 - area->x1);

        auto* p = _panels[i];
        p->startWrite();
        p->setAddrWindow(lx, ly, w, h);

        if (aw == w) {
            p->writePixels(line0, (uint32_t)(w * h));
        } else {
            for (int32_t row = 0; row < h; ++row) {
                p->writePixels(line0 + row * aw, (uint32_t)w);
            }
        }
        p->endWrite();

        // =================================================================
        // 🔧 【可调节 #2】屏间切换延时 — 调频率后还不稳再加大这个
        // =================================================================
        // SPI 从当前屏切换到下一屏时，给 CS 变化/总线脱险预留缓冲。
        // 硬件极限：0μs（完全无延时，CS 切换本身有几个时钟周期开销）
        // 越大越稳，但会拖慢整体刷新。
        //   delayMicroseconds(0)    极限（无延时，依赖 SPI 时钟间隙）
        //   delayMicroseconds(2)    激进
        //   delayMicroseconds(5)    较快 ✅ 当前
        //   delayMicroseconds(20)   推荐
        //   delayMicroseconds(50)   保守
        //   delayMicroseconds(100)  最保守
        //
        // 通过 platformio.ini 的 -DPANEL_SWITCH_DELAY_US=xxx 配置
        // =================================================================
#ifndef PANEL_SWITCH_DELAY_US
#define PANEL_SWITCH_DELAY_US 5
#endif
        delayMicroseconds(PANEL_SWITCH_DELAY_US);
    }
}
