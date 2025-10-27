#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
#define GOLD 0xFEA0

const int buttonEsc = A0,arrowUp = A1,arrowDown = A2,arrowRight= A3,arrowLeft = A4,buttonAdd = A5,buttonOk = 2,buttonPause = 3,buttonResume = 12;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

bool running = false;
int lastButtonOkState = HIGH, lastButtonAddState = HIGH;
unsigned long startTime = 0;
unsigned long previosSecond = 0;
const int xTimer = 5, yTimer = 50;
const int widthTimer = 230, heightTimer = 80;
const int buzzerPin = 4;
const int bluePin = 5, greenPin = 6, redPin = 7;
int timers = 0, lightUp = 0;
int charWidth, charHeight;
int milliseconds = 0, seconds = 0, minutes = 0, hours = 0;

void setup() {
  Serial.begin(9600);
  int stateButtonAdd = digitalRead(buttonAdd);

  pinMode(buttonOk, INPUT_PULLUP);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);

  tft.begin();

  charWidth = 12; // First Page (Title)
  charHeight = 16;
  int lineWidthTitle = strlen("Project Chronos") * charWidth;
  int xTitle = (240 - lineWidthTitle) / 2;
  int yTitle = (320 - charHeight) / 2;
  tft.setCursor(xTitle, yTitle);
  tft.setRotation(0);
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(GOLD);
  tft.setTextSize(2);
  tft.print("Project Chronos");

  delay(1000);

  tft.fillScreen(ILI9341_WHITE); // Second Page (Main Project)
  tft.setTextColor(GOLD);
  tft.setTextSize(2);
  tft.setCursor(5, 10);
  tft.print("Project Chronos");
  tft.drawFastHLine(0, 40, 240, ILI9341_BLACK);

  if (timers == 0) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    const char* line1 = "No timers to be displayed,";
    const char* line2 = "please add a timer by pressing '+'";
    charWidth = 6;
    charHeight = 8;
    int line1Width = strlen(line1) * charWidth;
    int line2Width = strlen(line2) * charWidth;
    int x1 = (240 - line1Width) / 2;
    int x2 = (240 - line2Width) / 2;
    int y = (320 - (charHeight * 3)) / 2;
    tft.setCursor(x1, y);
    tft.print(line1);
    tft.setCursor(x2, y + charHeight * 2);
    tft.print(line2);
  } else {
    tft.drawRect(xTimer, yTimer, widthTimer, heightTimer, ILI9341_BLACK);
    tft.fillRect(xTimer + 1, yTimer + 1, widthTimer - 2, heightTimer - 2, ILI9341_WHITE);
    tft.setTextSize(1);
    tft.setCursor(xTimer + 8, yTimer + 6);
    tft.print("Timers");
    tft.drawFastHLine(xTimer + 4, yTimer + 20, widthTimer - 8, ILI9341_BLACK);
  }
}

void loop() {
  int stateButtonOk = digitalRead(buttonOk);

  if (stateButtonOk == LOW && lastButtonOkState == HIGH) { //timer/stopwatch adder
    running = true;
    startTime = millis();
    lastButtonOkState = stateButtonOk;
  }
  if (running) {
    unsigned long elapse = millis() - startTime;
    unsigned long totalSeconds = (elapse / 1000), seconds = (totalSeconds) % 60, minutes = (totalSeconds / 60) % 60, hours = (totalSeconds / 3600) % 24;

    if (totalSeconds != previosSecond) {
      previosSecond = totalSeconds;
    }

  tft.setCursor(20, 70);
  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.print("Stopwatch: ");
  tft.print(hours);
  tft.print(":");
  tft.print(minutes);
  tft.print(":");
  tft.print(seconds);
  digitalWrite(greenPin, LOW);
  digitalWrite(redPin, HIGH);
 } else {
  while (lightUp <= 3) {
  digitalWrite(redPin, LOW);
  digitalWrite(buzzerPin, HIGH);
  delay(1000);
  lightUp++;
  }
 }


}

