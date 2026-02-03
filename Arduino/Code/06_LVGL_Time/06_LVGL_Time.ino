/**
 * 06_LVGL_Time - LVGL Clock with WiFi NTP Sync
 * 
 * Hardware: ESP32-S3 + ST7789 TFT (240x280)
 * 
 * This example demonstrates:
 *   - LVGL graphics library usage
 *   - Arc widget for seconds indicator
 *   - WiFi + NTP time synchronization
 *   - Modern watch face design
 * 
 * Before use: Edit WiFi settings in pin_config.h
 * 
 * Libraries: Arduino_GFX_Library, lvgl (8.x)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <lvgl.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "HWCDC.h"

HWCDC USBSerial;

// Display driver
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[LCD_WIDTH * 20];

// UI elements
static lv_obj_t *time_label;
static lv_obj_t *date_label;
static lv_obj_t *weekday_label;
static lv_obj_t *second_arc;
static lv_obj_t *status_label;

bool timeSync = false;

// LVGL display flush callback
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
  lv_disp_flush_ready(disp);
}

/**
 * Create watch face UI with arcs and labels
 */
void createWatchFace() {
  // Dark background
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x0d1117), 0);
  
  // Outer decorative arc
  lv_obj_t *outer_arc = lv_arc_create(lv_scr_act());
  lv_obj_set_size(outer_arc, 220, 220);
  lv_obj_align(outer_arc, LV_ALIGN_CENTER, 0, -10);
  lv_arc_set_rotation(outer_arc, 0);
  lv_arc_set_bg_angles(outer_arc, 0, 360);
  lv_arc_set_value(outer_arc, 100);
  lv_obj_remove_style(outer_arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(outer_arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_color(outer_arc, lv_color_hex(0x21262d), LV_PART_MAIN);
  lv_obj_set_style_arc_color(outer_arc, lv_color_hex(0x21262d), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(outer_arc, 3, LV_PART_MAIN);
  lv_obj_set_style_arc_width(outer_arc, 3, LV_PART_INDICATOR);
  
  // Seconds arc (animated)
  second_arc = lv_arc_create(lv_scr_act());
  lv_obj_set_size(second_arc, 200, 200);
  lv_obj_align(second_arc, LV_ALIGN_CENTER, 0, -10);
  lv_arc_set_rotation(second_arc, 270);
  lv_arc_set_bg_angles(second_arc, 0, 360);
  lv_arc_set_range(second_arc, 0, 60);
  lv_arc_set_value(second_arc, 0);
  lv_obj_remove_style(second_arc, NULL, LV_PART_KNOB);
  lv_obj_clear_flag(second_arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_color(second_arc, lv_color_hex(0x161b22), LV_PART_MAIN);
  lv_obj_set_style_arc_color(second_arc, lv_color_hex(0x58a6ff), LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(second_arc, 8, LV_PART_MAIN);
  lv_obj_set_style_arc_width(second_arc, 8, LV_PART_INDICATOR);
  
  // Time label (HH:MM)
  time_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), 0);
  lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -25);
  lv_label_set_text(time_label, "--:--");
  
  // Date label
  date_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(date_label, &lv_font_montserrat_16, 0);
  lv_obj_set_style_text_color(date_label, lv_color_hex(0x8b949e), 0);
  lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 25);
  lv_label_set_text(date_label, "----/--/--");
  
  // Weekday label
  weekday_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(weekday_label, &lv_font_montserrat_14, 0);
  lv_obj_set_style_text_color(weekday_label, lv_color_hex(0x7ee787), 0);
  lv_obj_align(weekday_label, LV_ALIGN_CENTER, 0, 50);
  lv_label_set_text(weekday_label, "---");
  
  // Status label (bottom)
  status_label = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_font(status_label, &lv_font_montserrat_12, 0);
  lv_obj_set_style_text_color(status_label, lv_color_hex(0x484f58), 0);
  lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -10);
  lv_label_set_text(status_label, "Connecting...");
}

/**
 * Update time display with current time
 */
void updateTimeDisplay(struct tm *timeinfo) {
  // Update time
  lv_label_set_text_fmt(time_label, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
  
  // Update seconds arc
  lv_arc_set_value(second_arc, timeinfo->tm_sec);
  
  // Update date
  lv_label_set_text_fmt(date_label, "%04d/%02d/%02d", 
    timeinfo->tm_year + 1900, 
    timeinfo->tm_mon + 1, 
    timeinfo->tm_mday);
  
  // Update weekday
  const char* weekdays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
  lv_label_set_text(weekday_label, weekdays[timeinfo->tm_wday]);
}

// Timer callback - updates time every 500ms
void updateTimeCallback(lv_timer_t *timer) {
  if (timeSync) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      updateTimeDisplay(&timeinfo);
    }
  }
}

void setup() {
  USBSerial.begin(115200);
  USBSerial.println("LVGL Time started!");
  
  // Backlight on
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  
  // Init display
  gfx->begin();
  gfx->fillScreen(0x0000);
  
  // Init LVGL
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
  createWatchFace();
  
  // Connect WiFi
  lv_label_set_text(status_label, "Connecting WiFi...");
  lv_timer_handler();
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 30) {
    delay(500);
    lv_timer_handler();
    retry++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    USBSerial.println("WiFi Connected!");
    lv_label_set_text(status_label, "Syncing time...");
    lv_timer_handler();
    
    // Configure NTP
    configTime(GMT_OFFSET, DST_OFFSET, NTP_SERVER);
    
    struct tm timeinfo;
    retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 10) {
      delay(500);
      lv_timer_handler();
      retry++;
    }
    
    if (getLocalTime(&timeinfo)) {
      timeSync = true;
      lv_label_set_text(status_label, "NTP Synced");
      USBSerial.println("Time synced!");
    } else {
      lv_label_set_text(status_label, "Sync failed");
    }
  } else {
    lv_label_set_text(status_label, "WiFi failed");
  }
  
  // Create timer for time updates
  lv_timer_create(updateTimeCallback, 500, NULL);
}

void loop() {
  lv_timer_handler();
  delay(5);
}
