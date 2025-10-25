#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
#define GOLD 0xFEA0

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Geometry for the static design
const int xTimer = 5;
const int yTimer = 40;
const int widthTimer = 230;
const int heightTimer = 90;

void setup() {
  Serial.begin(9600);
  tft.begin();

  // Clear screen and draw header
  tft.setRotation(0);
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(GOLD);
  tft.setTextSize(2);
  tft.setCursor(30, 160);
  tft.print("Project Chronos");

  delay(1000);

  // Main header
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(2);
  tft.setCursor(5, 10);
  tft.print("Project Chronos");
  tft.setCursor(220, 10);
  tft.setTextSize(2);
  tft.print("+");

  // Draw the timer 'div' (static design only)
  tft.drawRect(xTimer, yTimer, widthTimer, heightTimer, ILI9341_BLACK);
  tft.fillRect(xTimer + 1, yTimer + 1, widthTimer - 2, heightTimer - 2, ILI9341_WHITE);
  tft.setTextSize(1);
  tft.setCursor(xTimer + 8, yTimer + 6);
  tft.print("Timers");
  tft.drawFastHLine(xTimer + 4, yTimer + 20, widthTimer - 8, ILI9341_BLACK);
}

void loop() {
  // Intentionally empty: logic removed per request. Design remains.
}