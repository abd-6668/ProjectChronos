#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

const int GOLD = 0xFEA0; //gold color
//pins for lcd
const int TFT_CS = 10, TFT_DC = 9, TFT_RST = 8;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
//pins for buttons
const int buttonEsc = A0; //pins for buttons
const int arrowUp = A1;
const int arrowDown = A2;
const int arrowRight = A4;
const int arrowLeft = A3;
const int buttonAdd = A5;
const int buttonOk = 2;
const int buttonRemove = 3;
const int buttonStart = 12;
//pins for rgb and buzzer
const int buzzerPin = 4, bluePin = 5, greenPin = 6, redPin = 7;

//coordinates for timer design
const int xTimer = 5, widthTimer = 230, heightTimer = 80, maxTimers = 3;
//timer and page handling variables
int timers = 0, selectedTimer = -1, menuField = 0, lastMenuField = -1;
int selectedOption = -1, page = 0;
bool menuNeedsRedraw = false;
int lightUp = 0;
int settingsField = 0, lastSettingsField = -1;
bool settingsNeedsRedraw = false;
unsigned long lastTriggerTime = 0;
const unsigned long TRIGGER_RED_DURATION = 1200;
//invert color variables
bool inverted = 0;
bool gold = 1;
//flicker handling variables
int lastButtonOkState = HIGH;
int lastButtonAddState = HIGH;
int lastArrowUpState = HIGH;
int lastArrowDownState = HIGH;
int lastArrowRightState = HIGH;
int lastArrowLeftState = HIGH;
int lastButtonEscState = HIGH;
int lastRemoveState = HIGH;
int lastStartState = HIGH;
//middle text variables
int charWidth = 6, charHeight = 8;
//object for timer functionality
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
void drawSettingsMenu();
void deleteTimer(int index);
void (* resetFunc) (void) = 0;
void navtimer(int minusOrPlus);

void setup() {
  Serial.begin(9600);

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
  tft.invertDisplay(inverted);

  // intro
  textMiddle(12, 16, GOLD, "Project Chronos", ILI9341_WHITE, 2, 1500);
  textMiddle(12, 16, ILI9341_BLACK, "Version 1.13", ILI9341_WHITE, 2, 700);

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
    if (page == 0 && timers < maxTimers) {
      int idx = timers++;
      for (int d = 0; d < 6; ++d) timerArr[idx].digits[d] = 0;
      timerArr[idx].remainingMillis = 0;
      timerArr[idx].running = false;
      timerArr[idx].editing = false;
      timerArr[idx].editDigit = 0;
      timerArr[idx].lastUpdateMillis = millis();
      timerArr[idx].stopwatchElapsedMillis = 0;
      timerArr[idx].triggered = false;
      timerArr[idx].lastSelected = false;
      timerArr[idx].lastEditing = false;
      timerArr[idx].lastRunning = false;
      timerArr[idx].lastTriggered = false;
      timerArr[idx].lastRemainingSec = 0xFFFFFFFFUL;
      timerArr[idx].lastStopwatchSec = 0xFFFFFFFFUL;
      timerArr[idx].lastEditDigit = -1;

      if (selectedTimer < 0) selectedTimer = 0;
      
      if (timers == 1) {
        tft.fillRect(0, 45, 240, 275, ILI9341_WHITE);
      }

      if (selectedTimer >= 0 && timerArr[selectedTimer].editing) {
        delay(75);
        lastButtonAddState = stateButtonAdd;
        return;
    }

      drawTimer(idx);
    }
    delay(75);
  }
  lastButtonAddState = stateButtonAdd;

  if (stateButtonOk == LOW && lastButtonOkState == HIGH) { //menu
    if (page == 1) { //menu selection
      selectedOption = menuField;
      if (selectedOption == 0) {
        page = 2;
        settingsField = 0;
        lastSettingsField = -1;
        settingsNeedsRedraw = true;
  } else if (selectedOption == 1) {
    page = 3;
    drawSubPage("Help", "");
  } else if (selectedOption == 2) {
    page = 4;
    drawSubPage("About Us", "");
      } else if (selectedOption == 3) {
        mainPage();
      }
    }  else if (page == 2) { // settings menu selection
      switch(settingsField) {
        case 0:
          page = 10;
          drawSubPage("Contact Us", "");
          break;
        case 1:
          page = 11;
          drawSubPage("Light Mode", "");
          break;
        case 2:
          page = 12;
          drawSubPage("History", "  Soon...");
          break;
        case 3:
          page = 13;
          drawSubPage("Current Time", "  Soon...");
          break;
        case 4:
          page = 14;
          drawSubPage("Help", "");
          break;
        case 5:
          page = 15;
          drawSubPage("About Us", "");
          break;
        case 6:
          page = 16;
          drawSubPage("Restart", "");
          break;
      }
      delay(75);
    } else if (page == 11) {
      if (gold == 1) {
        inverted = 1;
        gold = 0;
      } else {
        inverted = 0;
        gold = 1;
      }
      tft.invertDisplay(inverted);
    } else if (page == 16) {
      resetFunc();
    } else if (page == 0) {
      if (selectedTimer >= 0 && selectedTimer < timers) { //enter edit mode for timer
        timerArr[selectedTimer].editing = true;
        timerArr[selectedTimer].editDigit = 0;
        timerArr[selectedTimer].running = false;
        timerArr[selectedTimer].lastRemainingSec = 0xFFFFFFFFUL;
        timerArr[selectedTimer].lastStopwatchSec = 0xFFFFFFFFUL;
        timerArr[selectedTimer].lastEditDigit = -99;
      }
    }
    delay(75);
  }

if (stateArrowLeft == LOW && lastArrowLeftState == HIGH) { 
  if (page == 0 && selectedTimer >= 0 && selectedTimer < timers && timerArr[selectedTimer].editing) {
    timerArr[selectedTimer].editDigit--;
    if (timerArr[selectedTimer].editDigit < 0) timerArr[selectedTimer].editDigit = 5;
  } else if (page == 0 && timers > 0) {
      navtimer(-1);
  }
  delay(75);
}

if (stateArrowRight == LOW && lastArrowRightState == HIGH) {
  if (page == 0 && selectedTimer >= 0 && selectedTimer < timers && timerArr[selectedTimer].editing) {
    timerArr[selectedTimer].editDigit++;
    if (timerArr[selectedTimer].editDigit > 5) timerArr[selectedTimer].editDigit = 0;
  } else if (page == 0 && timers > 0) {
      navtimer(+1);
  }
  delay(75);
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
    } else if (page == 0 && timers > 0) {
      navtimer(-1);
    } else if (page == 1) {
      menuField--;
      if (menuField < 0) menuField = 3;
      menuNeedsRedraw = true;
    } else if (page == 2) {
      settingsField--;
      if (settingsField < 0) settingsField = 6;
      settingsNeedsRedraw = true;
    }
    delay(75);
  }
  if (stateArrowDown == LOW && lastArrowDownState == HIGH) {
    if (page == 0 && selectedTimer >= 0 && selectedTimer < timers && timerArr[selectedTimer].editing) {
      int di = timerArr[selectedTimer].editDigit;
      uint8_t maxv = 9;
      if (di == 2 || di == 4) maxv = 5;
      if (timerArr[selectedTimer].digits[di] == 0) timerArr[selectedTimer].digits[di] = maxv;
      else timerArr[selectedTimer].digits[di]--;
    } else if (page == 0 && timers > 0) {
      navtimer(+1);
    } else if (page == 1) {
      menuField++;
      if (menuField > 3) menuField = 0;
      menuNeedsRedraw = true;
    } else if (page == 2) {
      settingsField++;
      if (settingsField > 6) settingsField = 0;
      settingsNeedsRedraw = true;
    }
    delay(75);
  }
  lastArrowDownState = stateArrowDown;
  lastArrowUpState = stateArrowUp;

    if (stateRemove == LOW && lastRemoveState == HIGH) {
    if (page == 0 && selectedTimer >= 0 && selectedTimer < timers) {
        if (timerArr[selectedTimer].editing) {
            delay(150);
            lastRemoveState = stateRemove;
            return;
        }
        deleteTimer(selectedTimer);
    }
    delay(75);
}
  lastRemoveState = stateRemove;

if (stateStart == LOW && lastStartState == HIGH) { //start-pause-resume
    if (selectedTimer >= 0 && selectedTimer < timers) {
        TimerData &T = timerArr[selectedTimer];
        if (!T.editing) {
            T.running = !T.running;
            T.lastUpdateMillis = millis();
            if (T.remainingMillis == 0 && T.running && T.stopwatchElapsedMillis == 0) {
                T.triggered = false;
            }
        }
    }
    delay(75);
}
lastStartState = stateStart;

  if (page == 0) {
  for (int i=0; i<timers; ++i) { //update timers
    TimerData &T = timerArr[i];
    if (T.running) {
      unsigned long elapsed = now - T.lastUpdateMillis;
      if (elapsed > 0) {
        if (T.remainingMillis > 0 && !T.triggered) {
          if (elapsed >= T.remainingMillis) {
            T.remainingMillis = 0;
            T.running = false;
            T.triggered = true;
            tone(buzzerPin, 1200, TRIGGER_RED_DURATION);
            lastTriggerTime = millis();
          } else {
            T.remainingMillis -= elapsed;
          }
        } else {
          T.stopwatchElapsedMillis += elapsed;
        }
        T.lastUpdateMillis = now;
      }
    } else {
      T.lastUpdateMillis = now;
    }
  }
}

  if (page == 0) { // timer UI
  for (int i=0; i<timers; ++i) {
    drawTimer(i);
  }

  if (timers > 0) { //selected timer indicator
    for (int i=0; i<timers; ++i) {
      int yBox = 50 + i * 85;
      if (i == selectedTimer) tft.drawRect(xTimer, yBox, widthTimer, heightTimer, GOLD);
      else tft.drawRect(xTimer, yBox, widthTimer, heightTimer, ILI9341_BLACK);
    }
  }
}

    if (stateButtonEsc == LOW && lastButtonEscState == HIGH) {
  delay(75);
  
  if (page == 0) { // Main page
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
    T.running = false;
    T.lastRemainingSec = 0xFFFFFFFFUL;
    T.lastStopwatchSec = 0xFFFFFFFFUL;
} else { // Open main menu
      page = 1;
      menuField = 0;
      lastMenuField = -1;
      menuNeedsRedraw = true;
      drawMenu();
    }
  } else if (page == 1) { // page handle
    mainPage();
    for (int t = 0; t < timers; t++) {
      timerArr[t].lastRunning   = !timerArr[t].running;
      timerArr[t].lastTriggered = !timerArr[t].triggered;
      timerArr[t].lastSelected  = false;
      timerArr[t].lastRemainingSec = 0xFFFFFFFFUL;
      timerArr[t].lastStopwatchSec = 0xFFFFFFFFUL;
      drawTimer(t);
    }
    lastMenuField = -1;
  } else if (page == 2) {
    page = 1;
    menuField = 0;
    lastMenuField = -1;
    menuNeedsRedraw = true;
    drawMenu();
  } else if (page >= 3 && page <= 9) {
    page = 1;
    menuField = 0;
    lastMenuField = -1;
    menuNeedsRedraw = true;
    drawMenu();
  } else {
    page = 2;
    settingsField = 0;
    lastSettingsField = -1;
    settingsNeedsRedraw = true;
    drawSettingsMenu();
  }
}
  lastButtonEscState = stateButtonEsc;

  if (page == 1 && menuNeedsRedraw) { //menu handle
    drawMenu();
    menuNeedsRedraw = false;
  }

if (page == 1 && menuNeedsRedraw) {
  drawMenu();
  menuNeedsRedraw = false;
} else if (page == 2 && settingsNeedsRedraw) {
  drawSettingsMenu();
  settingsNeedsRedraw = false;
}

if (page == 0) { //rgb light functionality
    bool anyTriggered = false;
    bool anyRunning = false;
    
    for (int i = 0; i < timers; i++) {
        if (timerArr[i].running) anyRunning = true;
        if (timerArr[i].triggered) anyTriggered = true;
    }
    bool recentTrigger = (millis() - lastTriggerTime < TRIGGER_RED_DURATION);
    
    if (recentTrigger) {
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, HIGH);
        digitalWrite(bluePin, HIGH);
    } else if (anyRunning) {
        digitalWrite(redPin, HIGH);
        digitalWrite(greenPin, LOW);
        digitalWrite(bluePin, HIGH);
    } else if (anyTriggered) {
        digitalWrite(redPin, LOW);
        digitalWrite(greenPin, HIGH);
        digitalWrite(bluePin, HIGH);
    } else {
        digitalWrite(redPin, HIGH);
        digitalWrite(greenPin, HIGH);
        digitalWrite(bluePin, LOW);
    }
} else if (page == 1) {
    digitalWrite(bluePin, LOW);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
}

  lastButtonOkState = stateButtonOk; //update buttons
  lastButtonAddState = stateButtonAdd;
  lastArrowUpState = stateArrowUp;
  lastArrowDownState = stateArrowDown;
  lastArrowRightState = stateArrowRight;
  lastArrowLeftState = stateArrowLeft;
  lastRemoveState = stateRemove;
  lastStartState = stateStart;
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

  bool isFirstDraw = (T.lastRemainingSec == 0xFFFFFFFFUL && T.lastStopwatchSec == 0xFFFFFFFFUL);

  if (T.lastSelected != sel) {
    tft.drawRect(x, yBox, w, h, sel ? GOLD : ILI9341_BLACK);
    T.lastSelected = sel;
  }

  if (isFirstDraw) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    tft.setCursor(x + 8, yBox + 6);
    tft.print("Timer:");
    tft.setCursor(x + 48, yBox + 6);
    tft.print(i + 1);
    tft.drawFastHLine(x + 4, yBox + 20, w - 8, ILI9341_BLACK);
  }

  if (T.lastEditing != T.editing) {
    tft.fillRect(x + 6, yBox + 26, w - 12, h - 30, ILI9341_WHITE);
    
    tft.fillRect(x + 148, yBox + 2, 60, 14, ILI9341_WHITE);
    tft.setCursor(x + 150, yBox + 6);
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK); 
    tft.print(T.editing ? "PAUSED" : "PAUSED");

    T.lastRemainingSec = 0xFFFFFFFFUL;
    T.lastStopwatchSec = 0xFFFFFFFFUL;
    T.lastEditDigit = -99;
    T.lastEditing = T.editing;
  }

  if (!isFirstDraw && (T.lastRunning != T.running || T.lastTriggered != T.triggered)) {
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

  if (T.editing) {
    char curDigits[10];
    sprintf(curDigits, "%01d%01d:%01d%01d:%01d%01d", T.digits[0], T.digits[1], T.digits[2], T.digits[3], T.digits[4], T.digits[5]);

    static char prevDigits[maxTimers][9] = {{0}};
    bool digitsChanged = (T.lastEditDigit != T.editDigit) || (strcmp(prevDigits[i], curDigits) != 0);

    if (digitsChanged) {
      strncpy(prevDigits[i], curDigits, sizeof(prevDigits[i]) - 1);
      prevDigits[i][sizeof(prevDigits[i]) - 1] = 0;
      
      tft.fillRect(x + 6, yBox + 26, w - 12, 24, ILI9341_WHITE);
      
      int baseX = x + 8;
      int baseY = yBox + 32;
      tft.setTextSize(1);
      int cx = baseX;
      const int step = 12;

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
}

void deleteTimer(int index) {
  if (index < 0 || index >= timers) return;
  if (timerArr[index].running) {
    timerArr[index].running = false;
  }

  int yDeleted = 50 + index * 85;
  tft.fillRect(xTimer, yDeleted, widthTimer, heightTimer, ILI9341_WHITE);

  if (timers == 1) {
    timers = 0;
    selectedTimer = -1;
    tft.fillRect(0, 45, 240, 275, ILI9341_WHITE);

    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    const char* line1 = "No timers to be displayed,";
    const char* line2 = "please add a timer by pressing 'Add'";
    int line1Width = strlen(line1) * 6;
    int line2Width = strlen(line2) * 6;
    int x1 = (240 - line1Width) / 2;
    int x2 = (240 - line2Width) / 2;
    int y = (320 - (8 * 3)) / 2;
    tft.setCursor(x1, y);
    tft.print(line1);
    tft.setCursor(x2, y + 8 * 2);
    tft.print(line2);
    return;
  }

  for (int i = index; i < timers - 1; i++) {
    timerArr[i] = timerArr[i + 1];
    timerArr[i].lastSelected = false;
    timerArr[i].lastEditing = false;
    timerArr[i].lastRunning = false;
    timerArr[i].lastTriggered = false;
    timerArr[i].lastRemainingSec = 0xFFFFFFFFUL;
    timerArr[i].lastStopwatchSec = 0xFFFFFFFFUL;
    timerArr[i].lastEditDigit = -99;
    drawTimer(i);
  }

  int lastTimerY = 50 + (timers - 1) * 85;
  tft.fillRect(xTimer, lastTimerY, widthTimer, heightTimer, ILI9341_WHITE);

  timers--;
  if (index >= timers) index = timers - 1;
  selectedTimer = index;
  
  for (int t = 0; t < timers; t++) {
    bool shouldBeSelected = (t == selectedTimer);
    if (timerArr[t].lastSelected != shouldBeSelected) {
      timerArr[t].lastSelected = false;
      drawTimer(t);
    }
  }
}


void drawMenu() { //menu design
  const char* options[4] = {"Settings", "Help", "About Us", "Close [ESC]"};
  int yPositions[4] = {80, 150, 220, 290};
  
  if (lastMenuField == -1) {
    tft.fillScreen(ILI9341_WHITE);
    tft.fillRect(0, 0, 240, 40, GOLD);
    tft.setTextWrap(false);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor(96, 10);
    tft.print("Menu");
    
    tft.drawLine(0, 40, 240, 40, GOLD);

    for (int i = 0; i < 4; i++) {
      int y = yPositions[i];
      tft.fillRect(20, y - 10, 200, 36, ILI9341_WHITE);
      if (i > 0) {
        tft.drawFastHLine(25, y - 12, 190, ILI9341_LIGHTGREY);
      }

      if (i == menuField) {
        tft.setTextSize(2);
        tft.setTextColor(GOLD);
      } else {
        tft.setTextSize(2);
        tft.setTextColor(ILI9341_BLACK);
      }
      int tw = strlen(options[i]) * 12;
      int x = (240 - tw) / 2;
      tft.setCursor(x, y);
      tft.print(options[i]);
    }
  }
  else if (menuField != lastMenuField) {
    int oldY = yPositions[lastMenuField];
    tft.fillRect(20, oldY - 12, 200, 38, ILI9341_WHITE);
    if (lastMenuField > 0) tft.drawFastHLine(25, oldY - 12, 190, ILI9341_LIGHTGREY);
    tft.setTextSize(2);
    tft.setTextColor(ILI9341_BLACK);
    int tw = strlen(options[lastMenuField]) * 12;
    int x = (240 - tw) / 2;
    tft.setCursor(x, oldY);
    tft.print(options[lastMenuField]);
    int newY = yPositions[menuField];
    tft.fillRect(20, newY - 12, 200, 38, ILI9341_WHITE);
    if (menuField > 0) tft.drawFastHLine(25, newY - 12, 190, ILI9341_LIGHTGREY);
    tft.setTextSize(2);
    tft.setTextColor(GOLD);
    tw = strlen(options[menuField]) * 12;
    x = (240 - tw) / 2;
    tft.setCursor(x, newY);
    tft.print(options[menuField]);
  }
  
  lastMenuField = menuField;
}


void drawSubPage(const char* title, const char* content) {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextWrap(true);

  tft.fillRect(0, 0, 240, 40, GOLD);
  tft.setTextSize(2);
  tft.setTextColor(ILI9341_WHITE);
  int titleWidth = strlen(title) * 12;
  int titleX = (240 - titleWidth) / 2;
  tft.setCursor(titleX, 10);
  tft.print(title);

  tft.setTextSize(1);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(0, 42);

  if (strcmp(title, "Help") == 0) {
    tft.print(F(" QUICK START:\n\n"));
    tft.print(F(" ADD TIMER:\n\n  Press '+' (Max 3)\n\n\n"));
    tft.print(F(" EDIT MODE:\n\n  Arrows->Select->OK\n\n  Set digits->ESC saves\n\n\n"));
    tft.print(F(" RUN/PAUSE:\n\n  Select->START button\n\n\n"));
    tft.print(F(" RGB LIGHTS:\n\n  Green=Run\n\n  Blue=Menu\n\n  Red=Done\n\n\n"));
    tft.print(F(" SUPPORT:\n\n  help.projectchronos@gmail.com"));
  } 
  else if (strcmp(title, "About Us") == 0) {
    tft.print(F(" CHRONOS V1.13\n\n"));
    tft.print(F(" By University Antonine\n\n\n"));
    tft.print(F(" TEAM (4):\n\n  Abdallah (Lead Software)\n  Roni (Software and Hardware)\n  Jawad (Lead Hardware)\n  Adam (Testing)\n\n\n"));
    tft.print(F(" FEATURES:\n\n  3 timers, RGB LEDs\n  Audio alerts, Stopwatch\n  Edit mode, and Menu.\n  A settings with different features.\n\n\n"));
    tft.print(F(" Last Update: 16/11/2025\n\n  -Optimization\n  -Fixed huge delay\n  Version: 1.13\n\n\n"));
    tft.print(F(" Thank you for using Project Chronos"));
  }
  else if (strcmp(title, "Contact Us") == 0) {
    tft.print(F(" REPORT ISSUES:\n\n\n"));
    tft.print(F("  -Timer bugs/errors\n\n"));
    tft.print(F("  -Hardware problems\n\n"));
    tft.print(F("  -Feature requests\n\n"));
    tft.print(F("  -General questions\n\n\n"));
    tft.print(F(" Email: help.projectchronos@gmail.com"));
  } else if (strcmp(title, "Light Mode") == 0) {
    tft.print(F(" Light Mode Change: \n\n"));
    tft.print(F(" Press OK (1-2) to: \n\n\n"));
    tft.print(F("  1. Change color to midnight blue.\n\n"));
    tft.print(F("  2. Change color to gold.\n\n"));
    tft.print(F("  Press ESC to leave."));
  }
  else {
    tft.print(content);
  }
  
  tft.setTextSize(2);
  if (strcmp(title, "Restart") == 0) {
    tft.print(F(" Restart: \n\n\n"));
    tft.print(F(" Press OK to restart\n\n"));
    tft.print(F(" Press ESC to cancel"));
  } else {
    tft.print(content);
  }

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

void drawSettingsMenu() {
  const char* options[7] = {"Contact Us", "Light Mode", "History", "Current Time", "Help", "About Us", "Restart"};
  
  if (lastSettingsField == -1) {
    tft.fillScreen(ILI9341_WHITE);

    tft.fillRect(0, 0, 240, 40, GOLD);
    tft.setTextWrap(false);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.setCursor((240 - 110) / 2, 10);
    tft.print("Settings");
    
    for (int i = 0; i < 7; i++) {
      int y = 50 + i * 34;
      
      if (i > 0) {
        tft.drawFastHLine(25, y - 3, 190, ILI9341_LIGHTGREY);
      }
      
      if (i == settingsField) {
        tft.setTextSize(2);
        tft.setTextColor(GOLD);
        int tw = strlen(options[i]) * 12;
        int x = (240 - tw) / 2;
        tft.setCursor(x, y);
        tft.print(options[i]);
      } else {
        tft.setTextSize(1);
        tft.setTextColor(ILI9341_BLACK);
        int tw = strlen(options[i]) * 6;
        int x = (240 - tw) / 2;
        tft.setCursor(x, y);
        tft.print(options[i]);
      }
    }
    
    // Back button
    tft.drawLine(0, 290, 240, 290, GOLD);
    tft.setTextSize(2);
    tft.setTextColor(GOLD);
    const char* back = "Back [Esc]";
    int backWidth = strlen(back) * 11;
    int backX = (240 - backWidth) / 2;
    tft.setCursor(backX, 300);
    tft.print(back);
  }
  else if (settingsField != lastSettingsField) {
    int oldY = 50 + lastSettingsField * 34;
    tft.fillRect(20, oldY - 5, 200, 34, ILI9341_WHITE);
    if (lastSettingsField > 0) tft.drawFastHLine(25, oldY - 3, 190, ILI9341_LIGHTGREY);
    
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    int tw = strlen(options[lastSettingsField]) * 6;
    int x = (240 - tw) / 2;
    tft.setCursor(x, oldY);
    tft.print(options[lastSettingsField]);
    
    int newY = 50 + settingsField * 34;
    tft.fillRect(20, newY - 5, 200, 34, ILI9341_WHITE);
    if (settingsField > 0) tft.drawFastHLine(25, newY - 3, 190, ILI9341_LIGHTGREY);
    
    tft.setTextSize(2);
    tft.setTextColor(GOLD);
    tw = strlen(options[settingsField]) * 12;
    x = (240 - tw) / 2;
    tft.setCursor(x, newY);
    tft.print(options[settingsField]);
  }
  
  lastSettingsField = settingsField;
}

void navtimer(int minusOrPlus) {
  if (timers == 0) return;
  int oldSelected = selectedTimer;
  selectedTimer += minusOrPlus;
  if (selectedTimer >= timers) selectedTimer = 0;
  if (selectedTimer < 0) selectedTimer = timers - 1;
  if (oldSelected >= 0) {
    timerArr[oldSelected].lastSelected = false;
    drawTimer(oldSelected);
  }
  timerArr[selectedTimer].lastSelected = false;
  drawTimer(selectedTimer);
}

void mainPage() { //main page
  page = 0;
  tft.fillScreen(ILI9341_WHITE);

  tft.fillRect(0,0,320,40,GOLD);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.print("Project Chronos");

  tft.setTextColor(ILI9341_BLACK);
  tft.setTextSize(1);
  tft.drawRect(200, 6, 25, 25, ILI9341_BLACK);
  tft.setCursor(204, 14);
  tft.print("ESC");
  tft.drawFastHLine(0, 40, 240, ILI9341_BLACK);

  if (timers == 0) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_BLACK);
    const char* line1 = "No timers to be displayed,";
    const char* line2 = "please add a timer by pressing 'Add'";
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
      tft.setTextSize(1);
      tft.setCursor(xTimer + 8, yTimer + 32);
      tft.print("Timer: 00:00:00");
      tft.setCursor(xTimer + 8, yTimer + 52);
      tft.print("Stopwatch: 00:00:00");
    }
  }
}