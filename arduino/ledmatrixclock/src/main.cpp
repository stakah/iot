#include <Arduino.h>

#include <SoftwareSerial.h>

//Mario's Ideas
//MAX7219 - using Led Control library to display 8x8 bitmap
#include <LedControl.h>

int DIN = 8;
int CS = 9;
int CLK = 10;
LedControl lc= LedControl(DIN, CLK, CS,4);
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

void refreshDisplay();
void drawNumber(int, int, int, int);
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

void loop() {
  delay(200);
  refreshDisplay();
}
