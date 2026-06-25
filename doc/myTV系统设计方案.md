# myTV 多屏显示系统设计方案

> 本文档为 myTV 多屏显示系统的 **设备端固件** 设计文档。
> Java 后台 + 管理前端设计请参阅 [后台管理系统设计方案.md](./后台管理系统设计方案.md)。

## 设计理念

深色科技风监控面板，信息层级分明，多屏各司其职。后台通过 MQTT 实现远程管理、内容推送、设备集群控制。

系统支持多种设备类型（三屏/四屏/不同尺寸），通过设备类型 + 设备 ID 精确控制每块屏幕的显示内容。

---

## 硬件规格

### 当前设备 (3_screen_tv)

| 项目 | 规格 |
|------|------|
| 主控 | ESP32-S3 N16R8 (双核 Xtensa LX7 @ 240MHz) |
| Flash | 16MB (QIO) |
| PSRAM | 8MB (Octal OPI) |
| 屏幕数量 | 3 块 (横排拼接) |
| 单屏分辨率 | 240×240 px (ST7789) |
| 拼接总分辨率 | 720×240 px |
| 屏幕接口 | SPI 共享总线 + 3 路独立 CS |
| SPI 频率 | 60MHz (可调 4~80MHz) |
| 背光 | PWM 5kHz，8-bit 分辨率 |
| WiFi | 802.11 b/g/n |
| 框架 | Arduino + PlatformIO |
| 图形库 | LovyanGFX + LVGL 9 |

### SPI 引脚分配

| 引脚 | 功能 | 说明 |
|------|------|------|
| GPIO 18 | MOSI (SDA) | SPI 数据线 |
| GPIO 16 | SCLK (SCL) | SPI 时钟线 |
| GPIO 17 | DC | 数据/命令切换 |
| GPIO 15 | RST | 屏幕复位 |
| GPIO 4 | BL | 背光控制 (硬件已上拉) |
| GPIO 38 | CS1 | 左屏片选 |
| GPIO 39 | CS2 | 中屏片选 |
| GPIO 2 | CS3 | 右屏片选 |

### 屏幕拓扑

```
      ┌──────────┬──────────┬──────────┐
      │   CS1    │   CS2    │   CS3    │
      │   左屏   │   中屏   │   右屏   │
      │  (0,0)   │ (240,0)  │ (480,0)  │
      └──────────┴──────────┴──────────┘
       240×240    240×240    240×240
       逻辑坐标: 720×240
```

---

## 设备类型系统

### 支持的设备类型

| device_type | 屏幕数 | 单屏尺寸 | 总分辨率 | 布局 |
|-------------|--------|---------|----------|------|
| `3_screen_tv` | 3 | 240×240 | 720×240 | 横排 1×3 |
| `4_screen_tv` | 4 | 240×240 | 960×240 | 横排 1×4 |
| `4_screen_matrix` | 4 | 240×240 | 480×480 | 矩阵 2×2 |
| `single_screen` | 1 | 240×240 | 240×240 | 单屏 |

### 设备上报硬件信息

设备上线时上报完整硬件配置：

```json
{
  "deviceId": "mytv_0A1B2C3D4E5F",
  "deviceType": "3_screen_tv",
  "hardware": {
    "chipModel": "ESP32-S3",
    "flashSizeMB": 16,
    "psramSizeMB": 8,
    "cpuFreqMHz": 240,
    "screenCount": 3,
    "screenWidth": 240,
    "screenHeight": 240,
    "totalWidth": 720,
    "totalHeight": 240,
    "layout": "horizontal",
    "panels": [
      {"index": 0, "offsetX": 0, "offsetY": 0, "width": 240, "height": 240},
      {"index": 1, "offsetX": 240, "offsetY": 0, "width": 240, "height": 240},
      {"index": 2, "offsetX": 480, "offsetY": 0, "width": 240, "height": 240}
    ]
  },
  "timestamp": 1718892000
}
```

---

## 屏幕布局总览 (3_screen_tv)

```
┌──────────────────┬──────────────────┬──────────────────┐
│      左屏         │      中屏         │      右屏         │
│  时间 + 环境数据  │  主监控 + 运行态  │   设备详细信息    │
│  (0,0)~(239,239) │ (240,0)~(479,239)│(480,0)~(719,239) │
└──────────────────┴──────────────────┴──────────────────┘
```

---

## 配色方案（科技风深色主题）

| 用途 | 颜色值 | 说明 |
|------|--------|------|
| 背景 | `#0a0e1a` | 深蓝黑，近纯黑 |
| 面板卡片 | `#121829` | 略浅的深蓝，用于区分区域 |
| 主文字 | `#e8f0ff` | 白偏冷蓝，高对比 |
| 次要文字 | `#6b7a99` | 蓝灰色，辅助信息 |
| 强调/高亮 | `#00d4ff` | 青蓝色，科技风核心色 |
| 时间数字 | `#00ffaa` | 青绿色，时间专属 |
| 警告 | `#ff6b35` | 橙红，异常/警告 |
| 成功 | `#00e676` | 鲜绿，正常状态 |
| 分隔线/边框 | `#1e2d4a` | 深蓝细线 |
| 进度条背景 | `#1a2540` | 暗蓝灰 |
| 进度条填充 | `#00d4ff` | 青蓝色填充 |

---

## 字体层级

| 层级 | 字号 | 用途 | 颜色 |
|------|------|------|------|
| **超大** | 64pt | 主时钟数字（时:分） | `#00ffaa` 青绿 |
| **大** | 36pt | 日期、主监控数值 | `#e8f0ff` 白蓝 |
| **中** | 22pt | 星期、温湿度、状态值 | `#e8f0ff` 白蓝 |
| **小** | 14pt | 标签、日志、IP、辅助 | `#6b7a99` 蓝灰 |
| **微型** | 10pt | 版本号、MAC地址 | `#4a5670` 暗灰 |

---

## 时间 + 天气获取方案

### 核心思路：后台统一下发

既然有 Java 后台，**时间和天气都由后台服务器统一获取后通过 MQTT 推送给设备**，好处：
- 后台服务器网络稳定，不受设备网络环境影响
- 后台可用 NTP 精确校时，时间精度毫秒级
- 天气 API 查询在后台统一缓存，多设备同城市只查一次，节省配额
- 设备端代码简化，无需处理第三方 API 调用和失败重试

### 后台时间源

后台服务器使用 NTP 校时（标准做法）：
- 服务器系统时间已通过 NTP 同步（Linux systemd-timesyncd / chrony）
- 设备连接 MQTT 时，后台在 `report/online` 响应中下发当前时间戳
- 后台可通过 `cmd/time` 主动广播时间校准

### 后台天气源（国内 API）

| 服务 | 地址 | 说明 |
|------|------|------|
| 和风天气 | `devapi.qweather.com` | 免费额度 1000次/天，推荐 |
| 心知天气 | `api.seniverse.com` | 免费版可用，备用 |

后台根据设备的 `city_id` 查询天气，缓存 30 分钟，通过 MQTT `cmd/weather` 推送。

### MQTT 时间与天气消息格式

```json
// cmd/time - 后台下发时间
{
  "action": "time",
  "timestamp": 1718892000,
  "timezone": 8
}

// cmd/weather - 后台下发天气
{
  "action": "weather",
  "city": "北京",
  "temp": 24.5,
  "humidity": 65,
  "weatherText": "多云",
  "weatherIcon": "cloudy",
  "updatedAt": 1718892000
}
```

---

## MQTT 通信架构（Java 后台 + 设备端）

### 整体架构

```
┌────────────────┐     MQTT      ┌──────────────┐     HTTP/WS    ┌──────────────┐
│  Java 后台      │ ◄──────────► │  EMQX/Mosquitto │ ◄──────────► │  Web 管理面板  │
│  (Spring Boot) │              │  MQTT Broker  │              │  (Vue/React) │
└────────────────┘              └──────┬───────┘              └──────────────┘
                                      │ MQTT
                                      ▼
                               ┌──────────────┐
                               │  ESP32 设备   │
                               │  (myTV)      │
                               └──────────────┘
```

### MQTT Topic 设计

```
# 单设备控制（通过 device_id 精确指定）
mytv/{device_id}/cmd/time            # 校时（后台下发时间戳）
mytv/{device_id}/cmd/weather         # 天气推送（后台查询后下发）
mytv/{device_id}/cmd/reboot          # 远程重启
mytv/{device_id}/cmd/brightness      # 调节亮度
mytv/{device_id}/cmd/volume          # 调节音量
mytv/{device_id}/cmd/display         # 更新显示内容（JSON）
mytv/{device_id}/cmd/ota             # 触发 OTA 升级
mytv/{device_id}/cmd/notify          # 推送通知消息（滚动/弹窗）
mytv/{device_id}/cmd/config          # 远程更新配置
mytv/{device_id}/cmd/audio/record    # 下发录音指令（录制指定时长后上传）
mytv/{device_id}/cmd/audio/stream    # 开启实时音频流上传
mytv/{device_id}/cmd/audio/stop      # 停止录音/流上传

# 按设备类型批量控制（后台通过通配符发布）
mytv/type/{device_type}/cmd/display  # 所有该类型设备执行显示命令
mytv/type/{device_type}/cmd/notify   # 所有该类型设备显示通知
mytv/type/{device_type}/cmd/reboot   # 批量重启某类型设备

# 全局广播（所有设备）
mytv/broadcast/cmd/display           # 全局显示命令
mytv/broadcast/cmd/notify            # 全局通知
mytv/broadcast/cmd/reboot            # 全局重启

# 设备上报
mytv/{device_id}/report/status       # 设备定时上报状态
mytv/{device_id}/report/online       # 设备上线 (LWT: offline)
mytv/{device_id}/report/log          # 故障日志上报
mytv/{device_id}/report/metrics      # 性能指标上报
mytv/{device_id}/report/hardware     # 设备硬件信息上报
mytv/{device_id}/report/audio/done   # 录音完成通知（含元数据，音频文件走 HTTP 上传）
```

### 屏幕定位方式

设备支持三种屏幕定位方式，兼容不同设备类型：

| 定位方式 | 字段 | 说明 | 适用场景 |
|----------|------|------|----------|
| 屏幕索引 | `screenIndex: 0` | 第 N 块屏 (0-based) | 精确控制单屏 |
| 屏幕名称 | `screen: "left"` | 左/中/右 (仅 3屏设备) | 向后兼容 |
| 像素坐标 | `position: {x,y,w,h}` | 大屏逻辑坐标 | 自定义区域显示 |

**屏幕索引与名称映射：**

```
3_screen_tv:  index 0 = left,  index 1 = middle, index 2 = right
4_screen_tv:  index 0 = screen_0, index 1 = screen_1, index 2 = screen_2, index 3 = screen_3
4_screen_matrix: index 0 = top_left, index 1 = top_right, index 2 = bottom_left, index 3 = bottom_right
```

### MQTT 消息格式（JSON）

#### cmd/display - 推送显示内容

**1. 通过屏幕索引控制（推荐，通用性强）：**
```json
{
  "action": "display",
  "screenIndex": 1,
  "content": {
    "type": "text",
    "text": "紧急通知：今天下午3点开会",
    "color": "#ff6b35",
    "fontSize": "large",
    "duration": 30
  }
}
```

**2. 通过像素坐标精确控制显示区域：**
```json
{
  "action": "display",
  "position": {
    "x": 10,
    "y": 50,
    "width": 220,
    "height": 140
  },
  "content": {
    "type": "text",
    "text": "局部消息内容",
    "color": "#00d4ff",
    "fontSize": "medium",
    "duration": 60
  }
}
```

**3. 跨屏显示（横跨多屏的大内容）：**
```json
{
  "action": "display",
  "position": {
    "x": 0,
    "y": 0,
    "width": 720,
    "height": 240
  },
  "content": {
    "type": "image_url",
    "url": "https://your-server.com/images/banner.jpg",
    "md5": "a1b2c3d4...",
    "duration": 60
  }
}
```

**4. 文字显示（通过屏幕名称，向后兼容）：**
```json
{
  "action": "display",
  "screen": "middle",
  "content": {
    "type": "text",
    "text": "紧急通知：今天下午3点开会",
    "color": "#ff6b35",
    "fontSize": "large",
    "duration": 30
  }
}
```

**5. 图片显示（Base64 直传，适合 < 50KB 小图）：**
```json
{
  "action": "display",
  "screenIndex": 0,
  "content": {
    "type": "image_base64",
    "format": "jpg",
    "data": "/9j/4AAQSkZJRgABAQEASABIAA...",
    "duration": 60
  }
}
```

**6. 图片显示（URL + HTTP 下载，推荐）：**
```json
{
  "action": "display",
  "screenIndex": 0,
  "content": {
    "type": "image_url",
    "url": "https://your-server.com/images/notice_001.jpg",
    "md5": "a1b2c3d4e5f6...",
    "duration": 60
  }
}
```

**7. 恢复默认显示：**
```json
{
  "action": "display",
  "screenIndex": 0,
  "content": {
    "type": "default"
  }
}
```

**8. 批量控制某类型设备的特定屏幕：**
```json
// 发布到 mytv/type/3_screen_tv/cmd/display
// 所有 3_screen_tv 设备的左屏(index=0)都会显示
{
  "action": "display",
  "screenIndex": 0,
  "content": {
    "type": "text",
    "text": "全体注意：紧急通知",
    "color": "#ff0000",
    "fontSize": "large",
    "duration": 60
  }
}
```

**图片下发能力说明：**

| 因素 | 限制 | 说明 |
|------|------|------|
| MQTT 消息大小 | 建议 < 256KB | 图片压缩 + 缩放至 240x240 |
| ESP32 内存 | PSRAM 8MB 可用 | 图片解码后存入 PSRAM |
| 图片格式 | JPG 推荐 | LVGL 支持 BMP/PNG/JPG |
| 传输方式 | Base64 直传 vs URL 下载 | 小图走 Base64，大图走 HTTP |
| 本地缓存 | LittleFS | URL 方式下载后可缓存避免重复请求 |

#### 其他命令

```json
// cmd/brightness - 调节亮度
{
  "action": "brightness",
  "value": 80
}

// report/status - 设备上报状态
{
  "deviceId": "mytv_001",
  "timestamp": 1718892000,
  "cpuTemp": 42.5,
  "heapUsed": 45,
  "psramUsed": 32,
  "wifiRssi": -45,
  "uptime": 8700,
  "fwVersion": "v1.2.0"
}
```

#### 录音上传能力

设备搭载 INMP441 MEMS 麦克风 (I2S RX, BCK=21, WS=48, DIN=47) 和 MAX98357A 扬声器，支持录音后上传到后台服务器保存或实时流式传输。

**硬件参数：**

| 参数 | 值 | 说明 |
|------|------|------|
| 采样率 | 16 kHz | 语音识别标准采样率 |
| 位深 | 16-bit | PCM 单声道 |
| 码率 | 32 KB/s | 16000 × 2 bytes |
| 3s 录音 | 96 KB | WAV 含 PCM 数据 |
| 30s 录音 | 960 KB | 存入 PSRAM 后上传 |
| 最大录音 | 60s | ~1.9 MB，PSRAM 足够 |

**两种上传模式：**

| 模式 | 协议 | 场景 | 延迟 |
|------|------|------|------|
| 录制后上传 | HTTP POST multipart | 短录音 (3-30s)，语音指令、留言 | 录完再传 |
| 实时流上传 | WebSocket / HTTP Chunked | 实时监控、远程对讲 | 近实时 |

**模式 1: 录制后上传（MQTT 触发 + HTTP 上传）**

```
后台                          设备 (ESP32)
  │                              │
  ├─ MQTT cmd/audio/record ───▶ │  开始录音
  │   {duration: 5, reason: "voice_cmd"} │
  │                              │  INMP441 I2S 录音 5s
  │                              │  去 DC 偏移 + 编码 WAV
  │                              │
  │ ◀─ HTTP POST /api/audio ─────┤  上传 WAV 文件 (multipart/form-data)
  │                              │  Headers: X-Device-Id, X-Recording-Id, X-Timestamp
  │                              │
  ├─ HTTP 200 OK ───────────────▶ │  上传成功
  │                              │
  │ ◀─ MQTT report/audio/done ─┤  上报元数据
  │   {recordingId, duration,    │
  │    fileSize, fileUrl}         │
```

**模式 2: 实时流上传（WebSocket）**

```
后台                          设备 (ESP32)
  │                              │
  ├─ MQTT cmd/audio/stream ────▶ │  开始实时流
  │   {sampleRate: 16000,        │
  │    wsUrl: "ws://..."}        │
  │                              │
  │ ◀─ WebSocket 连接 ────────────┤  建立连接
  │                              │
  │ ◀─ 二进制帧 (2KB chunk) ──┤  每 128ms 发送一帧
  │ ◀─ 二进制帧 (2KB chunk) ──┤  原始 PCM 16-bit
  │ ◀─ 二进制帧 ...          ──┤  持续流式传输
  │                              │
  ├─ MQTT cmd/audio/stop ────▶ │  停止流传输
  │                              │
  │                              │  关闭 WebSocket
```

**MQTT 消息格式：**

```json
// cmd/audio/record - 后台下发录音指令
{
  "action": "audio_record",
  "recordingId": "rec_20260620_143000",
  "duration": 5,
  "sampleRate": 16000,
  "reason": "voice_command",
  "uploadUrl": "http://api.example.com/api/audio/upload"
}

// cmd/audio/stream - 开启实时流
{
  "action": "audio_stream",
  "wsUrl": "ws://api.example.com/ws/audio/stream",
  "sampleRate": 16000,
  "channels": 1,
  "bitsPerSample": 16
}

// cmd/audio/stop - 停止录音/流
{
  "action": "audio_stop"
}

// report/audio/done - 设备上报录音完成
{
  "deviceId": "mytv_0A1B2C3D4E5F",
  "recordingId": "rec_20260620_143000",
  "timestamp": 1718892000,
  "duration": 5,
  "fileSize": 160044,
  "sampleRate": 16000,
  "format": "wav",
  "fileUrl": "/audio/mytv_0A1B/rec_20260620_143000.wav",
  "reason": "voice_command",
  "peakAmplitude": 18500,
  "rmsAmplitude": 4200
}
```

**HTTP 上传接口：**

```
POST /api/audio/upload
Content-Type: multipart/form-data
X-Device-Id: mytv_0A1B2C3D4E5F
X-Recording-Id: rec_20260620_143000
X-Timestamp: 1718892000
X-Duration: 5

Body: [WAV binary data]

Response 200:
{
  "success": true,
  "fileId": "audio_12345",
  "fileUrl": "/audio/mytv_0A1B/rec_20260620_143000.wav",
  "fileSize": 160044
}
```

**后台 WebSocket 流接收：**

```
// 连接路径: ws://api.example.com/ws/audio/stream?deviceId=xxx
// 每帧: 2KB PCM 16-bit mono 16kHz (64ms 音频)
// 后台处理：
//   1. 实时保存到临时文件
//   2. 可选：转接给 ASR (语音识别) 服务
//   3. 可选：转发给其他监听客户端
//   4. 流结束后合并为完整 WAV 存档
```

**设备端录音服务设计 (`AudioRecorder`)：**

```cpp
class AudioRecorder {
    // 硬件引脚（与 AudioTest.h 一致）
    static constexpr int MIC_BCK  = 21;
    static constexpr int MIC_WS   = 48;
    static constexpr int MIC_DIN  = 47;
    static constexpr uint32_t SAMPLE_RATE = 16000;

    // 录音状态
    enum State { IDLE, RECORDING, UPLOADING, STREAMING };

    // 录制后上传
    bool recordToWav(uint32_t durationSec, const char* recordingId);
    bool uploadWav(const char* recordingId, const char* uploadUrl);

    // 实时流上传
    bool startStreaming(const char* wsUrl);
    void stopStreaming();

    // 录音参数
    uint32_t getPeakAmplitude();
    uint32_t getRmsAmplitude();
};
```

**后台音频存储：**

| 存储类型 | 路径格式 | 保留时长 | 说明 |
|----------|----------|----------|------|
| 原始录音 | `/audio/{device_id}/{recordingId}.wav` | 永久 | 原始 WAV 文件 |
| 流式归档 | `/audio/{device_id}/stream_{timestamp}.wav` | 30天 | 实时流合并归档 |
| ASR 结果 | 数据库记录 | 永久 | 语音识别转文字结果 |

---

## 引导页配置增强

现有 Captive Portal 需新增以下配置项：

| 配置项 | 字段 | 默认值 | 说明 |
|--------|------|--------|------|
| 城市 | `city` | "北京" | 用于天气查询 |
| 城市代码 | `city_id` | "101010100" | 和风天气城市 ID |
| 时区 | `timezone` | 8 (UTC+8) | 已有，保留 |
| MQTT Broker | `mqtt_host` | "" | MQTT 服务器地址 |
| MQTT 端口 | `mqtt_port` | 1883 | MQTT 端口 |
| MQTT 用户 | `mqtt_user` | "" | MQTT 认证 |
| MQTT 密码 | `mqtt_pwd` | "" | MQTT 认证 |



---

## 各屏显示内容

### 左屏 (0,0 ~ 239,239) - 时间 + 天气/环境

**布局：**
```
┌────────────────────────┐
│ ━━━━━━━━━━━━━━━━━━━━━ │  <- 青蓝色装饰线
│  2026-06-20  星期六     │  <- 14pt 蓝灰
│                        │
│      14:30             │  <- 64pt 青绿 时间
│        :58             │  <- 22pt 青绿 秒
│                        │
│ ───────────────────── │  <- 深蓝分隔线
│  🌡 24.5°C   💧 65%    │  <- 22pt 温度+湿度
│  北京  ☁️ 多云       │  <- 14pt 城市+天气
│ ━━━━━━━━━━━━━━━━━━━━━ │  <- 底部装饰线
│  ● 后台已同步  UTC+8   │  <- 14pt 状态栏
└────────────────────────┘
```

| 信息项 | 字号 | 数据来源 | 示例 |
|--------|------|----------|------|
| 年-月-日 | 14pt | MQTT cmd/time | "2026-06-20" |
| 星期 | 14pt | MQTT cmd/time | "星期六" |
| 时:分 | 64pt | MQTT cmd/time | "14:30" |
| 秒 | 22pt | 本地 RTC 推算 | ":58" |
| 温度 | 22pt | MQTT cmd/weather | "24.5°C" |
| 湿度 | 22pt | MQTT cmd/weather | "65%" |
| 城市+天气 | 14pt | MQTT cmd/weather | "北京 ☁ 多云" |
| 同步状态 | 14pt | MQTTService | "● 后台已同步 UTC+8" |

---

### 中屏 (240,0 ~ 479,239) - 主监控 + MQTT 消息 + 运行态

**布局：**
```
┌────────────────────────┐
│ ━━━━━━━━━━━━━━━━━━━━━ │  <- 青蓝色装饰线
│  系统监控面板           │  <- 14pt 标题
│                        │
│  CPU      42.5°C       │  <- 36pt 白蓝
│  ████████░░░  65%      │  <- 进度条
│                        │
│  Heap     45%          │  <- 22pt
│  ██████░░░░  128/285K  │  <- 进度条
│                        │
│  PSRAM    32%          │  <- 22pt
│  ████░░░░░░  2.5/8M   │  <- 进度条
│ ───────────────────── │  <- 深蓝分隔线
│  [MQTT] 已连接         │  <- 14pt 日志
│  [INFO] 系统运行正常   │  <- 14pt 日志
│  [INFO] 已运行 2h35m   │  <- 14pt 日志
│  ★ 远程消息: 下午3点..│  <- 14pt MQTT推送(橙色)
└────────────────────────┘
```

| 信息项 | 字号 | 数据来源 | 示例 |
|--------|------|----------|------|
| CPU 温度 | 36pt | `temperatureRead()` | "42.5°C" |
| Heap 使用 | 22pt | `heap_caps_get_*` | "128KB / 285KB" |
| PSRAM 使用 | 22pt | `heap_caps_get_*` | "2.5MB / 8MB" |
| MQTT 状态 | 14pt | MQTTService | "[MQTT] 已连接" |
| 系统日志 | 14pt | 环形缓冲 | "[INFO] ..." |
| MQTT 推送消息 | 14pt | MQTT cmd/notify | "★ 远程消息: ..." |

---

### 右屏 (480,0 ~ 719,239) - 设备详细信息

**布局：**
```
┌────────────────────────┐
│ ━━━━━━━━━━━━━━━━━━━━━ │  <- 青蓝色装饰线
│  MyTV           v1.2.0 │  <- 22pt 设备名+版本
│ ───────────────────── │  <- 深蓝分隔线
│                        │
│  网络连接              │  <- 14pt 青蓝标题
│  WiFi:   Home-5G      │  <- 14pt
│  信号:   ▮▮▮▮▯ -45dBm │  <- 14pt
│  IP:     192.168.1.x  │  <- 14pt
│  MAC:    AA:BB:CC:..  │  <- 10pt 暗灰
│                        │
│  MQTT 服务             │  <- 14pt 青蓝标题
│  Broker: mqtt.xx.com  │  <- 14pt
│  状态:   ● 已连接     │  <- 14pt 绿色
│  消息数:  128         │  <- 14pt
│                        │
│  存储状态              │  <- 14pt 青蓝标题
│  Flash:  ████░ 1.2/4M │  <- 14pt + 进度条
│  LittleFS: ███░ 120K  │  <- 14pt + 进度条
│                        │
│  芯片: ESP32-S3  PwrOn│  <- 10pt 暗灰
└────────────────────────┘
```

| 信息项 | 字号 | 数据来源 | 示例 |
|--------|------|----------|------|
| 设备名称 | 22pt | `ConfigManager` | "MyTV" |
| 固件版本 | 22pt | 编译宏 `FW_VERSION` | "v1.2.0" |
| WiFi SSID | 14pt | `WiFi.SSID()` | "Home-5G" |
| WiFi 信号 | 14pt | `WiFi.RSSI()` | "▮▮▮▮▯ -45dBm" |
| IP 地址 | 14pt | `WiFi.localIP()` | "192.168.1.100" |
| MQTT Broker | 14pt | `ConfigManager` | "mqtt.xx.com" |
| MQTT 状态 | 14pt | MQTTService | "● 已连接" |
| MQTT 消息数 | 14pt | MQTTService 计数器 | "128" |
| Flash 使用 | 14pt | `ESP.getSketchSize()` | 进度条 |
| LittleFS | 14pt | `LittleFS.usedBytes()` | 进度条 |
| 芯片/复位 | 10pt | `esp_*` | "ESP32-S3 PwrOn" |

---

## 实现任务

### 第一阶段：基础准备

#### Task 1: 注释掉旧代码

- 注释 `NormalMode::begin()` 中的 `createDemoAnimation()` 调用
- 注释 `QuadPanel.cpp` 中屏幕方向校准代码（rotation calibration）
- 确保注释标记清晰，方便后续恢复

#### Task 2: 扩展 ConfigManager 新增配置项

在 `src/service/ConfigManager.h` 中新增：
- `getCity()` / `setCity()` - 城市名称，默认 "北京"
- `getCityId()` / `setCityId()` - 和风天气城市 ID，默认 "101010100"
- `getMqttHost()` / `setMqttHost()` - MQTT Broker 地址
- `getMqttPort()` / `setMqttPort()` - MQTT 端口，默认 1883
- `getMqttUser()` / `setMqttUser()` - MQTT 用户名
- `getMqttPwd()` / `setMqttPwd()` - MQTT 密码
- `saveAll()` 方法增加新参数

#### Task 3: 增强引导配置页面

在 `src/service/CaptivePortalService.h` + `ConfigPortal_html.h` 中：
- 新增城市选择下拉框（主要城市列表 + 城市代码）
- 新增时区选择下拉框（UTC-12 到 UTC+14）
- 新增 MQTT 配置区域（Broker/端口/用户/密码）
- 表单验证

### 第二阶段：时间 + 天气（设备端被动接收）

#### Task 4: 时间管理器 `src/service/TimeManager.h`

- 单例模式
- `setTime(uint32_t unixTimestamp, int8_t timezone)` - 接收后台下发的时间戳
- `getLocalTime()` - 返回 `struct tm`（基于后台时间戳 + millis() 本地推算）
- `isSynced()` - 是否已收到后台时间
- 断网时靠 millis() 继续走时，重新联网后后台自动重发校准

#### Task 5: 天气数据接收（MQTT cmd/weather 处理）

- 在 MQTTService 中处理 `cmd/weather` 消息
- 解析 JSON，更新天气缓存结构体
- InfoPanel UI 读取缓存结构体显示温度/湿度/天气文字

### 第三阶段：三屏 UI

#### Task 6: 信息面板 UI `src/ui/InfoPanel.h` + `src/ui/InfoPanel.cpp`

- `showInfoPanel()` - 创建三屏科技风布局
- 左屏：时间(64pt) + 日期 + 天气 + NTP 状态
- 中屏：CPU/Heap/PSRAM 进度条 + 日志 + MQTT 消息区
- 右屏：设备名 + WiFi + MQTT 状态 + 存储 + 芯片信息
- 配色方案按上方表格实现

#### Task 7: UI 刷新定时器

- 每秒：时间显示（时:分:秒 + 日期，基于 TimeManager 本地推算）
- 每 5 秒：设备状态（温度、内存、Uptime）
- 天气：由后台推送时刷新，设备端不主动轮询

#### Task 8: 集成到 NormalMode

- `NormalMode::begin()` 中启动信息面板 + TimeService + WeatherService + MQTTService
- `NormalMode::loop()` 中运行 LVGL + MQTT loop

### 第四阶段：MQTT 通信 + 录音

#### Task 9: MQTT 服务 `src/service/MQTTService.h`

- 单例模式
- `begin()` - 连接 MQTT Broker
- `loop()` - MQTT 消息循环
- `publish()` - 发布消息
- `subscribe()` - 订阅命令 Topic
- 命令处理：
  - `cmd/time` → TimeManager::setTime() 校准时间
  - `cmd/weather` → 更新天气缓存
  - `cmd/reboot` → `ESP.restart()`
  - `cmd/brightness` → 调节亮度
  - `cmd/display` → 更新指定屏幕显示内容
  - `cmd/notify` → 显示滚动/弹窗通知
  - `cmd/ota` → 触发 OTA 升级
  - `cmd/config` → 远程更新配置
  - `cmd/audio/record` → AudioRecorder::recordToWav() 开始录音
  - `cmd/audio/stream` → AudioRecorder::startStreaming() 开启实时流
  - `cmd/audio/stop` → AudioRecorder::stopStreaming() 停止录音/流
- LWT (Last Will) 配置：离线自动报告
- 定时上报状态（每 60 秒）

#### Task 10: MQTT 消息显示组件

- 中屏底部滚动消息区域
- 支持优先级：INFO（白）/ WARN（橙）/ URGENT（红）
- 消息队列，最多显示最近 5 条
- 消息自动过期（可配置 duration）

#### Task 11: 录音服务 `src/service/AudioRecorder.h`

基于现有 AudioTest.h 的录音能力，封装为服务供 MQTT 远程调用：

- 单例模式
- `recordToWav(durationSec, recordingId, uploadUrl)` - 录音 + HTTP 上传
  - 复用 INMP441 I2S 录音逻辑 (与 AudioTest.h 相同引脚/采样率)
  - 录音存入 PSRAM (最大 60s ~1.9MB)
  - 去 DC 偏移 + 编码 WAV
  - HTTP POST multipart/form-data 上传
  - 上传完成后 MQTT report/audio/done 上报元数据
- `startStreaming(wsUrl)` - WebSocket 实时流
  - I2S 录音 + 每 128ms 发送一帧 (2KB PCM)
  - 支持后台远程停止 (cmd/audio/stop)
- `stopStreaming()` - 停止流传输
- `getState()` - 返回当前状态 (IDLE/RECORDING/UPLOADING/STREAMING)
- 与 BootButton 互斥 (录音期间屏蔽 GPIO0 误触发)

> **后台架构设计（Java 后台 + 管理页面 + 数据库）已独立为 [后台管理系统设计方案.md](./后台管理系统设计方案.md)**

---

## 设备识别方案

### 设备 ID 生成规则

每台设备使用唯一 ID 标识，格式：`mytv_{mac_short}_{chip_id}`

```cpp
// 设备 ID 生成示例
String getDeviceId() {
    uint64_t chipId = ESP.getEfuseMac();
    char id[32];
    snprintf(id, sizeof(id), "mytv_%04X%08X",
             (uint16_t)(chipId >> 32), (uint32_t)chipId);
    return String(id);  // 例: "mytv_0A1B2C3D4E5F"
}
```

- **设备 ID**：基于 ESP32 eFuse MAC 地址生成，不可篡改
- **设备名称**：用户可自定义（ConfigManager::getDeviceName()），默认 "MyTV"
- **设备分组**：后台侧管理，用于批量控制（如 "办公室"、"展厅"）

### MQTT Topic 中的设备 ID 使用

```
mytv/{device_id}/cmd/*       # 后台 → 设备（下行命令）
mytv/{device_id}/report/*    # 设备 → 后台（上行数据）
```

### 设备首次上线流程

```
设备启动 → 连接 WiFi → 连接 MQTT
    ↓
发布 report/online 消息（包含设备完整信息）
    ↓
后台收到 → 检查设备是否已注册
    ↓
├─ 已注册 → 更新设备状态为在线，同步配置下发
│
└─ 未注册 → 自动注册设备（或等待管理员审批）
           └─ 下发默认配置
```

---

## 设备上报机制

### 上线上报（report/online）

设备每次联网后主动上报完整信息（包含设备类型和硬件配置）：

```json
{
  "deviceId": "mytv_0A1B2C3D4E5F",
  "deviceType": "3_screen_tv",
  "deviceName": "客厅电视",
  "fwVersion": "v1.2.0",
  "chipModel": "ESP32-S3",
  "macAddress": "AA:BB:CC:DD:EE:FF",
  "ipAddress": "192.168.1.100",
  "wifiSsid": "Home-5G",
  "wifiPassword": "***",
  "city": "北京",
  "cityId": "101010100",
  "timezone": 8,
  "mqttHost": "mqtt.example.com",
  "brightness": 80,
  "otaEnabled": true,
  "hardware": {
    "flashSizeMB": 16,
    "psramSizeMB": 8,
    "cpuFreqMHz": 240,
    "screenCount": 3,
    "screenWidth": 240,
    "screenHeight": 240,
    "totalWidth": 720,
    "totalHeight": 240,
    "layout": "horizontal",
    "panels": [
      {"index": 0, "offsetX": 0, "offsetY": 0, "width": 240, "height": 240},
      {"index": 1, "offsetX": 240, "offsetY": 0, "width": 240, "height": 240},
      {"index": 2, "offsetX": 480, "offsetY": 0, "width": 240, "height": 240}
    ],
    "audio": {
      "hasMic": true,
      "micModel": "INMP441",
      "micBckPin": 21,
      "micWsPin": 48,
      "micDinPin": 47,
      "hasSpeaker": true,
      "speakerModel": "MAX98357A",
      "sampleRate": 16000,
      "maxRecordDurationSec": 60
    }
  },
  "timestamp": 1718892000
}
```

### 状态上报（report/status）- 每 60 秒

```json
{
  "deviceId": "mytv_0A1B2C3D4E5F",
  "timestamp": 1718892060,
  "cpuTemp": 42.5,
  "heapUsedPct": 45,
  "heapUsedKB": 128,
  "heapTotalKB": 285,
  "psramUsedPct": 32,
  "psramUsedMB": 2.5,
  "psramTotalMB": 8,
  "wifiRssi": -45,
  "uptime": 8700,
  "mqttMsgCount": 128
}
```

### 日志上报（report/log）

```json
{
  "deviceId": "mytv_0A1B2C3D4E5F",
  "timestamp": 1718892060,
  "level": "INFO",
  "message": "OTA update completed successfully",
  "source": "OTAService"
}
```

### 配置同步上报（report/config）

当用户通过引导页修改配置时，设备上报完整配置到后台：

```json
{
  "deviceId": "mytv_0A1B2C3D4E5F",
  "timestamp": 1718892060,
  "config": {
    "wifiSsid": "Home-5G",
    "wifiPassword": "***",
    "deviceName": "客厅电视",
    "brightness": 80,
    "timezone": 8,
    "city": "北京",
    "cityId": "101010100",
    "mqttHost": "mqtt.example.com",
    "mqttPort": 1883,
    "otaEnabled": true
  }
}
```

---

## 装饰元素

| 元素 | 说明 |
|------|------|
| 顶部/底部装饰线 | 青蓝色 (`#00d4ff`) 2px 水平线，贯穿三屏 |
| 区域分隔线 | 深蓝 (`#1e2d4a`) 1px 细线 |
| 状态圆点 | 绿色=正常，橙色=警告，红色=异常 |
| 进度条 | 青蓝填充 (`#00d4ff`)，暗灰背景 (`#1a2540`) |
| 信号格 | ▮ 填充 / ▯ 空，青蓝色 |
| MQTT 消息 | 橙色 (`#ff6b35`) 前缀★，滚动显示 |
