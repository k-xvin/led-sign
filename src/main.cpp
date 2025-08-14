
#include <Arduino.h>
#include <FastLED.h>

#define MATRIX_WIDTH  30
#define MATRIX_HEIGHT 5
#define NUM_LEDS      (MATRIX_WIDTH * MATRIX_HEIGHT)
#define DATA_PIN      25

#define MAX_MILLIAMPS 300

CRGB leds[NUM_LEDS];

// Serpentine mapping for 2D matrix
uint16_t XY(uint8_t x, uint8_t y) {
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) return 0;
    if (y % 2 == 0) {
        return y * MATRIX_WIDTH + x;
    } else {
        return y * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - x);
    }
}

// 4x5 font, each character is 5 rows of 4 bits (left to right)
const uint8_t font4x5[][5] = {
    // ' ' (space)
    {0b0000, 0b0000, 0b0000, 0b0000, 0b0000},
    // 'A'
    {0b0110, 0b1001, 0b1111, 0b1001, 0b1001},
    // 'B'
    {0b1110, 0b1001, 0b1110, 0b1001, 0b1110},
    // 'C'
    {0b0111, 0b1000, 0b1000, 0b1000, 0b0111},
    // 'D'
    {0b1110, 0b1001, 0b1001, 0b1001, 0b1110},
    // 'E'
    {0b1111, 0b1000, 0b1110, 0b1000, 0b1111},
    // 'F'
    {0b1111, 0b1000, 0b1110, 0b1000, 0b1000},
    // 'G'
    {0b0111, 0b1000, 0b1011, 0b1001, 0b0111},
    // 'H'
    {0b1001, 0b1001, 0b1111, 0b1001, 0b1001},
    // 'I'
    {0b1111, 0b0010, 0b0010, 0b0010, 0b1111},
    // 'J'
    {0b0011, 0b0001, 0b0001, 0b1001, 0b0110},
    // 'K'
    {0b1001, 0b1010, 0b1100, 0b1010, 0b1001},
    // 'L'
    {0b1000, 0b1000, 0b1000, 0b1000, 0b1111},
    // 'M'
    {0b1001, 0b1111, 0b1001, 0b1001, 0b1001},
    // 'N'
    {0b1001, 0b1101, 0b1011, 0b1001, 0b1001},
    // 'O'
    {0b0110, 0b1001, 0b1001, 0b1001, 0b0110},
    // 'P'
    {0b1110, 0b1001, 0b1110, 0b1000, 0b1000},
    // 'Q'
    {0b0110, 0b1001, 0b1001, 0b1011, 0b0111},
    // 'R'
    {0b1110, 0b1001, 0b1110, 0b1010, 0b1001},
    // 'S'
    {0b0111, 0b1000, 0b0110, 0b0001, 0b1110},
    // 'T'
    {0b1111, 0b0010, 0b0010, 0b0010, 0b0010},
    // 'U'
    {0b1001, 0b1001, 0b1001, 0b1001, 0b0110},
    // 'V'
    {0b1001, 0b1001, 0b1001, 0b0101, 0b0010},
    // 'W'
    {0b1001, 0b1001, 0b1001, 0b1111, 0b1001},
    // 'X'
    {0b1001, 0b0101, 0b0010, 0b0101, 0b1001},
    // 'Y'
    {0b1001, 0b1001, 0b0110, 0b0010, 0b0010},
    // 'Z'
    {0b1111, 0b0001, 0b0010, 0b0100, 0b1111}};

// Map ASCII to font index
uint8_t getFontIndex(char c) {
    if (c == ' ') return 0;
    if (c >= 'A' && c <= 'Z') return (c - 'A') + 1;
    return 0;  // fallback to space
}

// Build text bitmap: rows[5][pixel columns]
void buildTextBitmap(const char* text, uint8_t rows[5][300], int& pixelWidth) {
    int col = 0;
    for (const char* p = text; *p; p++) {
        uint8_t idx = getFontIndex(toupper(*p));
        // For each column of the character (4 columns), reverse order for
        // left-to-right
        for (int8_t c = 3; c >= 0; c--) {
            for (uint8_t r = 0; r < 5; r++) {
                rows[r][col] = (font4x5[idx][r] >> c) & 0x01;
            }
            col++;
        }
        // Add 1 column spacing between characters
        for (uint8_t r = 0; r < 5; r++) {
            rows[r][col] = 0;
        }
        col++;
    }
    pixelWidth = col;
}

void drawBitmap(uint8_t rows[5][300], int pixelWidth, int scrollOffset,
                CRGB color) {
    FastLED.clear();
    for (uint8_t row = 0; row < 5; row++) {
        for (uint8_t col = 0; col < MATRIX_WIDTH; col++) {
            // Reverse horizontal direction for left-to-right scrolling
            int bitmapCol = (MATRIX_WIDTH - 1 - col) + scrollOffset;
            uint8_t displayRow = row;
            if (bitmapCol >= 0 && bitmapCol < pixelWidth &&
                rows[row][bitmapCol]) {
                leds[XY(col, displayRow)] = color;
            }
        }
    }
    FastLED.show();
}

void setup() {
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_MILLIAMPS);
    FastLED.setBrightness(255);
    FastLED.clear();
    FastLED.show();
}

void loop() {
    const char* message = "SOMETIMES THERE ARE PROBLEMS     DO NOT FRET";
    static uint8_t textBitmap[5][300];
    static int messagePixelWidth = 0;
    static int scrollOffset = -MATRIX_WIDTH;
    static bool initialized = false;
    if (!initialized) {
        buildTextBitmap(message, textBitmap, messagePixelWidth);
        initialized = true;
    }
    drawBitmap(textBitmap, messagePixelWidth, scrollOffset, CRGB::Red);
    scrollOffset++;
    if (scrollOffset > messagePixelWidth) {
        scrollOffset = -MATRIX_WIDTH;
    }
    delay(60);
}