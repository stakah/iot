#include <Arduino.h>

#include <SoftwareSerial.h>

//Mario's Ideas
//MAX7219 - using Led Control library to display 8x8 bitmap
#include <LedControl.h>

#include <rtc_demo.h>

// btn 0
#define DISPLAY_TIME 0
#define DISPLAY_YEAR 1
#define DISPLAY_DATE 2
#define SET_HOUR 3
#define SET_MINUTE 4
#define SET_YEAR 5
#define SET_MONTH 6
#define SET_DAY 7

int DIN = 8;
int CS = 9;
int CLK = 10;
LedControl lc= LedControl(DIN, CLK, CS,4);
int h,m,s,ms,hs = 0;
int year, month, day = 0;

const int debug = 0; // 1-debug messages, 0-no debug messages
int btnPin[4] = {2, 3, 4, 5};
int btnState[4] = {HIGH, HIGH, HIGH, HIGH};
int currentRead[4] = {0, 0, 0, 0};
int mode[4] = {DISPLAY_TIME, 0, 0, 0};
int btnModes[4] = {8, 2, 2, 2};


unsigned long debounce[4] = {0, 0, 0, 0};
unsigned long debounceDelay = 50;
unsigned long refreshInterval = 200L;
int blinkDotsN = 1000L / refreshInterval / 2;
int blinkDotsC = 0;

int reset_time = 0; // 0 = NO, 1 = YES

byte digits[10][8]={
  {B00001110, //0
   B00010001,
   B00010001,
   B00010101,
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

void refreshDisplay();
void drawNumber(int, int, int, int);
void tikClock();

void setup() {
  setup_rtc();
  // Serial.begin(9600);
  Serial.println("setup ...");
  for (int d=0; d<4; d++) {
    lc.shutdown(d,false);
    lc.setIntensity(d,0);
    lc.clearDisplay(d);
  }
  
  for (int i=0; i<4; i++) pinMode(btnPin[i], INPUT_PULLUP);

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

void drawNumber(int num, int x, int dots, int charWidth) {
  int col = 3 - x / 8;
  int dx = - (x % 8) + 1;
  int w = 8 - charWidth;
  int dxu = dx + w;
  int dxd = dxu + charWidth;
  int h,l = 0;
  int d = num / 10;
  int u = num % 10;

  byte btnHLed = 1 << mode[col];
  byte btnLLed = 1 << mode[col-1];

  for (int row=0; row<8; row++) {
    int vd = digits[d][row] << w;
    int vu = digits[u][row] << w;
    int line = (vu << dxu) | (vd << dxd);
    h = (line & 0xFF00) >> 8;
    l = (line & 0x00FF);
    if (dots > 0 && (row == 2 || row == 4)) {
      l |= 2;
    }
    if (row == 7) {
      h |= btnHLed;
      l |= btnLLed;
    }

    lc.setRow(col, row, h);
    lc.setRow(col-1, row, l);

  }
}

void displayTime() {
    drawNumber(h, 2, hs, 6);
    drawNumber(m, 16, 0, 6);
}
void displayYear() {
  int yH = year / 100;
  int yL = year % 100;
    drawNumber(yH, 2, 0, 6);
    drawNumber(yL, 16, 0, 6);
}
void displayDate() {
    drawNumber(month, 2, 0, 6);
    drawNumber(day, 16, 0, 6);
}
void displaySetHour() {
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
    } else {
      drawNumber(h, 2, hs, 6);
    }
    drawNumber(m, 16, 0, 6);
 
}
void displaySetMinute() {
    drawNumber(h, 2, hs, 6);
    if (hs == 0) {
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
      drawNumber(m, 16, 0, 6);
    }

}
void displaySetYear() {
    int yH = year / 100;
    int yL = year % 100;
 
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
     drawNumber(yH, 2, 0, 6);
    drawNumber(yL, 16, 0, 6);
   }

}
void displaySetMonth() {
    drawNumber(day, 16, 0, 6);
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
    } else {
      drawNumber(month, 2, 0, 6);
    }

}
void displaySetDay() {
    drawNumber(month, 2, 0, 6);
    if (hs == 0) {
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
      drawNumber(day, 16, 0, 6);
    }

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

void loop() {
  for (int btn=0; btn<4; btn++) {
    readBtn(btn);
    // Serial.print("btnState[");Serial.print(btn);Serial.print("]:");Serial.println(btnState[btn]);
  }

  get_date_time_rtc(year, month, day, h, m, s);
  refreshDisplay();
  // delay(20);

  // loop_rtc();
}
