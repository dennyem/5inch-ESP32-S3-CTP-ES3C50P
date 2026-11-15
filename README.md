# 5inch-ESP32-S3-CTP-ES3C50P
ESP32S3 Board with 5inch Display attached.  Available from Alibaba and others.

The example code that is available for this board is flawed, in that the display (RGB565 direct port from the ESP32S3) can experience severe jitter.

This repository contains the audio (MP3 I2S) player example with comments translated to English and a correction to make the display more stable.

ESP32-S3 RGB display instability, tearing, wrapping and jittering when using Arduino_GFX and a PSRAM framebuffer can be resolved by enabling a non-zero bounce buffer. On an 800×480 panel, a 4000-pixel bounce buffer (5 scan lines) completely eliminated display instability even under concurrent I2S audio DMA load.  If heavier DMA load starts producing artifacts or jittering the buffer can be increased to 8000-pixels (10 scan lines).
