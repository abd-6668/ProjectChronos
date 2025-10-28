#ifndef BMP_H
#define BMP_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Minimal bmp helper stub for ProjectChronos.
// This file provides a lightweight bmpDraw() function so sketches that
// include <bmp.h> will compile and get a visual placeholder on the TFT.
// Replace or expand this with a full BMP-from-SD implementation if needed.

static inline bool bmpDraw(const char* filename, Adafruit_ILI9341 &tft, int16_t x, int16_t y) {
  // Draw a simple placeholder box with the filename (truncated)
  const int w = 48;
  const int h = 32;
  tft.fillRect(x, y, w, h, ILI9341_WHITE);
  tft.drawRect(x, y, w, h, ILI9341_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(x + 2, y + (h/2) - 4);
  // print a short filename or "BMP"
  if (filename && filename[0]) {
    // print up to ~10 chars
    char buf[12];
    uint8_t i = 0;
    while (filename[i] && i < 10) { buf[i] = filename[i]; ++i; }
    buf[i] = '\0';
    tft.print(buf);
  } else {
    tft.print("BMP");
  }
  (void)filename; // silence unused warnings when compiled without printing
  return true;
}

#endif // BMP_H
