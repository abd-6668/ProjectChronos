#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <bmp.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
#define GOLD 0xFEA0

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const int buttonEsc = A0, arrowUp = A1, arrowDown = A2, arrowRight= A3, arrowLeft = A4, buttonAdd = A5, buttonOk = 2, buttonPause = 3, buttonResume = 12;
// Esc black - arrows red - Add green - Ok white - Pause yellow - Resume blue - Start Grey - Remove red 
bool running = false;
int lastButtonOkState = HIGH, lastButtonAddState = HIGH , lastArrowUpState = HIGH, lastArrowDownState = HIGH, lastArrowRightState = HIGH, lastArrowLeftState = HIGH, lastButtonEscState = HIGH;
unsigned long startTime = 0;
unsigned long previosSecond = 0;
const int xTimer = 5, yTimer = 50;
const int widthTimer = 230, heightTimer = 80;
const int buzzerPin = 4;
const int bluePin = 5, greenPin = 6, redPin = 7;
int timers = 0, lightUp = 0, selectedTimer = -1, selectedField = 0;
const int maxTimers = 3;
int charWidth, charHeight;
int milliseconds = 0, seconds = 0, minutes = 0, hours = 0;
int menuField = 0, selectedOption = -1;
int page = 0;
bool menuNeedsRedraw = false;

void textMiddle (int Width, int Height, int Color, char* string, int screenColor, int size, int delayer);
void mainPage ();

void setup() {
  Serial.begin(9600);

  pinMode(buttonOk, INPUT_PULLUP);
  pinMode(buttonAdd, INPUT_PULLUP);
  pinMode(buttonEsc, INPUT_PULLUP);
  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonResume, INPUT_PULLUP);
  pinMode(arrowUp, INPUT_PULLUP);
  pinMode(arrowLeft, INPUT_PULLUP);
  pinMode(arrowRight, INPUT_PULLUP);
  pinMode(arrowDown, INPUT_PULLUP);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);

  tft.begin();

  textMiddle(12, 16, GOLD, "Project Chronos", ILI9341_WHITE, 2, 2000); // title
  textMiddle(12, 16, ILI9341_BLACK, "Loading...", ILI9341_WHITE, 2, 1000); // loading

  mainPage();
}

void loop() {
  int stateButtonOk = digitalRead(buttonOk);
  int stateButtonAdd = digitalRead(buttonAdd);
  int stateArrowUp = digitalRead(arrowUp);
  int stateArrowDown = digitalRead(arrowDown);
  int stateArrowRight = digitalRead(arrowRight);
  int stateArrowLeft = digitalRead(arrowLeft);
  int stateButtonEsc = digitalRead(buttonEsc);

  if (stateButtonAdd == LOW && lastButtonAddState == HIGH && page == 0) {
    if (timers < maxTimers) {
      timers++;
      if (timers == 1) {
        tft.fillRect(0, 100, 240, 200, ILI9341_WHITE);
      }
      
      int index = timers;
      int yTimer = 50 + (index - 1) * 85;
      tft.drawRect(xTimer, yTimer, widthTimer, heightTimer, ILI9341_BLACK);
      tft.fillRect(xTimer + 1, yTimer + 1, widthTimer - 2, heightTimer - 2, ILI9341_WHITE);
      tft.setTextSize(1);
      tft.setCursor(xTimer + 8, yTimer + 6);
      tft.print("Timer:");
      tft.setCursor(xTimer + 48, yTimer + 6);
      tft.print(timers);
      tft.drawFastHLine(xTimer + 4, yTimer + 20, widthTimer - 8, ILI9341_BLACK);
      tft.setTextSize(1.75);
      tft.setCursor(xTimer + 8, yTimer + 32);
      tft.print("Timer: 00:00:00");
      tft.setCursor(xTimer + 8, yTimer + 52);
      tft.print("Stopwatch: 00:00:00");
    }
    delay(300);
  }
  lastButtonAddState  = stateButtonAdd;

  if (timers > 0 && page == 0) {
    if (stateArrowUp == LOW && lastArrowUpState == HIGH) {
      selectedTimer--;
      if (selectedTimer < 0) selectedTimer = timers - 1;
    }
    if (stateArrowDown == LOW && lastArrowDownState == HIGH) {
      selectedTimer++;
      if (selectedTimer >= timers) selectedTimer = 0;
    }

    for (int i = 0; i < timers; i++) {
      int yBox = 50 + i * 85;
      if (i == selectedTimer) {
        tft.drawRect(xTimer, yBox, widthTimer, heightTimer, GOLD);
      } else {
        tft.drawRect(xTimer, yBox, widthTimer, heightTimer, ILI9341_BLACK);
      }
    }
  }
  lastArrowDownState = stateArrowDown;
  lastArrowUpState = stateArrowUp;

  if (stateButtonEsc == LOW && lastButtonEscState == HIGH) { //menu
    if (page == 0) { 
    page = 1;
    menuField = 0;
    menuNeedsRedraw = true;
    tft.fillScreen(ILI9341_WHITE);
    tft.setRotation(0);
    delay(50);
    tft.drawRect(20, 40, 200, 260, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
  } 
  else if (page == 1) {
    mainPage();
  }
  delay(250);
}
if (page == 1) {

  if (stateArrowUp == LOW && lastArrowUpState == HIGH) {
    menuField--;
    if (menuField < 0) menuField = 3;
    menuNeedsRedraw = true;
  }
  if (stateArrowDown == LOW && lastArrowDownState == HIGH) {
    menuField++;
    if (menuField > 3) menuField = 0;
    menuNeedsRedraw = true;
  }

  if (menuNeedsRedraw) {
    const char* options[4] = {"Settings", "Help", "About Us", "Close [Esc]"};
    int yPositions[4] = {70, 125, 190, 255};

    tft.fillRect(22, 42, 196, 256, ILI9341_WHITE);
    tft.setTextSize(2);
    for (int i = 0; i < 4; i++) {
      if (i == menuField) tft.setTextColor(GOLD);
      else tft.setTextColor(ILI9341_BLACK);

      int textWidth = strlen(options[i]) * 12;
      int x = (240 - textWidth) / 2;
      tft.setCursor(x, yPositions[i]);
      tft.print(options[i]);
    }
    menuNeedsRedraw = false;
  }

  if (stateButtonOk == LOW && lastButtonOkState == HIGH) {
    selectedOption = menuField;
    if (selectedOption == 0) {
      tft.fillScreen(ILI9341_WHITE);
      tft.setCursor(60, 150);
      tft.print("Settings Page");
    } else if (selectedOption == 1) {
      tft.fillScreen(ILI9341_WHITE);
      tft.setCursor(80, 150);
      tft.print("Help Page");
    } else if (selectedOption == 2) {
      tft.fillScreen(ILI9341_WHITE);
      tft.setCursor(60, 150);
      tft.print("About Us Page");
    } else if (selectedOption == 3) {
      mainPage();
    }
  }

  lastArrowUpState = stateArrowUp;
  lastArrowDownState = stateArrowDown;
  lastButtonOkState = stateButtonOk;
}
  lastButtonEscState = stateButtonEsc;


 if (stateButtonOk == LOW && lastButtonOkState == HIGH && page == 0) { // stopwatch
    running = !running;
    if (running) {
      startTime = millis() - (previosSecond * 1000);
      lightUp = 0;
    }
  }
  lastButtonOkState = stateButtonOk;

  if (running) {
    unsigned long elapsed = millis() - startTime;
    unsigned long totalSeconds = elapsed / 1000;
    unsigned int seconds = totalSeconds % 60;
    unsigned int minutes = (totalSeconds / 60) % 60;
    unsigned int hours = (totalSeconds / 3600) % 24;

    if (totalSeconds != previosSecond) {
      previosSecond = totalSeconds;

      tft.fillRect(20, 70, 200, 20, ILI9341_WHITE);

      tft.setCursor(20, 70);
      tft.setTextSize(1);
      tft.setTextColor(ILI9341_BLACK);
      tft.print("Stopwatch: ");
      if (hours < 10) tft.print('0');
      tft.print(hours);
      tft.print(':');
      if (minutes < 10) tft.print('0');
      tft.print(minutes);
      tft.print(':');
      if (seconds < 10) tft.print('0');
      tft.print(seconds);

      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(bluePin, HIGH);
    }
  } else {
    if (lightUp < 3) {
      digitalWrite(redPin, LOW);
      digitalWrite(buzzerPin, HIGH);
      delay(300);
      digitalWrite(buzzerPin, LOW);
      lightUp++;
    }
  }

if (running) {
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, HIGH);
} else if (page == 1) {
  digitalWrite(redPin, HIGH);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, LOW);
} else {
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, HIGH);
  digitalWrite(bluePin, HIGH);
}
} 

void textMiddle(int Width, int Height, int Color, char* string, int screenColor, int size, int delayer) {
  charWidth = Width;
  charHeight = Height;
  int lineWidthTitle = strlen(string) * charWidth;
  int xTitle = (240 - lineWidthTitle) / 2;
  int yTitle = (320 - charHeight) / 2;
  tft.setCursor(xTitle, yTitle);
  tft.setRotation(0);
  tft.fillScreen(screenColor);
  tft.setTextColor(Color);
  tft.setTextSize(2);
  tft.print(string);
  delay(delayer);
}

void mainPage() {
  page = 0;
  tft.fillScreen(ILI9341_WHITE);

  tft.setTextColor(GOLD);
  tft.setTextSize(2);
  tft.setCursor(5, 10);
  tft.print("Project Chronos");
  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(0.75);
  tft.drawRect(200, 6, 25, 25, ILI9341_BLACK);
  tft.setCursor(204, 13.5);
  tft.print("ESC");
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
    for (int i = 0; i < timers; i++) {
      int yTimer = 50 + i * 85;
      tft.drawRect(xTimer, yTimer, widthTimer, heightTimer, ILI9341_BLACK);
      tft.fillRect(xTimer + 1, yTimer + 1, widthTimer - 2, heightTimer - 2, ILI9341_WHITE);
      tft.setTextSize(1);
      tft.setCursor(xTimer + 8, yTimer + 6);
      tft.print("Timer:");
      tft.setCursor(xTimer + 48, yTimer + 6);
      tft.print(i + 1);
      tft.drawFastHLine(xTimer + 4, yTimer + 20, widthTimer - 8, ILI9341_BLACK);
      tft.setTextSize(1.75);
      tft.setCursor(xTimer + 8, yTimer + 32);
      tft.print("Timer: 00:00:00");
      tft.setCursor(xTimer + 8, yTimer + 52);
      tft.print("Stopwatch: 00:00:00");
    }
  }
}
