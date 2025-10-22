#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(100);  // Increased delay for stability
  Serial.println("TFT init");

  // Remove pin 7 control as LED is connected directly to 5V through resistor
  
  if (!tft.begin()) {
    Serial.println("Could not initialize display");
    while (1);  // Stop if display initialization fails
  }
  Serial.println("Display initialized");
  
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor(10, 20);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Hellos");
  Serial.println("Hellos");

  tft.setCursor(10, 50);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  tft.print("Projecst");
  Serial.println("Projecst");
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    Serial.println("Hellos");
    Serial.println("Projecst");
  }
  delay(10);
}