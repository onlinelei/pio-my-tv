# myTV - ESP32-S3 四屏拼接显示系统

基于 **ESP32-S3 N16R8** + **LovyanGFX** + **LVGL 9.x** 的 480×480 四屏拼接显示方案。

通过 4 块 240×240 的 ST7789 SPI 屏幕共享同一 SPI 总线，配合独立 CS 引脚和 LVGL 的 partial render 模式，实现一块完整的 480×480 大屏。

---

## 一、硬件方案

### 1.1 主控

| 项 | 参数 |
|---|---|
| 主控 | ESP32-S3-WROOM-1 N16R8 |
| Flash | 16MB QIO |
| PSRAM | 8MB Octal (OPI) |
| CPU | 240MHz Dual-core Xtensa LX7 |
| 烧录 | 内置 USB-CDC（无需外挂串口芯片） |

### 1.2 屏幕

4 块独立 ST7789 240×240 SPI 屏幕，按 2×2 排列拼接成 480×480 大屏。

```
逻辑大屏布局（左上角为原点 (0,0)）：

      ┌──────────┬──────────┐
      │  CS1     │   CS2    │
      │ 第二象限 │ 第一象限 │
      │ (左上)   │  (右上)  │
      ├──────────┼──────────┤
      │  CS3     │   CS4    │
      │ 第三象限 │ 第四象限 │
      │ (左下)   │  (右下)  │
      └──────────┴──────────┘
              480 × 480
```

### 1.3 引脚连接

| 信号 | GPIO | 说明 |
|---|---|---|
| SPI MOSI (SDA) | IO18 | 4 屏共享 |
| SPI SCLK | IO16 | 4 屏共享 |
| LCD DC | IO17 | 4 屏共享 |
| LCD RST | IO15 | 4 屏共享（启动时统一复位） |
| LCD BL | IO4 | PWM 背光，4 屏共享，**LOW 有效** |
| CS1 | IO38 | 左上屏幕 |
| CS2 | IO39 | 右上屏幕 |
| CS3 | IO2  | 左下屏幕（注意：S3 Strapping Pin） |
| CS4 | IO1  | 右下屏幕 |

> ⚠️ IO2 是 ESP32-S3 的 Strapping Pin，启动时影响 boot 模式，焊接后请确保上电时该引脚不被外部强拉。

---

## 二、软件架构

### 2.1 技术栈

| 层 | 组件 | 版本 |
|---|---|---|
| 框架 | Arduino-ESP32 | platform-espressif32 ^6.6.0 |
| 显示驱动 | LovyanGFX | ^1.1.16 |
| GUI 库 | LVGL | ^9.0.0 |
| 构建系统 | PlatformIO | latest |

### 2.2 项目结构

```
src/
├── mian.cpp                # 入口：setup() / loop() + 动画演示
├── config/
│   ├── display_config.h    # ★ 屏幕引脚 / 旋转 / 亮度配置
│   └── lv_conf.h           # LVGL 配置
├── driver/
│   ├── QuadPanel.h/cpp     # ★ 四屏统一驱动 + LVGL flush 回调
│   └── WiFi_Singleton.h    # WiFi 初始化封装
└── service/
    ├── LVGLService.h       # LVGL 初始化（PSRAM 双缓冲 + partial render）
    └── MemoryMonitor.h     # 内存监控工具
```

### 2.3 核心机制

1. **共享 SPI 总线**：4 块屏幕共用 `SPI2_HOST`，通过 LovyanGFX 的 `bus_shared = true` + `use_lock = true` 实现总线仲裁。
2. **统一 RST 复位**：启动时手动统一拉低 RST 一次，避免每个 panel `init()` 时互相复位。各 panel 配置中 `pin_rst = -1`。
3. **LVGL Partial Render**：480×480 全屏帧缓存太大（≈450KB×2），改用 partial 模式，每次只渲染一条 480×60 的缓冲（PSRAM 中分配 ≈56KB×2 双缓冲）。
4. **脏区域分发**：`flushArea()` 计算 LVGL 脏区域与每块屏的交集，仅推送相关像素到对应屏幕。
5. **PWM 背光 + 软件亮度补偿**：4 屏共享 BL，物理 PWM 取最大亮度；亮度差异通过软件 RGB565 缩放补偿。

---

## 三、配置说明

所有屏幕可调参数集中在 [src/config/display_config.h](src/config/display_config.h)：

```cpp
static const PanelCfg kPanelCfg[PANEL_COUNT] = {
    // CS1 → 左上, UP朝右→左旋90°
    { PIN_LCD_CS1, /*rotation=*/3, /*offset_x=*/0,            /*offset_y=*/0,            /*brightness=*/100 },
    // CS2 → 右上, UP朝左→右旋90°
    { PIN_LCD_CS2, /*rotation=*/1, /*offset_x=*/SUB_SCREEN_W, /*offset_y=*/0,            /*brightness=*/100 },
    // CS3 → 左下
    { PIN_LCD_CS3, /*rotation=*/3, /*offset_x=*/0,            /*offset_y=*/SUB_SCREEN_H, /*brightness=*/100 },
    // CS4 → 右下
    { PIN_LCD_CS4, /*rotation=*/1, /*offset_x=*/SUB_SCREEN_W, /*offset_y=*/SUB_SCREEN_H, /*brightness=*/100 },
};
```

| 字段 | 含义 | 取值 |
|---|---|---|
| `cs_pin` | CS 引脚 GPIO | 见硬件表 |
| `rotation` | 屏幕旋转 | 0/1/2/3 (0°/90°CW/180°/270°CW) |
| `offset_x/y` | 在大屏中的位置 | 0 或 SUB_SCREEN_W/H |
| `brightness` | 亮度修正 | 1~100，默认 100 |

**SPI 频率**（同文件）：
```cpp
#define LCD_SPI_FREQ    20000000UL   // 当前 20MHz，稳定后可提升到 40MHz
```

---

## 四、构建与烧录

### 4.1 环境

```bash
# 安装 PlatformIO Core
pip install platformio
```

### 4.2 编译 / 烧录 / 监控

```bash
# 编译
pio run -e esp32-s3-n16r8

# 烧录（USB CDC，自动识别 /dev/cu.usbmodem*）
pio run -e esp32-s3-n16r8 -t upload

# 串口监控
pio device monitor
```

### 4.3 烧录失败

如遇 USB 端口连接失败，按住 **BOOT** 键再按 **RST** 键，让设备进入下载模式后重试。

---

## 五、当前功能

启动后流程：

1. **方向标识阶段**（前 3 秒）：每屏显示 `^UP^` / 大数字编号 / `vDNv`，用于校对 `rotation` 配置。
2. **动画演示**：
   - 中心 300px 旋转圆弧（跨越 4 屏交界）
   - 四角小 Spinner（独立刷新验证）
   - 水平 / 垂直进度条（拼接缝检测）
   - 中心文字 `480 x 480 Quad Screen`
3. WiFi 自动连接（SSID/密码在 platformio.ini）。
4. 串口输出内存使用 / WiFi 信息。

---

## 六、后续优化与开发方向

### 6.1 性能优化

- [ ] **DMA 双缓冲并行刷新**：当前 flush 是串行处理 4 屏，可探索 LovyanGFX 的 DMA 异步推送，让 LVGL 渲染下一帧时同时推送当前帧。
- [ ] **SPI 频率分级测试**：从 20MHz 提升到 27/33/40MHz，配合走线长度评估稳定性。
- [ ] **缩小 LVGL 缓冲**：当前 480×60 ≈ 56KB；如果 PSRAM 紧张可改为 480×40 进一步压缩。
- [ ] **CPU Core 拆分**：把 LVGL 渲染放到 Core 1，flush（SPI 推送）放到 Core 0，并发提升帧率。
- [ ] **跳过纯黑/不变区域**：在 flushArea 内对纯色区域做命中优化，减少 SPI 数据量。

### 6.2 显示功能

- [ ] **去掉调试动画，做正式 UI**：表盘、状态卡片、媒体控制等。
- [ ] **图片/动画资源**：接入 LittleFS 加载 PNG/GIF（LVGL 9.x 已支持）。
- [ ] **中文字体**：当前仅 montserrat 拉丁字符，需要接入中文字库（lv_font_conv 生成 GB2312 子集）。
- [ ] **触摸输入**：增加触摸屏（如 GT911 / FT6236），LVGL 接入 indev 输入设备。

### 6.3 系统功能

- [ ] **NTP 时间同步 + 时钟应用**。
- [ ] **MQTT 接入**：作为家居仪表盘显示传感器数据。
- [ ] **HTTP API**：远程推送图片 / 文字到屏幕。
- [ ] **OTA 升级**：充分利用 16MB Flash 做 A/B 双分区。
- [ ] **配置持久化**：把 `rotation` / `brightness` 改成可运行时调整 + 写 NVS。

### 6.4 工程化

- [ ] **WiFi 配置外置**：改用 WiFiManager 实现配网，避免硬编码到 platformio.ini。
- [ ] **单元测试**：恢复 `[env:native]` 测试环境，对 `flushArea` 的脏区域计算等纯逻辑做覆盖。
- [ ] **CI 构建**：GitHub Actions 跑编译验证。
- [ ] **日志分级**：用 esp_log 替代 Serial.println，按模块分级输出。

### 6.5 已知问题

- 屏间存在物理拼接缝（约 1~2mm），当前无法消除（硬件限制）。
- 4 屏亮度差异通过软件补偿（颜色缩放），高亮度场景近似线性，暗部可能有色阶损失。
- IO2 (CS3) 启动时受外部强拉可能导致 boot 失败，注意硬件设计。

---

## 七、分支说明

- `master`：原始单屏 ESP32 项目
- `feature/ray/4_screen`：当前四屏拼接开发分支
