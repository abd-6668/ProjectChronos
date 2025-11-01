#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define TFT_CS   10
#define TFT_DC   9
#define TFT_RST  8
#define GOLD 0xFEA0

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

const int buttonEsc = A0;
const int arrowUp = A1;
const int arrowDown = A2;
const int arrowRight = A3;
const int arrowLeft = A4;
const int buttonAdd = A5;
const int buttonOk = 2;
const int buttonRemove = 3;
const int buttonStart = 12;

const int buzzerPin = 4, bluePin = 5, greenPin = 6, redPin = 7;

bool running = false;
unsigned long startTime = 0, previosSecond = 0;

const int xTimer = 5, widthTimer = 230, heightTimer = 80, maxTimers = 3;

int timers = 0, selectedTimer = -1, menuField = 0;
int selectedOption = -1, page = 0; // 0 = main, 1 = menu, 2 = settings, 3 = help, 4 = about
bool menuNeedsRedraw = false;
int lightUp = 0;

int lastButtonOkState = HIGH;
int lastButtonAddState = HIGH;
int lastArrowUpState = HIGH;
int lastArrowDownState = HIGH;
int lastArrowRightState = HIGH;
int lastArrowLeftState = HIGH;
int lastButtonEscState = HIGH;
int lastRemoveState = HIGH;
int lastStartState = HIGH;

int charWidth = 6, charHeight = 8;
//timer structure
struct TimerData {
  uint8_t digits[6];
  unsigned long remainingMillis;
  bool running;
  bool editing;
  int editDigit;
  unsigned long lastUpdateMillis;
  unsigned long stopwatchElapsedMillis;
  bool triggered;

  bool lastSelected;
  bool lastEditing;
  bool lastRunning;
  bool lastTriggered;
  unsigned long lastRemainingSec;
  unsigned long lastStopwatchSec;
  int lastEditDigit;
};

TimerData timerArr[maxTimers];

void mainPage();
void drawTimer(int i);
void drawMenu();
void drawSubPage(const char* title, const char* content);
void textMiddle(int Width, int Height, int Color, const char* string, int screenColor, int size, int delayer);

void setup() {
  Serial.begin(115200);

  // pins
  pinMode(buttonOk, INPUT_PULLUP);
  pinMode(buttonAdd, INPUT_PULLUP);
  pinMode(buttonEsc, INPUT_PULLUP);
  pinMode(buttonRemove, INPUT_PULLUP);
  pinMode(buttonStart, INPUT_PULLUP);
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
  digitalWrite(buzzerPin, LOW);

  tft.begin();
  tft.setTextWrap(false);

  // intro
  textMiddle(12, 16, GOLD, "Project Chronos", ILI9341_WHITE, 2, 1500);
  textMiddle(6, 8, ILI9341_BLACK, "Beta Version 1.07...", ILI9341_WHITE, 1, 700);

  // initialize timers data
  for (int i=0; i<maxTimers; ++i) {
    for (int d=0; d<6; ++d) timerArr[i].digits[d] = 0;
    timerArr[i].remainingMillis = 0;
    timerArr[i].running = false;
    timerArr[i].editing = false;
    timerArr[i].editDigit = 0;
    timerArr[i].lastUpdateMillis = millis();
    timerArr[i].stopwatchElapsedMillis = 0;
    timerArr[i].triggered = false;

    timerArr[i].lastSelected = false;
    timerArr[i].lastEditing  = false;
    timerArr[i].lastRunning  = false;
    timerArr[i].lastTriggered= false;
    timerArr[i].lastRemainingSec = 0xFFFFFFFFUL;
    timerArr[i].lastStopwatchSec = 0xFFFFFFFFUL;
    timerArr[i].lastEditDigit = -1;
  }

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
  int stateRemove = digitalRead(buttonRemove);
  int stateStart = digitalRead(buttonStart);

  unsigned long now = millis();


  if (stateButtonAdd == LOW && lastButtonAddState == HIGH) { //timer functionallity
    if (timers < maxTimers) {
      if (timers == 0) {
        tft.fillRect(0, 100, 240, 220, ILI9341_WHITE);
      }
      int idx = timers++;
      for (int d=0; d<6; ++d) timerArr[idx].digits[d] = 0;
      timerArr[idx].remainingMillis = 0;
      timerArr[idx].running = false;
      timerArr[idx].editing = false;
      timerArr[idx].editDigit = 0;
      timerArr[idx].lastUpdateMillis = now;
      timerArr[idx].stopwatchElapsedMillis = 0;
      timerArr[idx].triggered = false;

      timerArr[idx].lastSelected = false;
      timerArr[idx].lastEditing  = false;
      timerArr[idx].lastRunning  = false;
      timerArr[idx].lastTriggered= false;
      timerArr[idx].lastRemainingSec = 0xFFFFFFFFUL;
      timerArr[idx].lastStopwatchSec = 0xFFFFFFFFUL;
      timerArr[idx].lastEditDigit = -1;

      if (selectedTimer < 0) selectedTimer = 0;
    }
    delay(200);
  }
  lastButtonAddState = stateButtonAdd;

  if (stateButtonOk == LOW && lastButtonOkState == HIGH) { //menu
    if (page == 1) { //menu selection
      selectedOption = menuField;
      if (selectedOption == 0) {
        page = 2;
        drawSubPage("Settings", "Soon...");
      } else if (selectedOption == 1) {
        page = 3;
        drawSubPage("Help", "Soon...");
      } else if (selectedOption == 2) {
        page = 4;
        drawSubPage("About Us", "Made by Project Chronos");
      } else if (selectedOption == 3) {
        mainPage();
      }
    } else if (page == 0) {
      if (selectedTimer >= 0 && selectedTimer < timers) { //enter edit mode for timer
        timerArr[selectedTimer].editing = true;
        timerArr[selectedTimer].editDigit = 0;
        timerArr[selectedTimer].running = false;
        timerArr[selectedTimer].lastRemainingSec = 0xFFFFFFFFUL;
        timerArr[selectedTimer].lastStopwatchSec = 0xFFFFFFFFUL;
        timerArr[selectedTimer].lastEditDigit = -99;
      } else {
        running = !running;
        if (running) {
          startTime = millis() - (previosSecond * 1000UL);
        }
      }
    }
    delay(140);
  }

  if (stateArrowLeft == LOW && lastArrowLeftState == HIGH) { //timer handle 
    if (page == 0 && selectedTimer >= 0 && selectedTimer < timers && timerArr[selectedTimer].editing) {
      timerArr[selectedTimer].editDigit--;
      if (timerArr[selectedTimer].editDigit < 0) timerArr[selectedTimer].editDigit = 5;
    } else if (page == 0) {
      if (timers > 0) {
        selectedTimer--;
        if (selectedTimer < 0) selectedTimer = timers - 1;
      }
    } else if (page == 1) {

    }
    delay(100);
  }

  if (stateArrowRight == LOW && lastArrowRightState == HIGH) {
    if (page == 0 && selectedTimer >= 0 && selectedTimer < timers && timerArr[selectedTimer].editing) {
      timerArr[selectedTimer].editDigit++;
      if (timerArr[selectedTimer].editDigit > 5) timerArr[selectedTimer].editDigit = 0;
    } else if (page == 0) {
      if (timers > 0) {
        selectedTimer++;
        if (selectedTimer >= timers) selectedTimer = 0;
      }
    }
    delay(100);
  }
  lastArrowLeftState = stateArrowLeft;
  lastArrowRightState = stateArrowRight;

  if (stateArrowUp == LOW && lastArrowUpState == HIGH) { //page handle
    if (page == 0 && selectedTimer >= 0 && selectedTimer < timers && timerArr[selectedTimer].editing) {
      int di = timerArr[selectedTimer].editDigit;
      uint8_t maxv = 9;
      if (di == 2 || di == 4) maxv = 5;
      timerArr[selectedTimer].digits[di]++;
      if (timerArr[selectedTimer].digits[di] > maxv) timerArr[selectedTimer].digits[di] = 0;
    } else if (page == 0) {
      if (timers > 0) {
        selectedTimer--;
        if (selectedTimer < 0) selectedTimer = timers - 1;
      }
    } else if (page == 1) {
      menuField--;
      if (menuField < 0) menuField = 3;
      menuNeedsRedraw = true;
    }
    delay(100);
  }
  if (stateArrowDown == LOW && lastArrowDownState == HIGH) {
    if (page == 0 && selectedTimer >= 0 && selectedTimer < timers && timerArr[selectedTimer].editing) {
      int di = timerArr[selectedTimer].editDigit;
      uint8_t maxv = 9;
      if (di == 2 || di == 4) maxv = 5;
      if (timerArr[selectedTimer].digits[di] == 0) timerArr[selectedTimer].digits[di] = maxv;
      else timerArr[selectedTimer].digits[di]--;
    } else if (page == 0) {
      if (timers > 0) {
        selectedTimer++;
        if (selectedTimer >= timers) selectedTimer = 0;
      }
    } else if (page == 1) {
      menuField++;
      if (menuField > 3) menuField = 0;
      menuNeedsRedraw = true;
    }
    delay(100);
  }
  lastArrowDownState = stateArrowDown;
  lastArrowUpState = stateArrowUp;

if (stateStart == LOW && lastStartState == HIGH) { //pause-resume
    if (selectedTimer >= 0 && selectedTimer < timers) {
        TimerData &T = timerArr[selectedTimer];
        if (T.remainingMillis > 0 && !T.triggered) {
            T.running = !T.running;
            T.lastUpdateMillis = millis();
        }
    }
    delay(120);
}
lastStartState = stateStart;

  for (int i=0; i<timers; ++i) { //update timers
    TimerData &T = timerArr[i];
    if (T.running && !T.triggered) {
      unsigned long elapsed = now - T.lastUpdateMillis;
      if (elapsed > 0) {
        if (elapsed >= T.remainingMillis) { //finish of timer
          T.remainingMillis = 0;
          T.running = false;
          T.triggered = true;
          T.stopwatchElapsedMillis += elapsed;
          tone(buzzerPin, 1200, 600);
          digitalWrite(redPin, LOW);
          digitalWrite(greenPin, HIGH);
          digitalWrite(bluePin, HIGH);
        } else {
          T.remainingMillis -= elapsed;
          T.stopwatchElapsedMillis += elapsed;
        }
        T.lastUpdateMillis = now;
      }
    } else {
      T.lastUpdateMillis = now;
    }
    if (!T.triggered) {

    }
  }

  for (int i=0; i<timers; ++i) { //timers added UI
    drawTimer(i);
  }

  if (timers > 0 && page == 0) { //selected timer indicator
    for (int i=0; i<timers; ++i) {
      int yBox = 50 + i * 85;
      if (i == selectedTimer) tft.drawRect(xTimer, yBox, widthTimer, heightTimer, GOLD);
      else tft.drawRect(xTimer, yBox, widthTimer, heightTimer, ILI9341_BLACK);
    }
  }

  if (stateButtonEsc == LOW && lastButtonEscState == HIGH) { //edit - menu handle
    if (page == 0) { //save value of timer
      if (selectedTimer >= 0 && selectedTimer < timers && timerArr[selectedTimer].editing) {
        TimerData &T = timerArr[selectedTimer];
        uint8_t h = T.digits[0]*10 + T.digits[1];
        uint8_t m = T.digits[2]*10 + T.digits[3];
        uint8_t s = T.digits[4]*10 + T.digits[5];
        unsigned long totalSeconds = (unsigned long)h*3600UL + (unsigned long)m*60UL + (unsigned long)s;
        T.remainingMillis = totalSeconds * 1000UL;
        T.stopwatchElapsedMillis = 0;
        T.editing = false;
        T.triggered = false;
        T.lastRemainingSec = 0xFFFFFFFFUL;
        T.lastStopwatchSec = 0xFFFFFFFFUL;
      } else { //if not editing open menu
        page = 1;
        menuField = 0;
        menuNeedsRedraw = true;
        drawMenu();
      }
    } else if (page == 1) { //return to mainpage
      mainPage();
    } else if (page >= 2) { //return to menu
      page = 1;
      menuField = 0;
      menuNeedsRedraw = true;
      drawMenu();
    }
    delay(200);
  }
  lastButtonEscState = stateButtonEsc;

  if (page == 1 && menuNeedsRedraw) { //menu handle
    drawMenu();
    menuNeedsRedraw = false;
  }

  if (page == 0) { //stopwatch functionality
    if (running) {
      unsigned long elapsed = millis() - startTime;
      unsigned long totalSeconds = elapsed / 1000UL;
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
        tft.print(hours); tft.print(':');
        if (minutes < 10) tft.print('0');
        tft.print(minutes); tft.print(':');
        if (seconds < 10) tft.print('0');
        tft.print(seconds);
        digitalWrite(greenPin, LOW);
        digitalWrite(redPin, HIGH);
        digitalWrite(bluePin, HIGH);
      }
    } else { //indicate finish of timer by rgb
      if (lightUp < 3) {
        digitalWrite(redPin, LOW);
        digitalWrite(buzzerPin, HIGH);
        delay(200);
        digitalWrite(buzzerPin, LOW);
        lightUp++;
      }
    }
  }

  if (running) { //rgb light function
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
  } else if (page == 1) {
    // menu page
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
  } else {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }

//update 
  lastButtonOkState = stateButtonOk;
  lastButtonAddState = stateButtonAdd;
  lastArrowUpState = stateArrowUp;
  lastArrowDownState = stateArrowDown;
  lastArrowRightState = stateArrowRight;
  lastArrowLeftState = stateArrowLeft;
  lastRemoveState = stateRemove;
  lastStartState = stateStart;
  delay(35);
}

void drawTimer(int i) { //timer draw
  if (i < 0 || i >= timers) return;

  const int yBox = 50 + i * 85;
  const int x = xTimer;
  const int w = widthTimer;
  const int h = heightTimer;

  TimerData &T = timerArr[i];

  unsigned long remSec = T.remainingMillis / 1000UL;
  unsigned long swSec  = T.stopwatchElapsedMillis / 1000UL;
  bool sel = (i == selectedTimer);

  if (T.lastSelected != sel || T.lastEditing != T.editing) { //edit mode
    tft.fillRect(x, yBox, w, h, ILI9341_WHITE);
    tft.drawRect(x, yBox, w, h, sel ? ILI9341_LIGHTGREY : ILI9341_BLACK);

    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(x + 8, yBox + 6);
    tft.print("Timer:");
    tft.setCursor(x + 48, yBox + 6);
    tft.print(i + 1);
    tft.drawFastHLine(x + 4, yBox + 20, w - 8, ILI9341_BLACK);

    T.lastRemainingSec = 0xFFFFFFFFUL;
    T.lastStopwatchSec = 0xFFFFFFFFUL;
    T.lastRunning = !T.running;
    T.lastTriggered = !T.triggered;
    T.lastEditDigit = -99;

    T.lastSelected = sel;
    T.lastEditing = T.editing;
  }

  if (T.editing) {//edit view
    bool needDigits = (T.lastEditDigit != T.editDigit);

    char curDigits[9];
    sprintf(curDigits, "%d%d:%d%d:%d%d", T.digits[0], T.digits[1], T.digits[2], T.digits[3], T.digits[4], T.digits[5]);

    static char prevDigits[maxTimers][9] = {{0}};
    if (strcmp(prevDigits[i], curDigits) != 0) {
      needDigits = true;
      strncpy(prevDigits[i], curDigits, sizeof(prevDigits[i])-1);
      prevDigits[i][sizeof(prevDigits[i])-1] = 0;
    }

    if (needDigits) {
      tft.fillRect(x + 6, yBox + 26, w - 12, 24, ILI9341_WHITE);
      int baseX = x + 8;
      int baseY = yBox + 32;
      tft.setTextSize(1);
      const int step = 12;
      int cx = baseX;

      auto printDigit = [&](int dIndex, char ch) {
        tft.setCursor(cx, baseY);
        tft.setTextColor((T.editDigit == dIndex) ? GOLD : ILI9341_BLACK);
        tft.print(ch);
        cx += step;
      };

      printDigit(0, '0' + T.digits[0]);
      printDigit(1, '0' + T.digits[1]);
      tft.setTextColor(ILI9341_BLACK); tft.setCursor(cx, baseY); tft.print(':'); cx += 8;
      printDigit(2, '0' + T.digits[2]);
      printDigit(3, '0' + T.digits[3]);
      tft.setTextColor(ILI9341_BLACK); tft.setCursor(cx, baseY); tft.print(':'); cx += 8;
      printDigit(4, '0' + T.digits[4]);
      printDigit(5, '0' + T.digits[5]);

      T.lastEditDigit = T.editDigit;
    }

    if (swSec != T.lastStopwatchSec || T.lastEditing != true) { //stopwatch
      tft.fillRect(x + 6, yBox + 48, w - 12, 16, ILI9341_WHITE);
      unsigned int sw_s = swSec % 60;
      unsigned int sw_m = (swSec / 60) % 60;
      unsigned int sw_h = (swSec / 3600);
      char sbuf[16];
      sprintf(sbuf, "%02u:%02u:%02u", sw_h, sw_m, sw_s);
      tft.setCursor(x + 8, yBox + 52);
      tft.setTextSize(1);
      tft.setTextColor(ILI9341_BLACK);
      tft.print("Stopwatch: ");
      tft.print(sbuf);
      T.lastStopwatchSec = swSec;
    }

    return;
  }

  if (T.lastRemainingSec != remSec || T.lastTriggered != T.triggered) {
    tft.fillRect(x + 6, yBox + 26, w - 12, 24, ILI9341_WHITE);
    unsigned int s = remSec % 60;
    unsigned int m = (remSec / 60) % 60;
    unsigned int h = (remSec / 3600);
    char timeStr[16];
    sprintf(timeStr, "%02u:%02u:%02u", h, m, s);
    tft.setTextSize(1);
    tft.setCursor(x + 8, yBox + 32);
    tft.setTextColor(T.triggered ? ILI9341_RED : ILI9341_BLACK);
    tft.print("Timer: ");
    tft.print(timeStr);
    T.lastRemainingSec = remSec;
  }

  if (swSec != T.lastStopwatchSec) {
    tft.fillRect(x + 6, yBox + 48, w - 12, 16, ILI9341_WHITE);
    unsigned int sw_s = swSec % 60;
    unsigned int sw_m = (swSec / 60) % 60;
    unsigned int sw_h = (swSec / 3600);
    char sbuf[16];
    sprintf(sbuf, "%02u:%02u:%02u", sw_h, sw_m, sw_s);
    tft.setCursor(x + 8, yBox + 52);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    tft.print("Stopwatch: ");
    tft.print(sbuf);
    T.lastStopwatchSec = swSec;
  }

  if (T.lastRunning != T.running || T.lastTriggered != T.triggered) {
    tft.fillRect(x + 148, yBox + 2, 60, 14, ILI9341_WHITE);
    tft.setCursor(x + 150, yBox + 6);
    tft.setTextSize(1);
    if (T.running) {
      tft.setTextColor(ILI9341_GREEN); tft.print("RUN");
    } else {
      if (T.triggered) { tft.setTextColor(ILI9341_RED); tft.print("DONE"); }
      else              { tft.setTextColor(ILI9341_BLACK); tft.print("PAUSED"); }
    }
    T.lastRunning = T.running;
    T.lastTriggered = T.triggered;
  }
}

void drawMenu() { //menu design
  tft.fillScreen(ILI9341_WHITE);

  tft.drawRoundRect(20, 30, 200, 300, 8, ILI9341_BLACK);
  tft.setTextWrap(false);

  tft.setTextColor(GOLD);
  tft.setTextSize(3);
  int titleWidth = 4 * 18;
  int titleX = (240 - titleWidth) / 2;
  tft.setCursor(titleX, 45);
  tft.print("Menu");

  const char* options[4] = {"Settings", "Help", "About Us", "Close [Esc]"};
  int yPositions[4] = {100, 160, 220, 280};

  for (int i=0; i<4; ++i) {
    if (i == menuField) {
      tft.fillRoundRect(30, yPositions[i] - 12, 180, 42, 6, GOLD);
      tft.setTextColor(ILI9341_WHITE);
    } else {
      tft.setTextColor(ILI9341_BLACK);
    }
    tft.setTextSize(2);
    int tw = strlen(options[i]) * 11;
    int x = (240 - tw) / 2;
    tft.setCursor(x, yPositions[i]);
    tft.print(options[i]);
  }
}


void drawSubPage(const char* title, const char* content) { //subpage menu
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextWrap(true);

  tft.fillRect(0, 0, 240, 40, GOLD);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  int titleWidth = strlen(title) * 12;
  int titleX = (240 - titleWidth) / 2;
  tft.setCursor(titleX, 10);
  tft.print(title);

  tft.setTextSize(2);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(18, 80);
  tft.print(content);

  tft.drawLine(0, 290, 240, 290, GOLD);
  tft.setTextSize(2);
  tft.setTextColor(GOLD);
  const char* back = "Back [Esc]";
  int backWidth = strlen(back) * 11;
  int backX = (240 - backWidth) / 2;
  tft.setCursor(backX, 300);
  tft.print(back);
}

void textMiddle(int Width, int Height, int Color, const char* string, int screenColor, int size, int delayer) {
  charWidth = Width;
  charHeight = Height;
  int lineWidthTitle = strlen(string) * charWidth;
  int xTitle = (240 - lineWidthTitle) / 2;
  int yTitle = (320 - charHeight) / 2;
  tft.setRotation(0);
  tft.fillScreen(screenColor);
  tft.setTextColor(Color);
  tft.setTextSize(size);
  tft.setCursor(xTitle, yTitle);
  tft.print(string);
  if (delayer > 0) delay(delayer);
}

void mainPage() { //main page
  page = 0;
  tft.fillScreen(ILI9341_WHITE);

  tft.setTextColor(GOLD);
  tft.setTextSize(2);
  tft.setCursor(5, 10);
  tft.print("Project Chronos");

  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.drawRect(200, 6, 25, 25, ILI9341_BLACK);
  tft.setCursor(204, 12);
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
    for (int i = 0; i < timers; ++i) {
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
