#include <Arduino.h>

#include <SoftwareSerial.h>

//Mario's Ideas
//MAX7219 - using Led Control library to display 8x8 bitmap
#include <LedControl.h>

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

byte digits[10][8]={
  {B01111000, //0
   B11000100,
   B10100100,
   B10110100,
   B10010100,
   B10001100,
   B01111000,
   B00000000},
  {B00010000, //1
   B01110000,
   B00010000,
   B00010000,
   B00010000,
   B00010000,
   B01111100,
   B00000000},
  {B01111000, //2
   B10000100,
   B00000100,
   B01111000,
   B10000000,
   B10000000,
   B11111100,
   B00000000},
  {B01111000, //3
   B10000100,
   B00000100,
   B00111000,
   B00000100,
   B10000100,
   B01111000,
   B00000000},
  {B00010000, //4
   B00110000,
   B01010000,
   B10010000,
   B11111100,
   B00010000,
   B00010000,
   B00000000},
  {B11111100, //5
   B10000000,
   B10111000,
   B11000100,
   B00000100,
   B10000100,
   B01111000,
   B00000000},
  {B01111000, //6
   B10000100,
   B10000000,
   B11111000,
   B10000100,
   B10000100,
   B01111000,
   B00000000},
  {B11111000, //7
   B10001000,
   B00010000,
   B00010000,
   B00100000,
   B00100000,
   B00100000,
   B00000000},
  {B01111000, //8
   B10000100,
   B10000100,
   B11111100,
   B10000100,
   B10000100,
   B01111000,
   B00000000},
  {B01111000, //9
   B10000100,
   B10000100,
   B01111100,
   B00000100,
   B10000100,
   B01111000,
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
void drawNumber(int, int, int, int, byte[LED_MODULES][8]);
void tikClock();

void setup() {
  Serial.begin(9600);
  for (int d=0; d<4; d++) {
    lc.shutdown(d,false);
    lc.setIntensity(d,0);
    lc.clearDisplay(d);
    h = 8;
    m = 9;
  }
  
  pinMode(LED_BUILTIN, OUTPUT);
  refreshDisplay();

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

}

ISR(TIMER0_COMPA_vect){    //This is the interrupt request
  tikClock();
}

void drawLines(byte modules[LED_MODULES][8]){
  for (int m=0; m<LED_MODULES; m++){
    for (int row=0; row<8; row++){
      lc.setRow(m, row, modules[m][row]);
    }
  }
}

void drawNumber(int num, int x, int dots, int charWidth, byte modules[LED_MODULES][8]) {
  int col = x / 8;
  int h,l = 0;
  int d = num / 10;
  int u = num % 10;
  int w = (8 - charWidth);
  int dx = ((col+1) * 8 -1) - x;

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

void drawSmallNumber(int num, int x, int dots, int charWidth, byte modules[LED_MODULES][8]) {
  int col = x / 8;
  int h,l = 0;
  int d = num / 10;
  int u = num % 10;
  int w = (8 - charWidth);
  int dx = ((col+1) * 8 -1) - x;

  byte btnHLed = 1 << mode[col];
  byte btnLLed = 1 << mode[col-1];

  for (int row=0; row<8; row++) {
    int vd = smallDigits[d][row] << w;
    int vu = smallDigits[u][row] << w;
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
  drawNumber(h, 31, hs, 6, modules);
  drawNumber(m, 19, 0, 6, modules);
  drawSmallNumber(s, 7,0,4,modules);
  drawLines(modules);
}

void displayYear() {
  int yH = year / 100;
  int yL = year % 100;
  byte modules[LED_MODULES][8] = {0};
    drawNumber(yH, 31, 0, 6, modules);
    drawNumber(yL, 15, 0, 6, modules);
  drawLines(modules);
}
void displayDate() {
  byte modules[LED_MODULES][8] = {0};
    drawNumber(month, 31, 0, 6, modules);
    drawNumber(day, 15, 0, 6, modules);
  drawLines(modules);
}
void displaySetHour() {
  byte modules[LED_MODULES][8] = {0};
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
    } else {
      drawNumber(h, 31, hs, 6, modules);
    }
    drawNumber(m, 15, 0, 6, modules);
  drawLines(modules);
 
}
void displaySetMinute() {
  byte modules[LED_MODULES][8] = {0};
    drawNumber(h, 31, hs, 6, modules);
    if (hs == 0) {
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
      drawNumber(m, 15, 0, 6, modules);
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
     drawNumber(yH, 31, 0, 6, modules);
    drawNumber(yL, 15, 0, 6, modules);
   }
  drawLines(modules);

}
void displaySetMonth() {
  byte modules[LED_MODULES][8] = {0};
    drawNumber(day, 15, 0, 6, modules);
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
    } else {
      drawNumber(month, 31, 0, 6, modules);
    }
  drawLines(modules);

}
void displaySetDay() {
  byte modules[LED_MODULES][8] = {0};
    drawNumber(month, 31, 0, 6, modules);
    if (hs == 0) {
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
      drawNumber(day, 15, 0, 6, modules);
    }
  drawLines(modules);

}

void refreshDisplay() {
  drawNumber(h, 3, hs, 0);
  drawNumber(m, 1, 0, 1);
}

void tikClock() {

  ms = (ms + 1) % 500;

  if (ms == 0) {
    hs = (hs + 1) % 2;

    if (hs == 0) {
      s = (s + 1) % 60;

      if (s == 0) {
        m = (m + 1) % 60;

        if (m == 0) {
          h = (h + 1) % 24;
        }
      }
    }
  }
}

void blink_LED() {
  static bool isON = false;
  static int count = 0;

  count++;
  if (count == 10) {
    count = 0;
    if (isON) {
      isON = false;
      digitalWrite(LED_BUILTIN, LOW);
      sprintf(buf, "%02d:%02d:%02d\r", h, m, s);
      sendSerial(buf);
    } else {
      isON = true;
      digitalWrite(LED_BUILTIN, HIGH);
    }
  }
}
void loop() {
  delay(200);
  refreshDisplay();
  blink_LED();
}
