#include <Arduino.h>

#include <SoftwareSerial.h>

//Mario's Ideas
//MAX7219 - using Led Control library to display 8x8 bitmap
#include <LedControl.h>

int DIN = 8;
int CS = 9;
int CLK = 10;
LedControl lc= LedControl(DIN, CLK, CS,4);
int leftPin = A0;
int h,m,s,hs = 0;

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

void setup() {
  Serial.begin(9600);
  for (int d=0; d<4; d++) {
    lc.shutdown(d,false);
    lc.setIntensity(d,0);
    lc.clearDisplay(d);
  }
}

void drawNumber(int num, int col, int dots, int offset) {
  int d = num / 10;
  int u = num % 10;

  for (int row=0; row<8; row++) {
    int vd = digits[d][row] >> offset;
    int vu = digits[u][row] >> offset;
    if (dots > 0 && (row == 2 || row == 4)) {
      vu |= 1;
    }
    lc.setRow(col, row, vd);
    lc.setRow(col-1, row, vu);

  }
}


void simpleClock() {
  delay(500);

  hs = (hs +1) % 2;

  if (hs == 0) {
    s = (s + 1) % 60;

    if (s == 0) {
      m = (m + 1) % 60;

      if (m == 0) {
        h = (h + 1) % 24;
      }
    }
  }

  drawNumber(h, 3, hs, 0);
  drawNumber(m, 1, 0, 1);
}

void loop() {
  simpleClock();
}
