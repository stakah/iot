#include <Arduino.h>

#include <SoftwareSerial.h>

//Mario's Ideas
//MAX7219 - using Led Control library to display 8x8 bitmap
#include <LedControl.h>

#include <rtc_demo.h>

#include "melodies.h"
int currentMelody = 0;

// buzzer
#define BUZZER 11

// btn 0
#define DISPLAY_TIME 0
#define DISPLAY_YEAR 1
#define DISPLAY_DATE 2
#define SET_HOUR 3
#define SET_MINUTE 4
#define SET_YEAR 5
#define SET_MONTH 6
#define SET_DAY 7

#define SEND_SERIAL
#ifdef SEND_SERIAL
#define sendSerial(txt) { Serial.write(txt); }
#endif

#define DIN  8
#define CS  9
#define CLK  10
#define LED_MODULES 4
LedControl lc= LedControl(DIN, CLK, CS, LED_MODULES);
int h,m,s,ms,hs = 0;
int year, month, day = 0;

const int debug = 0; // 1-debug messages, 0-no debug messages
int btnPin[4] = {2, 3, 4, 5};
int btnState[4] = {HIGH, HIGH, HIGH, HIGH};
int currentRead[4] = {0, 0, 0, 0};
int mode[4] = {DISPLAY_TIME, 0, 0, 0};
int btnModes[4] = {8, 3, 2, 2};


unsigned long debounce[4] = {0, 0, 0, 0};
unsigned long debounceDelay = 50;
unsigned long refreshInterval = 125L; //ms
int blinkDotsN = (1000L / refreshInterval) / 2; // 4 tiks per cycle (8 tiks total)
int blinkDotsC = 0;

int reset_time = 0; // 0 = NO, 1 = YES

byte digits[10][8]={
  {B00001110, //0
   B00010001,
   B00010001,
   B00010001,
   B00010001,
   B00010001,
   B00001110,
   B00000000},
  {B00000010, //1
   B00001110,
   B00000010,
   B00000010,
   B00000010,
   B00000010,
   B00001111,
   B00000000},
  {B00001110, //2
   B00010001,
   B00000001,
   B00001110,
   B00010000,
   B00010000,
   B00011111,
   B00000000},
  {B00001110, //3
   B00010001,
   B00000001,
   B00000110,
   B00000001,
   B00010001,
   B00001110,
   B00000000},
  {B00000010, //4
   B00000110,
   B00001010,
   B00010010,
   B00011111,
   B00000010,
   B00000010,
   B00000000},
  {B00011111, //5
   B00010000,
   B00010110,
   B00011001,
   B00000001,
   B00010001,
   B00001110,
   B00000000},
  {B00001110, //6
   B00010001,
   B00010000,
   B00011110,
   B00010001,
   B00010001,
   B00001110,
   B00000000},
  {B00011111, //7
   B00010001,
   B00000010,
   B00000010,
   B00000100,
   B00000100,
   B00000100,
   B00000000},
  {B00001110, //8
   B00010001,
   B00010001,
   B00001110,
   B00010001,
   B00010001,
   B00001110,
   B00000000},
  {B00001110, //9
   B00010001,
   B00010001,
   B00001111,
   B00000001,
   B00010001,
   B00001110,
   B00000000}
};

byte smallDigits[10][8]={
  {B00000000, //0
   B00000000,
   B00000111,
   B00000101,
   B00000101,
   B00000101,
   B00000111,
   B00000000},
  {B00000000, //1
   B00000000,
   B00000110,
   B00000010,
   B00000010,
   B00000010,
   B00000111,
   B00000000},
  {B00000000, //2
   B00000000,
   B00000111,
   B00000001,
   B00000111,
   B00000100,
   B00000111,
   B00000000},
  {B00000000, //3
   B00000000,
   B00000111,
   B00000001,
   B00000111,
   B00000001,
   B00000111,
   B00000000},
  {B00000000, //4
   B00000000,
   B00000101,
   B00000101,
   B00000111,
   B00000001,
   B00000001,
   B00000000},
  {B00000000, //5
   B00000000,
   B00000111,
   B00000100,
   B00000110,
   B00000001,
   B00000111,
   B00000000},
  {B00000000, //6
   B00000000,
   B00000111,
   B00000100,
   B00000111,
   B00000101,
   B00000111,
   B00000000},
  {B00000000, //7
   B00000000,
   B00000111,
   B00000001,
   B00000010,
   B00000100,
   B00000100,
   B00000000},
  {B00000000, //8
   B00000000,
   B00000111,
   B00000101,
   B00000111,
   B00000101,
   B00000111,
   B00000000},
  {B00000000, //9
   B00000000,
   B00000111,
   B00000101,
   B00000111,
   B00000001,
   B00000111,
   B00000000}
};

void refreshDisplay();
void drawNumber(byte[10][8], int, int, int, int, byte[LED_MODULES][8]);
void tikClock();
void blink_LED(int);

void setup() {
  pinMode(BUZZER, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
  while (!Serial); // for Leonardo/Micro/Zero

  if (setup_rtc() == RTC_STATUS_DEVICE_NOT_FOUND) {
    Serial.println("RTC device not found!");
    while (1) {
      blink_LED(40);
      delay(5);
    }
  }

  // Serial.begin(9600);
  Serial.println("setup ...");
  for (int d=0; d<4; d++) {
    lc.shutdown(d,false);
    lc.setIntensity(d,0);
    lc.clearDisplay(d);
  }
  
  for (int i=0; i<4; i++) pinMode(btnPin[i], INPUT_PULLUP);

  stop_melody();
  refreshDisplay();

/*
  TCCR0A=(1<<WGM01);    //Set the CTC mode   

  // OCR0A = (cLK)/PSC) * T - 1
  // CLK: Clock speed (16 MHz) = 16 10^6 Hz
  // PSC = [1 | 8 | 64 | 256]
  // T: Desired time period in seconds

  // OCR0A = (16e6/64) * 1e-3 - 1 =>
  // OCR0A = 249
  OCR0A=0xF9; //Value for ORC0A for 1ms 
  
  TIMSK0|=(1<<OCIE0A);   //Set the interrupt request
  sei(); //Enable interrupt
  
  TCCR0B|=(1<<CS01);    //Set the prescale 1/64 clock
  TCCR0B|=(1<<CS00);
*/

//set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // set compare match register for 400hz increments (T = 1/400 = 2.5ms)
  // OCR1A = 39999;// = (16*10^6) / (400*1) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Clock Select Bit Description
  // +----+----+----+--------------------------------------------------------+
  // |CS12|CS11|CS10| Description                                            |
  // +----+----+----+--------------------------------------------------------+
  // |  0 |  0 |  0 | No clock source (Timer/Counter stopped)                |
  // |  0 |  0 |  1 | clk/1 (No prescaling)                                  |
  // |  0 |  1 |  0 | clk/8 (From prescaler)                                 |
  // |  0 |  1 |  1 | clk/64 (From prescaler)                                |
  // |  1 |  0 |  0 | clk/256 (From prescaler)                               |
  // |  1 |  0 |  1 | clk/1024 (From prescaler)                              |
  // |  1 |  1 |  0 | External clock source on T1 pin. Clock on falling edge |
  // |  1 |  1 |  1 | External clock source on T1 pin. Clock on rising edge  |
  // +----+----+----+--------------------------------------------------------+
  // Set CS10 for no prescaler
  TCCR1B |= (1 << CS10 | 1 << CS12);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);


    // h = 12;
    // m = 34;

    if (debug) {
    Serial.print("Clock started: ");
    Serial.print(h); Serial.print(":"); Serial.println(m);
    }

}

ISR(TIMER1_COMPA_vect){    //This is the interrupt request
  // tikClock();
}

void drawLines(byte modules[LED_MODULES][8]){
  for (int m=0; m<LED_MODULES; m++){
    for (int row=0; row<8; row++){
      lc.setRow(m, row, modules[m][row]);
    }
  }
}

void drawNumber(byte digits[10][8], int num, int x, int dots, int charWidth, byte modules[LED_MODULES][8]) {
  int col = x / 8;
  int h,l = 0;
  int d = num / 10;
  int u = num % 10;
  int w = (8 - charWidth);
  int dx = ((col+1) * 8 -1) - x;

  byte btnHLed = 1 << mode[col];
  byte btnLLed = 1 << mode[col-1];

  for (int row=0; row<8; row++) {
    int vd = digits[d][row] << w;
    int vu = digits[u][row] << w;
    int line = (vd << (8)) | (vu << w);
    h = ((line >> 8) >> dx);
    l = (line >> dx) & 0x00FF;
    int k = (line << (8-dx)) & 0x00FF;
    if (dots > 0 && (row == 2 || row == 4)) {
      l |= (1 << 3);
    }
    if (debug && row == 7) {
      h |= btnHLed;
      l |= btnLLed;
    }

    modules[col][row] |= h;
     modules[col-1][row] |= l;
    if (col>=2) modules[col-2][row] |= k;
    // lc.setRow(col, row, h);
    // lc.setRow(col-1, row, l);

  }
}

void displayTime() {
  byte modules[LED_MODULES][8] = {0};
  drawNumber(digits, h, 31, hs, 6, modules);
  drawNumber(digits, m, 19, 0, 6, modules);
  drawNumber(smallDigits, s, 7,0,4,modules);
  drawLines(modules);
}

void displayYear() {
  int yH = year / 100;
  int yL = year % 100;
  byte modules[LED_MODULES][8] = {0};
    drawNumber(digits, yH, 31, 0, 6, modules);
    drawNumber(digits, yL, 15, 0, 6, modules);
  drawLines(modules);
}
void displayDate() {
  byte modules[LED_MODULES][8] = {0};
    drawNumber(digits, month, 31, 0, 6, modules);
    drawNumber(digits, day, 15, 0, 6, modules);
  drawLines(modules);
}
void displaySetHour() {
  byte modules[LED_MODULES][8] = {0};
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
    } else {
      drawNumber(digits, h, 31, hs, 6, modules);
    }
    drawNumber(digits, m, 15, 0, 6, modules);
  drawLines(modules);
 
}
void displaySetMinute() {
  byte modules[LED_MODULES][8] = {0};
    drawNumber(digits, h, 31, hs, 6, modules);
    if (hs == 0) {
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
      drawNumber(digits, m, 15, 0, 6, modules);
    }
  drawLines(modules);
}

char buf[256];

// void refreshDisplay() {
//   drawNumber(h, 3, hs, 0);
//   drawNumber(m, 1, 0, 1);
// }
void displaySetYear() {
  byte modules[LED_MODULES][8] = {0};
    int yH = year / 100;
    int yL = year % 100;
 
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
     drawNumber(digits, yH, 31, 0, 6, modules);
    drawNumber(digits, yL, 15, 0, 6, modules);
   }
  drawLines(modules);

}
void displaySetMonth() {
  byte modules[LED_MODULES][8] = {0};
    drawNumber(digits, day, 15, 0, 6, modules);
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
    } else {
      drawNumber(digits, month, 31, 0, 6, modules);
    }
  drawLines(modules);

}
void displaySetDay() {
  byte modules[LED_MODULES][8] = {0};
    drawNumber(digits, month, 31, 0, 6, modules);
    if (hs == 0) {
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
      drawNumber(digits, day, 15, 0, 6, modules);
    }
  drawLines(modules);

}

void refreshDisplay() {
  static unsigned long previousMillis = millis();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= refreshInterval) {
    previousMillis = currentMillis;
    blinkDotsC = (blinkDotsC + 1) % blinkDotsN;

    if (blinkDotsC == 0) hs = (hs + 1) % 2;

    switch(mode[0]) {
      case DISPLAY_TIME:
        if (reset_time == 1) {
          reset_time = 0;
          s = 0;
          adjust_rtc(year, month, day, h, m, s);
        }
        displayTime();
        break;
      case DISPLAY_YEAR:
        displayYear();
        break;
      case DISPLAY_DATE:
        displayDate();
        break;
      case SET_HOUR:
        reset_time = 1;
        displaySetHour();
        break;
      case SET_MINUTE:
        reset_time = 1;
        displaySetMinute();
        break;
      case SET_YEAR:
        displaySetYear();
        break;
      case SET_MONTH:
        displaySetMonth();
        break;
      case SET_DAY:
        displaySetDay();
        break;
    }
    if (debug) {
    Serial.print("previousMillis:");Serial.print(previousMillis);
    Serial.print(" currentMillis:");Serial.print(currentMillis);

    Serial.print("millis:");Serial.print(millis());
    Serial.println();
    }
  }
}

void tikClock1Hz() {
      s = (s + 1) % 60;

      if (s == 0) {
        m = (m + 1) % 60;

        if (m == 0) {
          h = (h + 1) % 24;
        }
      }

}

void tikClock2Hz() {
    ms = (ms + 1) % 2;
    if (ms == 0) {
      tikClock1Hz();      
    }

}

void tikClock400Hz() {
  ms = (ms + 1) % 400;

  if (ms == 0) {
    tikClock1Hz();
  }
}

void tikClock() {
  // tikClock2Hz();
  tikClock1Hz();
}

void stateHandler() {
  tone(BUZZER, 880, 150);

  switch (mode[0]) {
    case SET_HOUR:
        s = 0;
      if (btnState[3] == LOW) {
        h = (h + 23) % 24;
        adjust_rtc(year, month, day, h, m, s);
      } else if (btnState[2] == LOW) {
        h = (h + 1) % 24;
        adjust_rtc(year, month, day, h, m, s);
      }
      break;
    case SET_MINUTE:
        s = 0;
      if (btnState[3] == LOW) {
        m = (m + 59) % 60;
        adjust_rtc(year, month, day, h, m, s);
      } else if (btnState[2] == LOW) {
        m = (m + 1) % 60;
        adjust_rtc(year, month, day, h, m, s);
      }
      break;
    case SET_YEAR:
      if (btnState[3] == LOW) {
        year = (year + 2099) % 2100;
        adjust_rtc(year, month, day, h, m, s);
      } else if (btnState[2] == LOW) {
        year = (year + 1) % 2100;
        adjust_rtc(year, month, day, h, m, s);
      }
      break;
    case SET_MONTH:
      if (btnState[3] == LOW) {
        month = ((month - 1) + 11) % 12 + 1;
        adjust_rtc(year, month, day, h, m, s);
      } else if (btnState[2] == LOW) {
        month = month % 12 + 1;
        adjust_rtc(year, month, day, h, m, s);
      }
      break;
    case SET_DAY:
      bool isLeapYear = testLeapYear(year);
      int modulo = 30;

      switch (month) {
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12: 
          modulo = 31;
          break;
        case 2:
          modulo = isLeapYear ? 29 : 28;
          break;
      }

      if (btnState[3] == LOW) {
        day = (day - 1 + (modulo-1)) % modulo + 1; 
        adjust_rtc(year, month, day, h, m, s);
      } else if (btnState[2] == LOW) {
        day = day % modulo + 1;
        adjust_rtc(year, month, day, h, m, s);
      }
      break;
  }

  if (btnState[1] == LOW) {
    Serial.print("mode[1]:");
    Serial.println(mode[1]);
    begin_player();
    start_melody(mode[1] % btnModes[1]);
  }
  
}
void readBtn(int btn) {
  currentRead[btn] = digitalRead(btnPin[btn]);
  if (currentRead[btn] == btnState[btn]) {
    debounce[btn] = millis();
  }
  // if (btn==0){
  //   Serial.print("btnState:");Serial.print(btnState[btn]);
  //   Serial.print(" currentRead:");Serial.print(currentRead[btn]);
  //   Serial.print(" mode:");Serial.print(mode[btn]);
  //   Serial.println();
  // } 

  if (debug) {
  Serial.print(" currentRead:"); Serial.print(currentRead[btn]);
  Serial.print(" btn:"); Serial.print(btn);
  Serial.print(" btnState:"); Serial.print(btnState[btn]);
  Serial.print(" debounce:"); Serial.print(debounce[btn]);
  Serial.print(" millis:");Serial.println(millis());

  }

  if ((millis()-debounce[btn]) > debounceDelay) {
  // Serial.print(millis()-debounce[btn]);
  // Serial.print(" "); Serial.println(debounceDelay);
    if (currentRead[btn] == LOW && btnState[btn] == HIGH) {
      btnState[btn] = LOW;
      // Serial.print("[A] mode:");Serial.println(mode[btn]);
        mode[btn] = (mode[btn] + 1) % btnModes[btn];
        Serial.print("[B] mode:");Serial.println(mode[btn]);
        stateHandler();
    } else
    if (currentRead[btn] == HIGH && btnState[btn] == LOW) {
      btnState[btn] = HIGH;
    }
  }
}

void blink_LED(int onCycle) {
  static bool isON = false;
  static int count = 0;
  // int onCycleCount = onCycle >= 0 ? onCycle : 127;
  int offCycleCount = 256 - onCycle;
  count++;
  if (isON && count == onCycle) {
    count = 0;
    isON = false;
    digitalWrite(LED_BUILTIN, LOW);
    sprintf(buf, "%02d:%02d:%02d\r", h, m, s);
    sendSerial(buf);
  } else if (!isON && count == offCycleCount) {
    count = 0;
    isON = true;
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
void loop() {
  for (int btn=0; btn<4; btn++) {
    readBtn(btn);
    // Serial.print("btnState[");Serial.print(btn);Serial.print("]:");Serial.println(btnState[btn]);
  }

  get_date_time_rtc(year, month, day, h, m, s);
  refreshDisplay();
  // delay(20);

  // loop_rtc();
  blink_LED(LED_BUILTIN);

  // Serial.println("play_note()");
  play_note(BUZZER);
}
