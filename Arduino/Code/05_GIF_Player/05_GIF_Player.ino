/**
 * 05_GIF_Player - Animated GIF Player with WiFi Upload
 * 
 * Hardware: ESP32-S3 + ST7789 TFT (240x280)
 * 
 * This example demonstrates:
 *   - WiFi AP mode (creates hotspot)
 *   - Web server for file upload
 *   - GIF decoding and playback
 *   - FFat filesystem
 * 
 * Usage:
 *   1. Upload this sketch
 *   2. Connect phone to WiFi: GIF_Player (password: 12345678)
 *   3. Open browser: 192.168.4.1
 *   4. Upload GIF file, plays automatically
 * 
 * Libraries: Arduino_GFX_Library, AnimatedGIF
 * Partition: app3M_fat9M_16MB (requires FFat)
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <FFat.h>
#include <AnimatedGIF.h>
#include "Arduino_GFX_Library.h"
#include "pin_config.h"
#include "HWCDC.h"

HWCDC USBSerial;

// ==================== Configuration ====================
#define AP_SSID     "GIF_Player"
#define AP_PASSWORD "12345678"
#define GIF_FILENAME "/demo.gif"

// ==================== Hardware ====================
Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);
Arduino_GFX *gfx = new Arduino_ST7789(bus, LCD_RST, 0, true, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);

// ==================== Global Variables ====================
AnimatedGIF gif;
File gifFile;
WebServer server(80);

bool hasGif = false;
bool isUploading = false;
File uploadFile;

// ==================== GIF Decoder Callbacks ====================

void *GIFOpenFile(const char *fname, int32_t *pSize) {
  gifFile = FFat.open(fname, "r");
  if (gifFile) {
    *pSize = gifFile.size();
    return (void *)&gifFile;
  }
  return NULL;
}

void GIFCloseFile(void *pHandle) {
  File *f = static_cast<File *>(pHandle);
  if (f) f->close();
}

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen) {
  int32_t iBytesRead = iLen;
  File *f = static_cast<File *>(pFile->fHandle);
  if ((pFile->iSize - pFile->iPos) < iLen) {
    iBytesRead = pFile->iSize - pFile->iPos - 1;
  }
  if (iBytesRead <= 0) return 0;
  iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
  pFile->iPos = f->position();
  return iBytesRead;
}

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition) {
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  return pFile->iPos;
}

void GIFDraw(GIFDRAW *pDraw) {
  uint8_t *s;
  uint16_t *usPalette, usTemp[320];
  int x, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth + pDraw->iX > LCD_WIDTH) {
    iWidth = LCD_WIDTH - pDraw->iX;
  }
  
  usPalette = pDraw->pPalette;
  int y = pDraw->iY + pDraw->y;
  
  if (y >= LCD_HEIGHT || pDraw->iX >= LCD_WIDTH || iWidth < 1) return;
  
  s = pDraw->pPixels;
  
  if (pDraw->ucDisposalMethod == 2) {
    for (x = 0; x < iWidth; x++) {
      if (s[x] == pDraw->ucTransparent) {
        s[x] = pDraw->ucBackground;
      }
    }
    pDraw->ucHasTransparency = 0;
  }

  if (pDraw->ucHasTransparency) {
    uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
    int iCount;
    pEnd = s + iWidth;
    x = 0;
    iCount = 0;
    
    while (x < iWidth) {
      c = ucTransparent - 1;
      uint16_t *d = usTemp;
      while (c != ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent) {
          s--;
        } else {
          *d++ = usPalette[c];
          iCount++;
        }
      }
      if (iCount) {
        gfx->draw16bitBeRGBBitmap(pDraw->iX + x, y, usTemp, iCount, 1);
        x += iCount;
        iCount = 0;
      }
      c = ucTransparent;
      while (c == ucTransparent && s < pEnd) {
        c = *s++;
        if (c == ucTransparent) iCount++;
        else s--;
      }
      if (iCount) {
        x += iCount;
        iCount = 0;
      }
    }
  } else {
    s = pDraw->pPixels;
    for (x = 0; x < iWidth; x++) {
      usTemp[x] = usPalette[*s++];
    }
    gfx->draw16bitBeRGBBitmap(pDraw->iX, y, usTemp, iWidth, 1);
  }
}

// ==================== Web Server Handlers ====================

void handleRoot() {
  String html = F("<!DOCTYPE html><html><head>");
  html += F("<meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<title>GIF Upload</title><style>");
  html += F("body{font-family:Arial;text-align:center;padding:20px;background:#1a1a2e;color:#fff}");
  html += F("h1{color:#00d4ff}.box{background:#16213e;padding:30px;border-radius:10px;margin:20px auto;max-width:400px}");
  html += F("button{background:#00d4ff;color:#000;padding:15px 30px;border:none;border-radius:5px;font-size:16px;cursor:pointer}");
  html += F("button:hover{background:#00a8cc}button:disabled{background:#555;cursor:not-allowed}");
  html += F(".info{color:#888;font-size:14px;margin-top:20px}");
  html += F(".pbox{display:none;margin:20px 0}.pbar{background:#333;border-radius:10px;height:20px;overflow:hidden}");
  html += F(".pfill{background:#00d4ff;height:100%;width:0%;transition:width 0.2s}.ptxt{margin-top:10px;color:#00d4ff}");
  html += F("</style></head><body><h1>GIF Player</h1><div class='box'>");
  html += F("<h3>Upload GIF File</h3><input type='file' id='f' accept='.gif'><br>");
  html += F("<div class='pbox' id='pb'><div class='pbar'><div class='pfill' id='pf'></div></div>");
  html += F("<div class='ptxt' id='pt'>0%</div></div>");
  html += F("<button id='btn' onclick='up()'>Upload</button></div>");
  html += F("<div class='info'>Screen: 240x280<br>Recommended: 240x280 or smaller</div>");
  html += F("<script>");
  html += F("function up(){var f=document.getElementById('f').files[0];if(!f){alert('Select file');return;}");
  html += F("var b=document.getElementById('btn'),pb=document.getElementById('pb'),pf=document.getElementById('pf'),pt=document.getElementById('pt');");
  html += F("b.disabled=true;pb.style.display='block';var x=new XMLHttpRequest(),d=new FormData();d.append('file',f);");
  html += F("x.upload.onprogress=function(e){if(e.lengthComputable){var p=Math.round(e.loaded/e.total*100);");
  html += F("pf.style.width=p+'%';pt.textContent=p+'% ('+Math.round(e.loaded/1024)+'/'+Math.round(e.total/1024)+' KB)';}};");
  html += F("x.onload=function(){if(x.status==200)document.body.innerHTML=x.responseText;else{alert('Failed');b.disabled=false;}};");
  html += F("x.onerror=function(){alert('Error');b.disabled=false;};x.open('POST','/upload',true);x.send(d);}");
  html += F("</script></body></html>");
  server.send(200, "text/html", html);
}

void handleUpload() {
  HTTPUpload& upload = server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    isUploading = true;
    gfx->fillScreen(BLACK);
    gfx->setTextColor(CYAN);
    gfx->setTextSize(2);
    gfx->setCursor(40, 120);
    gfx->println("Uploading...");
    
    if (FFat.exists(GIF_FILENAME)) {
      FFat.remove(GIF_FILENAME);
    }
    uploadFile = FFat.open(GIF_FILENAME, "w");
    USBSerial.printf("Upload start: %s\n", upload.filename.c_str());
  } 
  else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  } 
  else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
      USBSerial.printf("Upload done: %d bytes\n", upload.totalSize);
      hasGif = true;
    }
    isUploading = false;
  }
}

void handleUploadComplete() {
  String html = F("<!DOCTYPE html><html><body style='background:#1a1a2e;color:#fff;text-align:center;padding:50px;font-family:Arial'>");
  html += F("<h1 style='color:#00d4ff'>Upload Success!</h1><p>GIF will play now.</p>");
  html += F("<a href='/' style='color:#00d4ff'>Upload Another</a></body></html>");
  server.send(200, "text/html", html);
}

// ==================== Display Functions ====================

void showWaitingScreen() {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(CYAN);
  gfx->setTextSize(2);
  gfx->setCursor(50, 50);
  gfx->println("GIF Player");
  
  gfx->setTextColor(WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 100);
  gfx->println("1. Connect WiFi:");
  
  gfx->setTextColor(YELLOW);
  gfx->setTextSize(2);
  gfx->setCursor(20, 120);
  gfx->println(AP_SSID);
  
  gfx->setTextColor(WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 150);
  gfx->print("   Password: ");
  gfx->println(AP_PASSWORD);
  
  gfx->setCursor(20, 180);
  gfx->println("2. Open browser:");
  
  gfx->setTextColor(GREEN);
  gfx->setTextSize(2);
  gfx->setCursor(20, 200);
  gfx->println("192.168.4.1");
  
  gfx->setTextColor(WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(20, 240);
  gfx->println("3. Upload your GIF file");
}

// ==================== Main Program ====================

void setup() {
  USBSerial.begin(115200);
  USBSerial.println("GIF Player started!");
  
  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);
  
  gfx->begin();
  gfx->fillScreen(BLACK);
  
  if (!FFat.begin(true)) {
    USBSerial.println("FFat mount failed!");
    gfx->setTextColor(RED);
    gfx->setTextSize(2);
    gfx->setCursor(20, 100);
    gfx->println("FFat Error!");
    while (1) delay(1000);
  }
  
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(20, 100);
  gfx->println("Starting AP...");
  
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  USBSerial.print("AP IP: ");
  USBSerial.println(WiFi.softAPIP());
  
  server.on("/", handleRoot);
  server.on("/upload", HTTP_POST, handleUploadComplete, handleUpload);
  server.begin();
  
  gif.begin(BIG_ENDIAN_PIXELS);
  
  hasGif = FFat.exists(GIF_FILENAME);
  
  showWaitingScreen();
  
  if (hasGif) {
    delay(1000);
  }
}

void loop() {
  server.handleClient();
  
  if (isUploading) return;
  
  if (hasGif) {
    if (gif.open(GIF_FILENAME, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw)) {
      gfx->fillScreen(BLACK);
      while (gif.playFrame(true, NULL)) {
        server.handleClient();
      }
      gif.close();
    } else {
      hasGif = false;
      showWaitingScreen();
    }
  }
  
  delay(10);
}
