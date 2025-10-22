#include <Arduino.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("TFT init");

  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor(10, 20);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Hello Wokwis");
  Serial.println("Hello Wokwis");

  tft.setCursor(10, 50);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  tft.print("Project Chronoss");
  Serial.println("Project Chronoss");
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    Serial.println("Hello Wokwiw");
    Serial.println("Project Chronoss");
  }
  delay(10);
}