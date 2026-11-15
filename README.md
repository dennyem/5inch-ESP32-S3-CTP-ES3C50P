# 5inch-ESP32-S3-CTP-ES3C50P
ESP32S3 Board with 5inch Display attached.  Available from Alibaba and others.

The example code that is available for this board is flawed, in that the display (RGB565 direct port from the ESP32S3) can experience severe jitter.

This repository contains the audio (MP3 I2S) player example with comments translated to English and a correction to make the display more stable.

ESP32-S3 RGB display instability, tearing, wrapping and jittering when using Arduino_GFX and a PSRAM framebuffer can be resolved by enabling a non-zero bounce buffer. On an 800×480 panel, a 4000-pixel bounce buffer (5 scan lines) completely eliminated display instability even under concurrent I2S audio DMA load.  If heavier DMA load starts producing artifacts or jittering the buffer can be increased to 8000-pixels (10 scan lines).

The init code below is the default supplied by the manufacturer.

Arduino_ESP32RGBPanel *rgb_panel = new Arduino_ESP32RGBPanel(
    PIN_DE, PIN_VSYNC, PIN_HSYNC, PIN_PCLK,
    PIN_R0, PIN_R1, PIN_R2, PIN_R3, PIN_R4,
    PIN_G0, PIN_G1, PIN_G2, PIN_G3, PIN_G4, PIN_G5,
    PIN_B0, PIN_B1, PIN_B2, PIN_B3, PIN_B4,
    HSYNC_POLARITY, HSYNC_FRONT_PORCH, HSYNC_PULSE_WIDTH, HSYNC_BACK_PORCH,
    VSYNC_POLARITY, VSYNC_FRONT_PORCH, VSYNC_PULSE_WIDTH, VSYNC_BACK_PORCH,
    PCLK_ACTIVE_NEG, PCLK_FREQ
)



This is the corrected version.

Arduino_ESP32RGBPanel *rgb_panel = new Arduino_ESP32RGBPanel(
    PIN_DE, PIN_VSYNC, PIN_HSYNC, PIN_PCLK,
    PIN_R0, PIN_R1, PIN_R2, PIN_R3, PIN_R4,
    PIN_G0, PIN_G1, PIN_G2, PIN_G3, PIN_G4, PIN_G5,
    PIN_B0, PIN_B1, PIN_B2, PIN_B3, PIN_B4,
    HSYNC_POLARITY, HSYNC_FRONT_PORCH, HSYNC_PULSE_WIDTH, HSYNC_BACK_PORCH,
    VSYNC_POLARITY, VSYNC_FRONT_PORCH, VSYNC_PULSE_WIDTH, VSYNC_BACK_PORCH,
    PCLK_ACTIVE_NEG, PCLK_FREQ,
    
    /* The settings below are not included in the files provided by the board manufacturer.
     * They help to reduce the effects of DMA timing issues causing jitter and random artifacts on the display
     */

    false,   // useBigEndian        
    0,       // de_idle_high
    0,       // pclk_idle_high
    8000     // bounce buffer size (10 scan lines).  
)
