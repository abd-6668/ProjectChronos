#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Pins used in diagram.json: MOSI=11, MISO=12, SCK=13
#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("TFT init");

  // Backlight control: pin 7 sinks the LCD LED through the resistor in the diagram
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW); // drive LOW to turn the backlight on (common-anode through resistor to 5V)

  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);

  tft.setCursor(10, 20);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.print("Hello Wokwi");
  Serial.println("Hello Wokwi");

  tft.setCursor(10, 50);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  tft.print("Project Chronos");
  Serial.println("Project Chronos");
}

void loop() {
  // Periodically print the same messages so they always show in the serial monitor
  static unsigned long last = 0;
  if (millis() - last > 1000) {
    last = millis();
    Serial.println("Hello Wokwi");
    Serial.println("Project Chronos");
  }
  if (Serial.available()) {
    int incomingByte = Serial.read();
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }
  delay(10);
}