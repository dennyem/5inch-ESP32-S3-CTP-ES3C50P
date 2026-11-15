#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Audio.h>
#include <U8g2lib.h>
#include <Arduino_GFX_Library.h>

// ====================== I2S Audio Pins ======================
#define I2S_BCLK     0
#define I2S_LRCLK    18
#define I2S_SDATA    17

// ====================== SD Card SPI Pins ======================
#define SD_CS        10
#define SD_SCLK      12
#define SD_MISO      13
#define SD_MOSI      11

// ====================== RGB LCD Configuration ======================
#define TFT_BL       2
#define LCD_WIDTH    800
#define LCD_HEIGHT   480

#define HSYNC_POLARITY 0
#define HSYNC_FRONT_PORCH 8    // Thfp Typ=8
#define HSYNC_PULSE_WIDTH 4    // Thw Typ=4
#define HSYNC_BACK_PORCH 8     // Thbp Typ=8

#define VSYNC_POLARITY 0
#define VSYNC_FRONT_PORCH 8    // Tvfp Typ=8
#define VSYNC_PULSE_WIDTH 4    // Tvw Typ=4
#define VSYNC_BACK_PORCH 8     // Tvbp Typ=8

#define PCLK_FREQ 14000000
#define PCLK_ACTIVE_NEG 1

#define PIN_DE    40
#define PIN_VSYNC 41
#define PIN_HSYNC 39
#define PIN_PCLK  42

#define PIN_R0 45
#define PIN_R1 48
#define PIN_R2 47
#define PIN_R3 21
#define PIN_R4 14

#define PIN_G0 5
#define PIN_G1 6
#define PIN_G2 7
#define PIN_G3 15
#define PIN_G4 16
#define PIN_G5 4

#define PIN_B0 8
#define PIN_B1 3
#define PIN_B2 46
#define PIN_B3 9
#define PIN_B4 1

// ====================== Playback Configuration ======================
#define MAX_MP3_FILES 50

char mp3Playlist[MAX_MP3_FILES][64];
int playlistCount = 0;
int currentSongIndex = 0;
bool isPlaying = false;

#define SPI_CLOCK_SPEED 10000000
#define VOLUME_LEVEL 8

// ====================== Global Objects ======================
Audio audio;

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
);

Arduino_RGB_Display *gfx = new Arduino_RGB_Display(
    LCD_WIDTH, LCD_HEIGHT, rgb_panel, 0, true
);

// ====================== Display Initialization ======================
void initDisplay()
{
    // Serial.printf("PSRAM before: %u\n", ESP.getFreePsram());

    gfx->begin();
    
    // Serial.printf("PSRAM after : %u\n", ESP.getFreePsram());    
    
    gfx->fillScreen(RGB565_BLACK);
    gfx->setUTF8Print(true);

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    gfx->setFont(u8g2_font_helvB14_tf);
    gfx->setTextSize(2);
}

// ====================== Display: Now Playing ======================
void showPlayingScreen(const char* fileName)
{
    static char oldFileName[64];

    if(strcmp(oldFileName, fileName) != 0) {
        strcpy(oldFileName, fileName);
        gfx->fillScreen(RGB565_BLACK);
        gfx->setTextColor(RGB565_WHITE);

        gfx->setCursor(40, 100);
        gfx->print("Now Playing:");

        gfx->setCursor(40, 160);
        gfx->print(fileName);
    }
}

// ====================== Display: No Audio Files Found ======================
void showNoAudioScreen()
{
    gfx->fillScreen(RGB565_BLACK);
    gfx->setTextColor(RGB565_YELLOW);

    gfx->setCursor(40, 150);
    gfx->print("No audio files found on SD card");

    gfx->setCursor(40, 210);
    gfx->print("Add MP3 files and restart");
}

// ====================== Display: Initialising ======================
void showInitScreen()
{
    gfx->fillScreen(RGB565_BLACK);
    gfx->setTextColor(RGB565_CYAN);

    gfx->setCursor(10, 180);
    gfx->print("System Initialising...");
}

// ====================== Scan MP3 Files ======================
void scanMp3Files()
{
    Serial.println("Scanning MP3 files...");
    playlistCount = 0;

    File root = SD.open("/");
    if (!root) return;

    File file = root.openNextFile();

    while (file)
    {
        if (!file.isDirectory())
        {
            const char* name = file.name();
            int len = strlen(name);

            if (len > 4 &&
                strcasecmp(name + len - 4, ".mp3") == 0)
            {
                if (playlistCount < MAX_MP3_FILES)
                {
                    strncpy(mp3Playlist[playlistCount], name, 63);
                    Serial.println(name);
                    playlistCount++;
                }
            }
        }

        file.close();
        file = root.openNextFile();
    }

    root.close();
}

// ====================== Play Track By Index ======================
bool playSongByIndex(int index)
{
    if (index < 0 || index >= playlistCount)
    {
        isPlaying = false;
        return false;
    }

    audio.stopSong();
    delay(100);

    const char* path = mp3Playlist[index];

    showPlayingScreen(path);   // Automatically update screen with filename

    Serial.printf("Playing: %s\n", path);

    audio.connecttoFS(SD, path);
    delay(500);

    if (!audio.isRunning())
    {
        audio.connecttoFS(SD, path);
        delay(500);

        if (!audio.isRunning())
            return false;
    }

    isPlaying = true;
    currentSongIndex = index;

    return true;
}

// ====================== Next Track ======================
void playNextSong()
{
    currentSongIndex++;

    if (currentSongIndex >= playlistCount)
        currentSongIndex = 0;

    if (!playSongByIndex(currentSongIndex))
    {
        delay(1000);
        playNextSong();
    }
}

// ====================== SETUP ======================
void setup()
{
    Serial.begin(115200);

    initDisplay();
    showInitScreen();
    delay(1000);

    // SD card initialisation
    SPI.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);

    if (!SD.begin(SD_CS, SPI, SPI_CLOCK_SPEED))
    {
        Serial.println("SD Card Initialisation Failed");

        gfx->fillScreen(RGB565_BLACK);
        gfx->setTextColor(RGB565_RED);

        gfx->setCursor(40, 150);
        gfx->print("SD Card Not Detected");

        gfx->setCursor(40, 210);
        gfx->print("Insert SD card and restart");

        while (1);
    }

    scanMp3Files();

    // Audio initialisation
    audio.setPinout(I2S_BCLK, I2S_LRCLK, I2S_SDATA);
    audio.forceMono(true);
    audio.setVolume(VOLUME_LEVEL);

    if (playlistCount > 0)
    {
        playSongByIndex(0);
    }
    else
    {
        showNoAudioScreen();
        while (1);
    }
}

// ====================== LOOP ======================
void loop()
{
    audio.loop();

    static unsigned long lastFeed = millis();

    if (isPlaying && audio.isRunning())
    {
        static uint32_t t = 0;

        if (millis() - t > 1000) {
            t = millis();

            Serial.printf("Heap=%u PSRAM=%u\n", ESP.getFreeHeap(), ESP.getFreePsram());
        }

        lastFeed = millis();
    }

    // Playback stall detection
    if (isPlaying && !audio.isRunning() &&
        (millis() - lastFeed > 3000))
    {
        isPlaying = false;
        delay(500);
        playNextSong();
    }

    // Automatically advance when playback finishes
    if (isPlaying && !audio.isRunning())
    {
        isPlaying = false;
        delay(500);
        playNextSong();
    }

    delay(100);
}
