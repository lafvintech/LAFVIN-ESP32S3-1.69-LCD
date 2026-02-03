/**
 * 02_ColorTest - Matrix Rain Effect
 * 
 * Hardware: ESP32-S3 + ST7789 TFT (240x280)
 * 
 * This example demonstrates:
 *   - Character drawing with color gradient
 *   - Animation with falling text columns
 *   - RGB565 color manipulation
 * 
 * Library: Arduino_GFX_Library
 */

#include <Arduino.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "HWCDC.h"

HWCDC USBSerial;

// Display driver
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// Matrix rain parameters
#define CHAR_SIZE   8                         // Character pixel size
#define COLUMNS     (LCD_WIDTH / CHAR_SIZE)   // Number of columns
#define ROWS        (LCD_HEIGHT / CHAR_SIZE)  // Number of rows
#define TAIL_LEN    12                        // Trail length

// State for each column
int heads[COLUMNS];           // Head position of each column
int speeds[COLUMNS];          // Fall speed of each column
char chars[COLUMNS][ROWS];    // Characters in each column

/**
 * Calculate trail color based on distance from head
 * Head is white, then fades to green, then black
 */
uint16_t getTrailColor(int distance, uint16_t baseColor) {
  if (distance == 0) return WHITE;      // Head is brightest
  if (distance >= TAIL_LEN) return BLACK;
  
  // Fade effect
  int brightness = 255 - (distance * 255 / TAIL_LEN);
  
  if (baseColor == GREEN) {
    return gfx->color565(0, brightness, 0);
  } else if (baseColor == CYAN) {
    return gfx->color565(0, brightness, brightness);
  } else if (baseColor == MAGENTA) {
    return gfx->color565(brightness, 0, brightness);
  }
  return gfx->color565(0, brightness, 0);
}

void setup() {
  USBSerial.begin(115200);
  USBSerial.println("Matrix Rain started!");
  
  // Backlight on
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  
  // Init display
  gfx->begin();
  gfx->fillScreen(BLACK);
  
  // Initialize each column with random values
  for (int col = 0; col < COLUMNS; col++) {
    heads[col] = random(-ROWS, 0);    // Random start position (above screen)
    speeds[col] = random(1, 3);       // Random speed (1-2)
    for (int row = 0; row < ROWS; row++) {
      chars[col][row] = random(33, 127);  // Random ASCII character
    }
  }
}

void loop() {
  for (int col = 0; col < COLUMNS; col++) {
    int x = col * CHAR_SIZE;
    
    // Draw the entire trail
    for (int i = 0; i < TAIL_LEN + 2; i++) {
      int row = heads[col] - i;
      if (row >= 0 && row < ROWS) {
        int y = row * CHAR_SIZE;
        uint16_t color = getTrailColor(i, GREEN);
        
        // Randomly change head character for flicker effect
        if (i == 0 && random(100) < 30) {
          chars[col][row] = random(33, 127);
        }
        
        gfx->drawChar(x, y, chars[col][row], color, BLACK);
      }
    }
    
    // Erase tail end
    int tailEnd = heads[col] - TAIL_LEN - 1;
    if (tailEnd >= 0 && tailEnd < ROWS) {
      gfx->fillRect(x, tailEnd * CHAR_SIZE, CHAR_SIZE, CHAR_SIZE, BLACK);
    }
    
    // Move head down
    heads[col] += speeds[col];
    
    // Reset to top when off screen
    if (heads[col] - TAIL_LEN > ROWS) {
      heads[col] = random(-ROWS/2, 0);
      speeds[col] = random(1, 3);
    }
  }
  
  delay(50);
}
