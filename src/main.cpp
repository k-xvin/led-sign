#include <Arduino.h>
#include <FastLED.h>

typedef unsigned char u8;

enum {
    kMatrixWidth = 30,
    kMatrixHeight = 5,
    kNumLeds = kMatrixHeight * kMatrixWidth,
    kDataPin = 25,
    kMaxMilliamps = 300,  // 300mA is plenty bright
};

// Based on https://alasseearfalas.itch.io/another-tiny-pixel-font-mono-3x5
const u8 font3x5[][5] = {
    {0b000, 0b000, 0b000, 0b000, 0b000}, // ' '
    {0b010, 0b101, 0b111, 0b101, 0b101}, // 'A'
    {0b110, 0b101, 0b110, 0b101, 0b110}, // 'B'
    {0b011, 0b100, 0b100, 0b100, 0b011}, // 'C'
    {0b110, 0b101, 0b101, 0b101, 0b110}, // 'D'
    {0b111, 0b100, 0b110, 0b100, 0b111}, // 'E'
    {0b111, 0b100, 0b110, 0b100, 0b100}, // 'F'
    {0b011, 0b100, 0b101, 0b101, 0b011}, // 'G'
    {0b101, 0b101, 0b111, 0b101, 0b101}, // 'H'
    {0b111, 0b010, 0b010, 0b010, 0b111}, // 'I'
    {0b001, 0b001, 0b001, 0b101, 0b010}, // 'J'
    {0b101, 0b101, 0b110, 0b101, 0b101}, // 'K'
    {0b100, 0b100, 0b100, 0b100, 0b111}, // 'L'
    {0b101, 0b111, 0b111, 0b101, 0b101}, // 'M'
    {0b111, 0b101, 0b101, 0b101, 0b101}, // 'N'
    {0b010, 0b101, 0b101, 0b101, 0b010}, // 'O'
    {0b111, 0b101, 0b111, 0b100, 0b100}, // 'P'
    {0b011, 0b101, 0b101, 0b110, 0b011}, // 'Q'
    {0b111, 0b101, 0b110, 0b101, 0b101}, // 'R'
    {0b011, 0b100, 0b111, 0b001, 0b110}, // 'S'
    {0b111, 0b010, 0b010, 0b010, 0b010}, // 'T'
    {0b101, 0b101, 0b101, 0b101, 0b111}, // 'U'
    {0b101, 0b101, 0b101, 0b010, 0b010}, // 'V'
    {0b101, 0b101, 0b111, 0b111, 0b101}, // 'W'
    {0b101, 0b101, 0b010, 0b101, 0b101}, // 'X'
    {0b101, 0b101, 0b010, 0b010, 0b010}, // 'Y'
    {0b111, 0b001, 0b010, 0b100, 0b111}  // 'Z'
};

CRGB s_leds[kNumLeds];

// Serpentine mapping for 2D matrix
u8 XY(u8 x, u8 y) {
    if (x >= kMatrixWidth || y >= kMatrixHeight) return 0;
    if (y % 2 == 0) {
        return y * kMatrixWidth + x;
    } else {
        return y * kMatrixWidth + (kMatrixWidth - 1 - x);
    }
}

// Map ASCII to font index
u8 getFontIndex(char c) {
    if (c == ' ') return 0;
    if (c >= 'A' && c <= 'Z') return (c - 'A') + 1;
    return 0;  // fallback to space
}

// Build text bitmap: rows[5][pixel columns]
void buildTextBitmap(const char* text, u8 rows[5][300], int& pixelWidth) {
    int col = 0;
    for (const char* p = text; *p; p++) {
        u8 idx = getFontIndex(toupper(*p));
        // For each column of the character (3 columns), left-to-right
        for (int8_t c = 0; c < 3; c++) {
            for (u8 r = 0; r < 5; r++) {
                rows[r][col] = (font3x5[idx][r] >> (2 - c)) & 0x01;
            }
            col++;
        }
        // Add 1 column spacing between characters
        for (u8 r = 0; r < 5; r++) {
            rows[r][col] = 0;
        }
        col++;
    }
    pixelWidth = col;
}

void drawBitmap(u8 rows[5][300], int pixelWidth, int scrollOffset, CRGB color) {
    FastLED.clear();
    for (u8 row = 0; row < 5; row++) {
        for (u8 col = 0; col < kMatrixWidth; col++) {
            // Reverse the column order for left-to-right text
            int bitmapCol = (kMatrixWidth - 1 - col) + scrollOffset;
            u8 displayRow = row;
            if (bitmapCol >= 0 && bitmapCol < pixelWidth &&
                rows[row][bitmapCol]) {
                s_leds[XY(col, displayRow)] = color;
            }
        }
    }
    FastLED.show();
}

void setup() {
    FastLED.addLeds<WS2812B, kDataPin, GRB>(s_leds, kNumLeds);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, kMaxMilliamps);
    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();
}

void loop() {
    const char* message = "SOMETIMES THERE ARE PROBLEMS     DO NOT FRET";
    static u8 textBitmap[5][300];
    static int messagePixelWidth = 0;
    static int scrollOffset = -kMatrixWidth;
    static bool initialized = false;
    if (!initialized) {
        buildTextBitmap(message, textBitmap, messagePixelWidth);
        initialized = true;
    }
    drawBitmap(textBitmap, messagePixelWidth, scrollOffset, CRGB::Red);
    scrollOffset++;
    if (scrollOffset > messagePixelWidth) {
        scrollOffset = -kMatrixWidth;
    }
    delay(60);
}