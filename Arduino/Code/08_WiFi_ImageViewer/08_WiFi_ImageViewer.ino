/**
 * 08_WiFi_ImageViewer - WiFi Image Upload Display
 * 
 * Hardware: ESP32-S3 + ST7789 (240x280)
 * 
 * Features: 
 *   - Creates WiFi AP hotspot
 *   - Web page for image upload (supports JPG/PNG)
 *   - Browser-side auto resize to screen dimensions
 *   - JPEG decode and center display
 * 
 * Usage:
 *   1. Upload to ESP32-S3
 *   2. Connect to WiFi: Image_Viewer (password: 12345678)
 *   3. Open browser: 192.168.4.1
 *   4. Select image, auto resize and upload
 * 
 * Dependencies: 
 *   - Arduino_GFX_Library
 *   - JPEGDEC
 * 
 * Partition: app3M_fat9M_16MB (requires FFat)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FFat.h>
#include <JPEGDEC.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "HWCDC.h"

HWCDC USBSerial;

// ==================== Configuration ====================
#define AP_SSID     "Image_Viewer"
#define AP_PASSWORD "12345678"
#define IMG_FILENAME "/image.jpg"

// ==================== Hardware Init ====================
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// ==================== Global Variables ====================
JPEGDEC jpeg;
File jpegFile;
WebServer server(80);

bool hasImage = false;
bool isUploading = false;
bool needRefresh = false;
File uploadFile;

// ==================== JPEG Decode Callbacks ====================
void *jpegOpen(const char *filename, int32_t *size) {
  jpegFile = FFat.open(filename, "r");
  if (jpegFile) {
    *size = jpegFile.size();
    return &jpegFile;
  }
  return NULL;
}

void jpegClose(void *handle) {
  if (jpegFile) jpegFile.close();
}

int32_t jpegRead(JPEGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!jpegFile) return 0;
  return jpegFile.read(buffer, length);
}

int32_t jpegSeek(JPEGFILE *handle, int32_t position) {
  if (!jpegFile) return 0;
  return jpegFile.seek(position);
}

// Draw callback - renders decoded pixels to display
int jpegDraw(JPEGDRAW *pDraw) {
  gfx->draw16bitBeRGBBitmap(pDraw->x, pDraw->y, pDraw->pPixels, pDraw->iWidth, pDraw->iHeight);
  return 1;
}

// ==================== Web Server ====================
void handleRoot() {
  // Serve upload page with client-side image resize
  String html = F("<!DOCTYPE html><html><head>");
  html += F("<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<title>Image Upload</title><style>");
  html += F("body{font-family:Arial;text-align:center;padding:20px;background:#0d1117;color:#fff}");
  html += F("h1{color:#58a6ff}.box{background:#161b22;padding:30px;border-radius:10px;margin:20px auto;max-width:400px;border:1px solid #30363d}");
  html += F("button{background:#238636;color:#fff;padding:15px 30px;border:none;border-radius:6px;font-size:16px;cursor:pointer;margin:10px}");
  html += F("button:hover{background:#2ea043}button:disabled{background:#555;cursor:not-allowed}");
  html += F("#preview{max-width:240px;margin:10px auto;border:2px solid #30363d;border-radius:4px;display:none}");
  html += F("#status{color:#f0883e;margin:10px 0}.info{color:#8b949e;font-size:14px;margin-top:20px}");
  html += F("</style></head><body><h1>Image Viewer</h1><div class='box'>");
  html += F("<h3>Upload Image</h3><input type='file' id='f' accept='image/*'><br>");
  html += F("<img id='preview'><canvas id='c' style='display:none'></canvas>");
  html += F("<div id='status'></div><button id='btn' disabled>Upload</button></div>");
  html += F("<div class='info'>Auto resize to 240x280<br>Supports JPG, PNG</div>");
  
  // JavaScript for client-side image resize before upload
  html += F("<script>");
  html += F("var b=null,W=240,H=280;");
  html += F("document.getElementById('f').onchange=function(e){");
  html += F("var f=e.target.files[0];if(!f)return;");
  html += F("document.getElementById('status').textContent='Processing...';");
  html += F("var i=new Image();i.onload=function(){");
  // Calculate scale to fit screen while maintaining aspect ratio
  html += F("var w=i.width,h=i.height,s=Math.min(W/w,H/h,1);");
  html += F("w=Math.round(w*s);h=Math.round(h*s);");
  // Draw resized image to canvas
  html += F("var c=document.getElementById('c');c.width=w;c.height=h;");
  html += F("c.getContext('2d').drawImage(i,0,0,w,h);");
  // Convert to JPEG blob
  html += F("c.toBlob(function(x){b=x;");
  html += F("document.getElementById('preview').src=URL.createObjectURL(x);");
  html += F("document.getElementById('preview').style.display='block';");
  html += F("document.getElementById('btn').disabled=false;");
  html += F("document.getElementById('status').textContent='Ready: '+w+'x'+h+' ('+(x.size/1024|0)+'KB)';");
  html += F("},'image/jpeg',0.9);};i.src=URL.createObjectURL(f);};");
  
  // Upload handler
  html += F("document.getElementById('btn').onclick=function(){");
  html += F("if(!b)return;var s=document.getElementById('status'),t=this;");
  html += F("s.textContent='Uploading...';t.disabled=true;");
  html += F("var d=new FormData();d.append('file',b,'image.jpg');");
  html += F("fetch('/upload',{method:'POST',body:d}).then(function(r){return r.text();})");
  html += F(".then(function(h){document.body.innerHTML=h;})");
  html += F(".catch(function(e){s.textContent='Error: '+e;t.disabled=false;});};");
  html += F("</script></body></html>");
  server.send(200, "text/html", html);
}

// Handle file upload in chunks
void handleUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    isUploading = true;
    // Show uploading status on screen
    gfx->fillScreen(BLACK);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(2);
    gfx->setCursor(50, 130);
    gfx->println("Uploading...");
    
    // Remove old image if exists
    if (FFat.exists(IMG_FILENAME)) {
      FFat.remove(IMG_FILENAME);
    }
    uploadFile = FFat.open(IMG_FILENAME, "w");
    USBSerial.printf("Upload start: %s\n", upload.filename.c_str());
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) {
    // Write chunk to file
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } 
  else if (upload.status == UPLOAD_FILE_END) {
    // Close file and trigger display
    if (uploadFile) {
      uploadFile.close();
      USBSerial.printf("Upload done: %d bytes\n", upload.totalSize);
      hasImage = true;
      needRefresh = true;
    }
    isUploading = false;
  }
}

void handleUploadComplete() {
  String html = F("<!DOCTYPE html><html><body style='background:#0d1117;color:#fff;text-align:center;padding:50px;font-family:Arial'>");
  html += F("<h1 style='color:#58a6ff'>Upload Success!</h1>");
  html += F("<p>Image will display now.</p>");
  html += F("<a href='/' style='color:#58a6ff'>Upload Another</a></body></html>");
  server.send(200, "text/html", html);
}

// ==================== Display Functions ====================
void showWaitingScreen() {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(CYAN);
  gfx->setTextSize(2);
  gfx->setCursor(30, 40);
  gfx->println("Image Viewer");
  
  gfx->setTextColor(WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 90);
  gfx->println("1. Connect WiFi:");
  
  gfx->setTextColor(YELLOW);
  gfx->setTextSize(2);
  gfx->setCursor(20, 110);
  gfx->println(AP_SSID);
  
  gfx->setTextColor(WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 140);
  gfx->print("   Password: ");
  gfx->println(AP_PASSWORD);
  
  gfx->setCursor(20, 170);
  gfx->println("2. Open browser:");
  
  gfx->setTextColor(GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(20, 190);
  gfx->println("192.168.4.1");
  
  gfx->setTextColor(WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 230);
  gfx->println("3. Upload image");
}

void showImage() {
  if (!FFat.exists(IMG_FILENAME)) {
    USBSerial.println("Image file not found");
    showWaitingScreen();
    return;
  }
  
  gfx->fillScreen(BLACK);
  
  if (jpeg.open(IMG_FILENAME, jpegOpen, jpegClose, jpegRead, jpegSeek, jpegDraw)) {
    int w = jpeg.getWidth();
    int h = jpeg.getHeight();
    USBSerial.printf("Image: %dx%d\n", w, h);
    
    // Set pixel format for correct colors
    jpeg.setPixelType(RGB565_BIG_ENDIAN);
    
    // Center image on screen
    int x = (LCD_WIDTH - w) / 2;
    int y = (LCD_HEIGHT - h) / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    
    jpeg.decode(x, y, 0);
    jpeg.close();
    
    USBSerial.println("Image displayed");
  } else {
    USBSerial.println("Failed to decode JPEG");
    gfx->setTextColor(RED);
    gfx->setTextSize(2);
    gfx->setCursor(30, 130);
    gfx->println("Decode Error!");
  }
}

// ==================== Main Program ====================
void setup() {
  USBSerial.begin(115200);
  USBSerial.println("WiFi Image Viewer started!");
  
  // Backlight on
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  
  // Display init
  gfx->begin();
  gfx->fillScreen(BLACK);
  
  // FFat filesystem init
  if (!FFat.begin(true)) {
    USBSerial.println("FFat mount failed!");
    gfx->setTextColor(RED);
    gfx->setTextSize(2);
    gfx->setCursor(20, 100);
    gfx->println("FFat Error!");
    while (1) delay(1000);
  }
  
  // Start WiFi AP
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(20, 100);
  gfx->println("Starting AP...");
  
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  USBSerial.print("AP IP: ");
  USBSerial.println(WiFi.softAPIP());
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, handleUploadComplete, handleUpload);
  server.begin();
  
  // Check for existing image
  hasImage = FFat.exists(IMG_FILENAME);
  
  if (hasImage) {
    needRefresh = true;
  } else {
    showWaitingScreen();
  }
}

void loop() {
  server.handleClient();
  
  // Display image after upload completes
  if (needRefresh && !isUploading) {
    needRefresh = false;
    showImage();
  }
  
  delay(10);
}
