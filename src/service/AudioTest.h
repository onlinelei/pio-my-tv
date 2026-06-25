#ifndef AUDIO_TEST_H
#define AUDIO_TEST_H

#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>
#include "service/BootButton.h"

/**
 * @brief 音频硬件验证（INMP441 + MAX98357A）
 *
 * 串口命令：
 *   'a' = 录音 3s + 回放（完整测试）
 *   's' = 仅播放 440Hz 正弦波 3s（验证扬声器）
 *   'm' = 播放多音符旋律（验证扬声器音质）
 *   'w' = 录音并通过串口输出 WAV 格式（PC 端分析）
 *   'p' = GPIO 探测（万用表检测）
 *   'f' = 恢复出厂设置（不需要按 BOOT 键）
 */
class AudioTest
{
public:
    static constexpr int MIC_BCK  = 21;
    static constexpr int MIC_WS   = 48;
    static constexpr int MIC_DIN  = 47;

    static constexpr int SPK_BCLK = 13;
    static constexpr int SPK_LRCK = 14;
    static constexpr int SPK_DIN  = 12;

    static constexpr int LED_PIN  = 5;

    static constexpr uint32_t SAMPLE_RATE = 16000;
    static constexpr uint32_t DURATION_S  = 3;
    static constexpr uint32_t TOTAL_SAMPLES = SAMPLE_RATE * DURATION_S;

    static constexpr uint32_t CHUNK = 1024;

    // ============================================================
    //  扬声器单独测试：播放 440Hz 正弦波
    // ============================================================
    static void testSpeaker()
    {
        Serial.println("[AudioTest] ===== Speaker Test (440Hz tone, 3s) =====");
        pinMode(LED_PIN, OUTPUT);

        // 先确保 I2S 干净状态
        i2s_driver_uninstall(I2S_NUM_1);

        const i2s_config_t spk_cfg = {
            .mode        = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format  = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,
            .dma_buf_len   = 256,
            .use_apll = true,      // APLL 时钟更精确，减少抖动
            .tx_desc_auto_clear = true,
        };
        const i2s_pin_config_t spk_pins = {
            .bck_io_num   = SPK_BCLK,
            .ws_io_num    = SPK_LRCK,
            .data_out_num = SPK_DIN,
            .data_in_num  = I2S_PIN_NO_CHANGE,
        };
        ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_1, &spk_cfg, 0, nullptr));
        ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_1, &spk_pins));
        i2s_zero_dma_buffer(I2S_NUM_1);

        Serial.printf("[AudioTest] I2S_NUM_1 init done (BCLK=IO%d, LRCLK=IO%d, DIN=IO%d, APLL=ON)\n",
                      SPK_BCLK, SPK_LRCK, SPK_DIN);

        // 先送 500ms 静音，让 DMA 稳定
        {
            int16_t *silence = (int16_t *)calloc(SAMPLE_RATE / 2 * 2, sizeof(int16_t));
            size_t w = 0;
            i2s_write(I2S_NUM_1, silence, SAMPLE_RATE / 2 * 2 * sizeof(int16_t),
                      &w, pdMS_TO_TICKS(600));
            free(silence);
            Serial.println("[AudioTest] DMA warmed up (500ms silence)");
        }

        // 生成 440Hz 正弦波，高振幅
        const float freq = 440.0f;
        const float amplitude = 28000.0f;   // ~85% 满幅
        const uint32_t PLAY_CHUNK = SAMPLE_RATE / 10;   // 100ms
        int16_t *stereo = (int16_t *)malloc(PLAY_CHUNK * 2 * sizeof(int16_t));

        Serial.printf("[AudioTest] Playing %uHz sine, amplitude=%.0f, %us...\n",
                      (unsigned)freq, amplitude, DURATION_S);

        uint32_t frames_played = 0;
        bool led = false;

        while (frames_played < TOTAL_SAMPLES) {
            uint32_t cnt = min(PLAY_CHUNK, TOTAL_SAMPLES - frames_played);
            for (uint32_t i = 0; i < cnt; i++) {
                float t = (float)(frames_played + i) / SAMPLE_RATE;
                int16_t sample = (int16_t)(amplitude * sinf(2.0f * M_PI * freq * t));
                stereo[i * 2]     = sample;
                stereo[i * 2 + 1] = sample;
            }
            size_t w = 0;
            i2s_write(I2S_NUM_1, stereo, cnt * 2 * sizeof(int16_t),
                      &w, pdMS_TO_TICKS(200));
            frames_played += cnt;
            led = !led;
            digitalWrite(LED_PIN, led ? HIGH : LOW);

            if (frames_played % SAMPLE_RATE == 0) {
                Serial.printf("[AudioTest]   %us / %us\n",
                              frames_played / SAMPLE_RATE, DURATION_S);
            }
        }

        // 播放结束后送 200ms 静音，平滑停止
        {
            int16_t *silence = (int16_t *)calloc(SAMPLE_RATE / 5 * 2, sizeof(int16_t));
            size_t w = 0;
            i2s_write(I2S_NUM_1, silence, SAMPLE_RATE / 5 * 2 * sizeof(int16_t),
                      &w, pdMS_TO_TICKS(300));
            free(silence);
        }

        digitalWrite(LED_PIN, LOW);
        free(stereo);
        Serial.println("[AudioTest] ===== Speaker Test Complete =====");
        Serial.println("[AudioTest] Should hear a clean 440Hz 'beep' tone.");
        Serial.println("[AudioTest] If crackling: check BCLK/LRCLK/DIN wiring quality.");

        i2s_driver_uninstall(I2S_NUM_1);
    }

    // ============================================================
    //  完整测试：录音 + 回放
    // ============================================================
    static void testRecordAndPlay()
    {
        Serial.println("[AudioTest] ========== Record + Playback Test ==========");
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, LOW);

        // ============================================================
        //  PHASE 1: 录音（仅安装麦克风 I2S）
        //  INMP441 输出 24-bit 左对齐，ESP32-S3 I2S 32-bit 模式读取
        //  正确提取：(int16_t)((uint32_t)raw >> 8) 取 bits[23:8]
        // ============================================================
        i2s_driver_uninstall(I2S_NUM_0);
        i2s_driver_uninstall(I2S_NUM_1);

        const i2s_config_t mic_cfg = {
            .mode        = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
            .channel_format  = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 4,
            .dma_buf_len   = CHUNK,
            .use_apll = true,       // APLL 减少时钟抖动，降低录音底噪
            .tx_desc_auto_clear = false,
        };
        const i2s_pin_config_t mic_pins = {
            .bck_io_num   = MIC_BCK,
            .ws_io_num    = MIC_WS,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num  = MIC_DIN,
        };
        ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &mic_cfg, 0, nullptr));
        ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &mic_pins));
        i2s_zero_dma_buffer(I2S_NUM_0);

        // 32-bit stereo: 每帧 2 个 int32_t (L + R)
        size_t dma_bytes = CHUNK * 2 * sizeof(int32_t);
        int32_t *dma_buf = (int32_t *)malloc(dma_bytes);
        int16_t *pcm = (int16_t *)malloc(TOTAL_SAMPLES * sizeof(int16_t));

        if (!dma_buf || !pcm) {
            Serial.println("[AudioTest] FATAL: alloc failed");
            free(dma_buf); free(pcm);
            i2s_driver_uninstall(I2S_NUM_0);
            return;
        }

        // 丢弃前 500ms（I2S 时钟稳定）
        {
            size_t discard_bytes = (SAMPLE_RATE / 2) * 2 * sizeof(int32_t);
            int32_t *discard = (int32_t *)malloc(discard_bytes);
            if (discard) {
                size_t dummy = 0;
                i2s_read(I2S_NUM_0, discard, discard_bytes, &dummy, pdMS_TO_TICKS(600));
                free(discard);
            }
        }

        Serial.printf("[AudioTest] PHASE 1: Recording %us (32-bit stereo)...\n", DURATION_S);
        digitalWrite(LED_PIN, HIGH);

        uint32_t frames_captured = 0;
        int64_t sumL = 0;
        int32_t peakL = 0;

        while (frames_captured < TOTAL_SAMPLES) {
            uint32_t need = TOTAL_SAMPLES - frames_captured;
            uint32_t to_read = (need < CHUNK) ? need : CHUNK;
            size_t bytes_read = 0;

            i2s_read(I2S_NUM_0, dma_buf, to_read * 2 * sizeof(int32_t),
                     &bytes_read, pdMS_TO_TICKS(200));

            uint32_t frames_read = bytes_read / (2 * sizeof(int32_t));
            for (uint32_t i = 0; i < frames_read; i++) {
                int32_t l_raw = dma_buf[i * 2];       // L channel
                // 正确提取 16-bit 音频：逻辑右移 8 位，取 bits[23:8]
                int16_t sample = (int16_t)((uint32_t)l_raw >> 8);
                pcm[frames_captured + i] = sample;
                sumL += sample;
                int32_t absL = sample < 0 ? -sample : sample;
                if (absL > peakL) peakL = absL;

                if ((frames_captured + i) % SAMPLE_RATE == 0) {
                    Serial.printf("  @%us: raw=0x%08lx  pcm=%d\n",
                                  (frames_captured + i) / SAMPLE_RATE,
                                  (unsigned long)l_raw,
                                  pcm[frames_captured + i]);
                }
            }
            frames_captured += frames_read;
        }

        digitalWrite(LED_PIN, LOW);
        Serial.printf("[AudioTest] Recorded %u frames, peak=%ld\n",
                      frames_captured, peakL);

        // 卸载麦克风 I2S
        free(dma_buf);
        i2s_driver_uninstall(I2S_NUM_0);

        if (peakL < 50) {
            Serial.println("[AudioTest] SILENT - mic not working");
            free(pcm);
            return;
        }

        // 去除 DC 偏移
        {
            int32_t mean = (int32_t)(sumL / (int64_t)frames_captured);
            Serial.printf("[AudioTest] DC offset = %ld, removing...\n", mean);
            for (uint32_t i = 0; i < frames_captured; i++) {
                int32_t v = (int32_t)pcm[i] - mean;
                if (v > 32767) v = 32767;
                if (v < -32768) v = -32768;
                pcm[i] = (int16_t)v;
            }
        }

        // ============================================================
        //  PHASE 2: 回放（仅安装扬声器 I2S）
        // ============================================================
        Serial.println("[AudioTest] PHASE 2: Playback...");

        const i2s_config_t spk_cfg = {
            .mode        = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format  = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,
            .dma_buf_len   = 256,
            .use_apll = true,
            .tx_desc_auto_clear = true,
        };
        const i2s_pin_config_t spk_pins = {
            .bck_io_num   = SPK_BCLK,
            .ws_io_num    = SPK_LRCK,
            .data_out_num = SPK_DIN,
            .data_in_num  = I2S_PIN_NO_CHANGE,
        };
        ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_1, &spk_cfg, 0, nullptr));
        ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_1, &spk_pins));
        i2s_zero_dma_buffer(I2S_NUM_1);

        // DMA 预热：500ms 静音
        {
            int16_t *silence = (int16_t *)calloc(SAMPLE_RATE / 2 * 2, sizeof(int16_t));
            size_t w = 0;
            i2s_write(I2S_NUM_1, silence, SAMPLE_RATE / 2 * 2 * sizeof(int16_t),
                      &w, pdMS_TO_TICKS(600));
            free(silence);
        }

        // 播放
        const uint32_t PLAY_CHUNK_SZ = SAMPLE_RATE / 10;
        int16_t *stereo = (int16_t *)malloc(PLAY_CHUNK_SZ * 2 * sizeof(int16_t));
        uint32_t pos = 0;
        bool led = false;

        while (pos < frames_captured) {
            uint32_t cnt = min(PLAY_CHUNK_SZ, frames_captured - pos);
            for (uint32_t i = 0; i < cnt; i++) {
                stereo[i * 2]     = pcm[pos + i];
                stereo[i * 2 + 1] = pcm[pos + i];
            }
            size_t w = 0;
            i2s_write(I2S_NUM_1, stereo, cnt * 2 * sizeof(int16_t),
                      &w, pdMS_TO_TICKS(200));
            pos += cnt;
            led = !led;
            digitalWrite(LED_PIN, led ? HIGH : LOW);
        }

        // 尾部静音
        {
            int16_t *silence = (int16_t *)calloc(SAMPLE_RATE / 5 * 2, sizeof(int16_t));
            size_t w = 0;
            i2s_write(I2S_NUM_1, silence, SAMPLE_RATE / 5 * 2 * sizeof(int16_t),
                      &w, pdMS_TO_TICKS(300));
            free(silence);
        }

        free(stereo);
        free(pcm);

        digitalWrite(LED_PIN, LOW);
        Serial.println("[AudioTest] ========== Test Complete ==========");
        i2s_driver_uninstall(I2S_NUM_1);
    }

    // ============================================================
    //  播放多音符旋律（验证扬声器音质）
    // ============================================================
    static void testMelody()
    {
        Serial.println("[AudioTest] ===== Melody Test =====");
        pinMode(LED_PIN, OUTPUT);

        i2s_driver_uninstall(I2S_NUM_1);

        const i2s_config_t spk_cfg = {
            .mode        = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
            .channel_format  = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 8,
            .dma_buf_len   = 256,
            .use_apll = true,
            .tx_desc_auto_clear = true,
        };
        const i2s_pin_config_t spk_pins = {
            .bck_io_num   = SPK_BCLK,
            .ws_io_num    = SPK_LRCK,
            .data_out_num = SPK_DIN,
            .data_in_num  = I2S_PIN_NO_CHANGE,
        };
        ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_1, &spk_cfg, 0, nullptr));
        ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_1, &spk_pins));
        i2s_zero_dma_buffer(I2S_NUM_1);

        // 音符频率表 (Hz)，0 = 休止
        static const float notes[] = {
            262, 294, 330, 349, 392, 440, 494, 523,  // C4 D4 E4 F4 G4 A4 B4 C5
            523, 494, 440, 392, 349, 330, 294, 262,  // 下行
            0,   262, 330, 392, 523, 392, 330, 262   // 跳跃旋律
        };
        static const int noteCount = sizeof(notes) / sizeof(notes[0]);
        const uint32_t noteDuration = SAMPLE_RATE / 3;  // 每个音符 ~333ms
        const float amplitude = 24000.0f;

        int16_t *buf = (int16_t *)malloc(noteDuration * 2 * sizeof(int16_t));
        Serial.printf("[AudioTest] Playing %d notes (organ-like with harmonics)...\n", noteCount);

        for (int n = 0; n < noteCount; n++) {
            float freq = notes[n];
            for (uint32_t i = 0; i < noteDuration; i++) {
                int16_t sample = 0;
                if (freq > 0) {
                    float t_note = (float)i / SAMPLE_RATE;
                    // 包络: attack + sustain + release
                    float env = 1.0f;
                    if (t_note < 0.015f) env = t_note / 0.015f;                   // 15ms attack
                    else if (t_note > 0.27f) env = (0.333f - t_note) / 0.063f;    // release
                    if (env < 0) env = 0;
                    if (env > 1) env = 1;

                    // 叠加谐波模拟管风琴音色
                    float phase = 2.0f * M_PI * freq * t_note;
                    float sig = sinf(phase) * 0.6f           // 基频 60%
                              + sinf(phase * 2.0f) * 0.25f   // 2次谐波 25%
                              + sinf(phase * 3.0f) * 0.1f    // 3次谐波 10%
                              + sinf(phase * 4.0f) * 0.05f;  // 4次谐波 5%

                    sample = (int16_t)(amplitude * env * sig);
                }
                buf[i * 2]     = sample;
                buf[i * 2 + 1] = sample;
            }
            size_t w = 0;
            i2s_write(I2S_NUM_1, buf, noteDuration * 2 * sizeof(int16_t),
                      &w, pdMS_TO_TICKS(500));
            digitalWrite(LED_PIN, (n % 2) ? HIGH : LOW);
        }

        // 尾部静音
        {
            int16_t *silence = (int16_t *)calloc(SAMPLE_RATE / 5 * 2, sizeof(int16_t));
            size_t w = 0;
            i2s_write(I2S_NUM_1, silence, SAMPLE_RATE / 5 * 2 * sizeof(int16_t),
                      &w, pdMS_TO_TICKS(300));
            free(silence);
        }

        free(buf);
        digitalWrite(LED_PIN, LOW);
        Serial.println("[AudioTest] ===== Melody Test Complete =====");
        Serial.println("[AudioTest] Should hear a clean ascending-descending scale.");
        i2s_driver_uninstall(I2S_NUM_1);
    }

    // ============================================================
    //  录音并通过串口输出 WAV 格式（方便 PC 端分析）
    // ============================================================
    static void testRecordDumpWav()
    {
        Serial.println("[AudioTest] ===== Record + WAV Dump =====");
        Serial.println("[AudioTest] Will output WAV binary data after recording.");
        Serial.println("[AudioTest] Capture with: pio device monitor > recording.wav");
        Serial.println("[AudioTest] (or redirect serial output to file)");
        pinMode(LED_PIN, OUTPUT);
        digitalWrite(LED_PIN, LOW);

        i2s_driver_uninstall(I2S_NUM_0);

        const i2s_config_t mic_cfg = {
            .mode        = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
            .sample_rate = SAMPLE_RATE,
            .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
            .channel_format  = I2S_CHANNEL_FMT_RIGHT_LEFT,
            .communication_format = I2S_COMM_FORMAT_STAND_I2S,
            .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
            .dma_buf_count = 4,
            .dma_buf_len   = CHUNK,
            .use_apll = true,       // APLL 减少时钟抖动，降低录音底噪
            .tx_desc_auto_clear = false,
        };
        const i2s_pin_config_t mic_pins = {
            .bck_io_num   = MIC_BCK,
            .ws_io_num    = MIC_WS,
            .data_out_num = I2S_PIN_NO_CHANGE,
            .data_in_num  = MIC_DIN,
        };
        ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &mic_cfg, 0, nullptr));
        ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &mic_pins));
        i2s_zero_dma_buffer(I2S_NUM_0);

        size_t dma_bytes = CHUNK * 2 * sizeof(int32_t);
        int32_t *dma_buf = (int32_t *)malloc(dma_bytes);
        int16_t *pcm = (int16_t *)malloc(TOTAL_SAMPLES * sizeof(int16_t));

        if (!dma_buf || !pcm) {
            Serial.println("[AudioTest] FATAL: alloc failed");
            free(dma_buf); free(pcm);
            i2s_driver_uninstall(I2S_NUM_0);
            return;
        }

        // 丢弃前 500ms
        {
            size_t discard_bytes = (SAMPLE_RATE / 2) * 2 * sizeof(int32_t);
            int32_t *discard = (int32_t *)malloc(discard_bytes);
            if (discard) {
                size_t dummy = 0;
                i2s_read(I2S_NUM_0, discard, discard_bytes, &dummy, pdMS_TO_TICKS(600));
                free(discard);
            }
        }

        Serial.printf("[AudioTest] Recording %us...\n", DURATION_S);
        digitalWrite(LED_PIN, HIGH);

        uint32_t frames_captured = 0;
        int64_t sumL = 0;
        int32_t peakL = 0;
        int64_t sumSq = 0;  // 用于计算 RMS
        int16_t minSample = 32767;
        int16_t maxSample = -32768;
        uint32_t total_bytes_read = 0;
        bool first_chunk = true;

        while (frames_captured < TOTAL_SAMPLES) {
            uint32_t need = TOTAL_SAMPLES - frames_captured;
            uint32_t to_read = (need < CHUNK) ? need : CHUNK;
            size_t bytes_read = 0;
            i2s_read(I2S_NUM_0, dma_buf, to_read * 2 * sizeof(int32_t),
                     &bytes_read, pdMS_TO_TICKS(200));
            uint32_t frames_read = bytes_read / (2 * sizeof(int32_t));
            total_bytes_read += bytes_read;

            // 打印第一批数据的原始 DMA 数据（诊断）
            if (first_chunk && frames_read >= 10) {
                first_chunk = false;
                Serial.println("[AudioTest] ── First 10 DMA frames (raw) ──");
                Serial.println("[AudioTest]  #   L_raw(32bit)    R_raw(32bit)    L_pcm16  R_pcm16");
                for (uint32_t i = 0; i < 10; i++) {
                    int32_t l_raw = dma_buf[i * 2];
                    int32_t r_raw = dma_buf[i * 2 + 1];
                    int16_t l_pcm = (int16_t)((uint32_t)l_raw >> 8);
                    int16_t r_pcm = (int16_t)((uint32_t)r_raw >> 8);
                    Serial.printf("[AudioTest] %2d  0x%08lx  0x%08lx  %6d  %6d\n",
                                  i, (unsigned long)l_raw, (unsigned long)r_raw, l_pcm, r_pcm);
                }
                Serial.printf("[AudioTest] bytes_read=%u, expected=%u\n",
                              bytes_read, to_read * 2 * sizeof(int32_t));
            }

            for (uint32_t i = 0; i < frames_read; i++) {
                int32_t l_raw = dma_buf[i * 2];
                int16_t sample = (int16_t)((uint32_t)l_raw >> 8);
                pcm[frames_captured + i] = sample;
                sumL += sample;
                sumSq += (int64_t)sample * sample;
                if (sample < minSample) minSample = sample;
                if (sample > maxSample) maxSample = sample;
                int32_t absS = sample < 0 ? -sample : sample;
                if (absS > peakL) peakL = absS;
            }
            frames_captured += frames_read;
        }

        digitalWrite(LED_PIN, LOW);
        free(dma_buf);
        i2s_driver_uninstall(I2S_NUM_0);

        // ─── 诊断统计 ───
        int32_t mean = (int32_t)(sumL / (int64_t)frames_captured);
        int32_t rms = (int32_t)sqrt((double)sumSq / frames_captured);
        Serial.println("[AudioTest] ── Recording Statistics ──");
        Serial.printf("[AudioTest]   Frames   : %u / %u\n", frames_captured, TOTAL_SAMPLES);
        Serial.printf("[AudioTest]   DMA bytes: %u (expected %u)\n",
                      total_bytes_read, TOTAL_SAMPLES * 2 * sizeof(int32_t));
        Serial.printf("[AudioTest]   DC offset: %ld\n", mean);
        Serial.printf("[AudioTest]   Peak     : %ld\n", peakL);
        Serial.printf("[AudioTest]   RMS      : %ld\n", rms);
        Serial.printf("[AudioTest]   Min/Max  : %d / %d\n", minSample, maxSample);
        Serial.printf("[AudioTest]   Range    : %d\n", maxSample - minSample);

        if (peakL < 50) {
            Serial.println("[AudioTest]   ⚠ SILENT — mic not capturing audio (check wiring)");
        } else if (rms > 5000 && peakL > 25000) {
            Serial.println("[AudioTest]   ⚠ HIGH NOISE — possible hardware issue");
        } else if (rms < 200 && peakL < 1000) {
            Serial.println("[AudioTest]   ✓ LOW signal — quiet environment, try speaking into mic");
        } else {
            Serial.println("[AudioTest]   ✓ Signal looks normal");
        }

        // 去除 DC 偏移
        Serial.printf("[AudioTest] Removing DC offset (%ld)...\n", mean);
        for (uint32_t i = 0; i < frames_captured; i++) {
            int32_t v = (int32_t)pcm[i] - mean;
            if (v > 32767) v = 32767;
            if (v < -32768) v = -32768;
            pcm[i] = (int16_t)v;
        }

        // ─── 输出 WAV 二进制数据 ───
        // 使用文本标记行 "WAVBIN:XXXXX\n" 让捕获脚本精确定位
        uint32_t dataBytes = frames_captured * sizeof(int16_t);
        uint32_t fileSize = 44 + dataBytes;  // WAV header + PCM data
        Serial.printf("WAVBIN:%u\n", fileSize);  // 精确标记，捕获脚本用此行定位
        delay(50);
        Serial.flush();
        delay(50);

        // WAV header (44 bytes)
        uint8_t wavHeader[44] = {0};
        wavHeader[0] = 'R'; wavHeader[1] = 'I'; wavHeader[2] = 'F'; wavHeader[3] = 'F';
        uint32_t chunkSize = 36 + dataBytes;
        memcpy(&wavHeader[4], &chunkSize, 4);
        wavHeader[8] = 'W'; wavHeader[9] = 'A'; wavHeader[10] = 'V'; wavHeader[11] = 'E';
        wavHeader[12] = 'f'; wavHeader[13] = 'm'; wavHeader[14] = 't'; wavHeader[15] = ' ';
        uint32_t fmtSize = 16;
        memcpy(&wavHeader[16], &fmtSize, 4);
        uint16_t audioFmt = 1;
        memcpy(&wavHeader[20], &audioFmt, 2);
        uint16_t numChannels = 1;
        memcpy(&wavHeader[22], &numChannels, 2);
        uint32_t sr = SAMPLE_RATE;
        memcpy(&wavHeader[24], &sr, 4);
        uint32_t byteRate = SAMPLE_RATE * sizeof(int16_t);
        memcpy(&wavHeader[28], &byteRate, 4);
        uint16_t blockAlign = sizeof(int16_t);
        memcpy(&wavHeader[32], &blockAlign, 2);
        uint16_t bitsPerSample = 16;
        memcpy(&wavHeader[34], &bitsPerSample, 2);
        wavHeader[36] = 'd'; wavHeader[37] = 'a'; wavHeader[38] = 't'; wavHeader[39] = 'a';
        memcpy(&wavHeader[40], &dataBytes, 4);

        Serial.write(wavHeader, 44);
        Serial.write((uint8_t *)pcm, dataBytes);
        Serial.flush();
        delay(100);

        Serial.printf("\n[AudioTest] WAV sent: %u bytes, %u frames, %uHz mono 16-bit\n",
                      fileSize, frames_captured, SAMPLE_RATE);

        free(pcm);
    }

    static void pollSerial()
    {
        if (Serial.available()) {
            char c = Serial.read();
            bool isAudioTest = (c == 'a' || c == 's' || c == 'm' || c == 'w' || c == 'p');
            if (isAudioTest) {
                // 音频测试前屏蔽 BootButton，覆盖整个测试过程
                // GPIO0 在 I2S 操作期间可能受 EMI 干扰读低
                BootButton::getInstance().suppressFor(30000);
            }
            if (c == 'a') testRecordAndPlay();
            else if (c == 's') testSpeaker();
            else if (c == 'm') testMelody();
            else if (c == 'w') testRecordDumpWav();
            else if (c == 'p') probeGPIO();
            else if (c == 'f') BootButton::serialFactoryReset();

            if (isAudioTest) {
                // 测试结束后再屏蔽 5s，等待 GPIO0 稳定
                BootButton::getInstance().suppressFor(5000);
            }
        }
    }

    // ============================================================
    //  GPIO 探测：不启动 I2S，直接手动翻转引脚
    // ============================================================
    static void probeGPIO()
    {
        Serial.println("[AudioTest] ===== GPIO Probe =====");
        Serial.println("[AudioTest] Toggling speaker pins for 5s...");
        Serial.println("[AudioTest] Use multimeter (AC voltage mode) on each pin:");
        Serial.printf("[AudioTest]   IO%d (BCLK)  → expect ~0.5-1.5V AC\n", SPK_BCLK);
        Serial.printf("[AudioTest]   IO%d (LRCLK) → expect ~0.5-1.5V AC\n", SPK_LRCK);
        Serial.printf("[AudioTest]   IO%d (DIN)   → expect ~0.5-1.5V AC\n", SPK_DIN);
        Serial.println("[AudioTest] If any pin reads 0V → broken connection");

        pinMode(SPK_BCLK, OUTPUT);
        pinMode(SPK_LRCK, OUTPUT);
        pinMode(SPK_DIN, OUTPUT);

        unsigned long start = millis();
        uint32_t count = 0;
        while (millis() - start < 5000) {
            digitalWrite(SPK_BCLK, (count % 2) ? HIGH : LOW);
            digitalWrite(SPK_LRCK, (count % 32) ? HIGH : LOW);
            digitalWrite(SPK_DIN, (count % 3) ? HIGH : LOW);
            count++;
            delayMicroseconds(10);
        }

        Serial.printf("[AudioTest] Toggled %u times in 5s\n", (unsigned)count);
        Serial.println("[AudioTest] ===== Probe Complete =====");

        // 恢复为输入（避免和后续 I2S 冲突）
        pinMode(SPK_BCLK, INPUT);
        pinMode(SPK_LRCK, INPUT);
        pinMode(SPK_DIN, INPUT);
    }
};

#endif // AUDIO_TEST_H
