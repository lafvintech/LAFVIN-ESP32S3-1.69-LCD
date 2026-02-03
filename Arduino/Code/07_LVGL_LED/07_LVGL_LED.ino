/**
 * 07_LVGL_LED - LVGL Interface + WS2812 RGB LED Control
 * 
 * Hardware: ESP32-S3 + ST7789 (240x280) + WS2812
 * 
 * Features: 
 *   - Single click GPIO0: Change to random color
 *   - Double click GPIO0: Toggle LED on/off
 *   - LVGL interface shows current LED status and color
 * 
 * Dependencies: 
 *   - Arduino_GFX_Library
 *   - lvgl (8.x)
 *   - Adafruit_NeoPixel
 */

#include <Arduino.h>
#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "HWCDC.h"
#include <Adafruit_NeoPixel.h>

HWCDC USBSerial;

// Display driver
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// WS2812 RGB LED
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[LCD_WIDTH * 20];

// UI elements
static lv_obj_t *led_indicator;
static lv_obj_t *status_label;
static lv_obj_t *color_label;
static lv_obj_t *hint_label;

// LED state
bool ledOn = true;
uint8_t currentR = 255, currentG = 0, currentB = 0;

// Button detection variables
unsigned long lastClickTime = 0;
int clickCount = 0;
bool btnPressed = false;
unsigned long btnPressTime = 0;


// LVGL display flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp);
}

// Update physical LED
void updateLED() {
  if (ledOn) {
    strip.setPixelColor(0, strip.Color(currentR, currentG, currentB));
  } else {
    strip.setPixelColor(0, 0);
  }
  strip.show();
}

// Set random color
void setRandomColor() {
  currentR = random(256);
  currentG = random(256);
  currentB = random(256);
  updateLED();
  updateUI();
  USBSerial.printf("Color: R=%d G=%d B=%d\n", currentR, currentG, currentB);
}

// Toggle LED on/off
void toggleLED() {
  ledOn = !ledOn;
  updateLED();
  updateUI();
  USBSerial.printf("LED: %s\n", ledOn ? "ON" : "OFF");
}

// Update UI display
void updateUI() {
  // Update LED indicator color
  if (ledOn) {
    lv_obj_set_style_bg_color(led_indicator, lv_color_make(currentR, currentG, currentB), 0);
    lv_obj_set_style_shadow_color(led_indicator, lv_color_make(currentR, currentG, currentB), 0);
    lv_obj_set_style_shadow_opa(led_indicator, LV_OPA_70, 0);
  } else {
    lv_obj_set_style_bg_color(led_indicator, lv_color_hex(0x333333), 0);
    lv_obj_set_style_shadow_opa(led_indicator, LV_OPA_0, 0);
  }
  
  // Update status text
  lv_label_set_text(status_label, ledOn ? "LED ON" : "LED OFF");
  lv_obj_set_style_text_color(status_label, ledOn ? lv_color_hex(0x7ee787) : lv_color_hex(0xf85149), 0);
  
  // Update color value display
  lv_label_set_text_fmt(color_label, "#%02X%02X%02X", currentR, currentG, currentB);
}

// Create UI interface
void createUI() {
  // Dark background
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x0d1117), 0);
  
  // Title
  lv_obj_t *title = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);
  lv_label_set_text(title, "RGB LED Control");
  
  // LED indicator (large circle)
  led_indicator = lv_obj_create(lv_scr_act());
  lv_obj_set_size(led_indicator, 120, 120);
  lv_obj_align(led_indicator, LV_ALIGN_CENTER, 0, -20);
  lv_obj_set_style_radius(led_indicator, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(led_indicator, lv_color_make(currentR, currentG, currentB), 0);
  lv_obj_set_style_border_width(led_indicator, 3, 0);
  lv_obj_set_style_border_color(led_indicator, lv_color_hex(0x30363d), 0);
  lv_obj_set_style_shadow_width(led_indicator, 40, 0);
  lv_obj_set_style_shadow_spread(led_indicator, 10, 0);
  lv_obj_set_style_shadow_color(led_indicator, lv_color_make(currentR, currentG, currentB), 0);
  lv_obj_set_style_shadow_opa(led_indicator, LV_OPA_70, 0);
  
  // Status label
  status_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_24, 0);
  lv_obj_set_style_text_color(status_label, lv_color_hex(0x7ee787), 0);
  lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 70);
  lv_label_set_text(status_label, "LED ON");
  
  // Color value label
  color_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(color_label, &lv_font_montserrat_18, 0);
  lv_obj_set_style_text_color(color_label, lv_color_hex(0x8b949e), 0);
  lv_obj_align(color_label, LV_ALIGN_CENTER, 0, 100);
  lv_label_set_text_fmt(color_label, "#%02X%02X%02X", currentR, currentG, currentB);
  
  // Operation hint
  hint_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(hint_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(hint_label, lv_color_hex(0x484f58), 0);
  lv_obj_align(hint_label, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_label_set_text(hint_label, "Click: Color | Double: ON/OFF");
}


// Button handler with debounce and click detection
void handleButton() {
  bool currentState = (digitalRead(BTN_PIN) == LOW);
  unsigned long now = millis();
  
  // Press detection
  if (currentState && !btnPressed) {
    btnPressed = true;
    btnPressTime = now;
  }
  
  // Release detection
  if (!currentState && btnPressed) {
    btnPressed = false;
    unsigned long pressDuration = now - btnPressTime;
    
    // Valid click (debounce 50ms, exclude long press 500ms)
    if (pressDuration > 50 && pressDuration < 500) {
      clickCount++;
      lastClickTime = now;
    }
  }
  
  // Determine click type (300ms timeout)
  if (clickCount > 0 && (now - lastClickTime > 300)) {
    if (clickCount == 1) {
      // Single click - random color
      setRandomColor();
    } else if (clickCount >= 2) {
      // Double click - toggle LED
      toggleLED();
    }
    clickCount = 0;
  }
}

void setup() {
  USBSerial.begin(115200);
  USBSerial.println("LVGL LED Control started!");
  
  // Button init
  pinMode(BTN_PIN, INPUT_PULLUP);
  
  // LED init
  strip.begin();
  strip.setBrightness(50);
  updateLED();
  
  // Backlight
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  
  // Display init
  gfx->begin();
  gfx->fillScreen(0x0000);
  
  // LVGL init
  lv_init();
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, LCD_WIDTH * 20);
  
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LCD_WIDTH;
  disp_drv.ver_res = LCD_HEIGHT;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);
  
  // Create UI
  createUI();
  
  randomSeed(analogRead(0));
}

void loop() {
  handleButton();
  lv_timer_handler();
  delay(5);
}
