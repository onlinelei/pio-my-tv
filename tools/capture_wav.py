#!/usr/bin/env python3
"""
从 ESP32 串口捕获 WAV 录音数据。

协议：设备发送文本行 "WAVBIN:XXXXX\n" 后紧跟 XXXXX 字节的纯 WAV 二进制数据。
脚本等待该标记行，然后精确读取指定字节数。

用法：
  1. 关闭 pio device monitor
  2. 运行：~/.platformio/penv/bin/python tools/capture_wav.py
  3. 脚本自动发送 'w' 并捕获
  4. 保存到 /tmp/mic_recording.wav
"""

import sys
import os
import time

# PlatformIO 的 pyserial 路径
_pio_serial = os.path.expanduser(
    "~/.platformio/penv/lib/python3.11/site-packages"
)
if os.path.isdir(_pio_serial):
    sys.path.insert(0, _pio_serial)

import serial

PORT = "/dev/cu.usbmodem14301"
BAUD = 115200
OUTPUT = "/tmp/mic_recording.wav"
MARKER = b"WAVBIN:"


def main():
    print(f"Opening {PORT} at {BAUD} baud...")
    ser = serial.Serial(PORT, BAUD, timeout=2, dsrdtr=False, rtscts=False)
    ser.dtr = False
    ser.rts = False
    time.sleep(0.5)
    ser.reset_input_buffer()

    print("Sending 'w' command to trigger recording...")
    ser.write(b"w\n")
    print("Waiting for WAV data (recording ~3s + processing)...")

    # 逐行读取，等待 WAVBIN:XXXXX 标记
    expected_size = 0
    text_buf = b""

    while True:
        # 读取直到遇到换行符
        line = ser.read_until(b"\n", 4096)
        if not line:
            continue

        text_buf += line

        # 打印设备文本日志（非二进制部分）
        try:
            text = line.decode("utf-8", errors="replace").rstrip()
            if text:
                print(f"  [DEV] {text}")
        except Exception:
            pass

        if line.startswith(MARKER):
            try:
                expected_size = int(line[len(MARKER):].strip())
                print(f"\n  >>> Found WAVBIN marker: {expected_size} bytes expected")
                break
            except ValueError:
                pass

    if expected_size == 0:
        print("ERROR: Did not receive WAVBIN marker.")
        ser.close()
        return

    # 精确读取指定字节数的 WAV 二进制数据
    print(f"  Reading {expected_size} bytes of WAV data...")
    wav_data = bytearray()
    while len(wav_data) < expected_size:
        remaining = expected_size - len(wav_data)
        chunk = ser.read(min(remaining, 4096))
        if not chunk:
            print(f"  WARNING: Timeout. Got {len(wav_data)}/{expected_size} bytes.")
            break
        wav_data.extend(chunk)
        # 进度
        if len(wav_data) % 16000 < len(chunk):
            pct = len(wav_data) * 100 // expected_size
            print(f"  {len(wav_data)} / {expected_size} bytes ({pct}%)")

    ser.close()

    if len(wav_data) >= 44:
        with open(OUTPUT, "wb") as f:
            f.write(wav_data)
        print(f"\nSaved {len(wav_data)} bytes to {OUTPUT}")
        print(f"Play with:  open {OUTPUT}")
        print(f"Or:         ffplay {OUTPUT}")

        # 验证 WAV 头
        if wav_data[:4] == b"RIFF":
            print("WAV header OK (RIFF)")
        else:
            print(f"WARNING: No RIFF header found. First 4 bytes: {wav_data[:4].hex()}")
    else:
        print(f"ERROR: Only got {len(wav_data)} bytes, not enough for a WAV file.")


if __name__ == "__main__":
    main()
