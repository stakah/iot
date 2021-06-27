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

byte Cat[8] ={B10001000,
              B11111000,
              B10101000,
              B01110010,
              B00100100,
              B01111100,
              B01111110,
              B10111100 };
byte Apple [8]={B01100000,
                B00010000,
                B01101100,
                B11111010,
                B11111110,
                B11111110,
                B01111100,
                B00101000};
byte Heart [8]={B01101100,
                B10010010,
                B10000010,
                B10000010,
                B10000010,
                B01000100,
                B00101000,
                B00010000};
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

int debug = 1;
void setup() {
  Serial.begin(9600);
  for (int d=0; d<4; d++) {
    lc.shutdown(d,false);
    lc.setIntensity(d,0);
    lc.clearDisplay(d);
  }
//  m = 39;
}

void drawNumber(int num, int col, int dots, int offset) {
  int d = num / 10;
  int u = num % 10;
//    Serial.print("num:");Serial.print(num);
//    Serial.print(" col:");Serial.print(col);
//    Serial.print(" d:"); Serial.print(d);
//    Serial.print(" u:"); Serial.print(u);
//    Serial.println();
  for (int row=0; row<8; row++) {
    int vd = digits[d][row] >> offset;
    int vu = digits[u][row] >> offset;
    if (dots > 0 && (row == 2 || row == 4)) {
      vu |= 1;
    }
    lc.setRow(col, row, vd);
    lc.setRow(col-1, row, vu);
//    Serial.print(" vd:");Serial.print(vd);
//    Serial.print(" vu:");Serial.println(vu);
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
//  pictures();
//  vu();
  simpleClock();
}

int leftCh = 32;
int rightCh = 0;
int maxAna = 0;
int minAna = 2000;

void drawLine(int row, int val) {
  byte aux;
  for (int d=0; d<4; d++) {
    aux = val>=8 * (d+1) ? 8 : min(val,8 * (d+1))-(8 * d);
    lc.setRow(d, row, 255 >> (8 - aux));
    
  }
  
}
void vu() {
//  leftCh = random(33);
//  rightCh = random(33);
  int anaLeft = analogRead(leftPin);
  // 24 - x - 335
  //  0 - y -  32
  // (335-25)/(32-0) = (x-25)/(y-0)]
  // y * 32 = 310 * (x-25) => y = 310/32 * (x-25)
  
  leftCh = abs(anaLeft - 0) * 348 / 32;
  rightCh = leftCh;

  if (anaLeft > maxAna) {
    maxAna = anaLeft;
    Serial.print("minAna: ");
    Serial.print(minAna);
    Serial.print("maxAna: ");
    Serial.println(maxAna);
  }
  else if (anaLeft < minAna) {
    minAna = anaLeft;
    Serial.print("minAna: ");
    Serial.print(minAna);
    Serial.print("maxAna: ");
    Serial.println(maxAna);
  }
  
  if (false && debug) {
  Serial.print("leftPin: ");
  Serial.print(anaLeft);
  Serial.print("  leftCh: ");
  Serial.println(leftCh);
  delay(30);
  }
  
  drawLine(1, leftCh);
  drawLine(2, leftCh);

  drawLine(5, rightCh);
  drawLine(6, rightCh);

//  delayMicroseconds(20);
   delay(50);
}

void pictures(){
  static int c = 0;
  static int d = 1;
  static int e = 2;
  static int f = 3;
//  do {
    for(int i=0;i<8;i++) {
      lc.setRow(d,i,Apple[i]);
      lc.setRow(e,i,Cat[i]);
      lc.setRow(f,i,Heart[i]);
      lc.setRow(c,i,0);
      delay(200);
    }
    delay(5000);
    c = (c + 1) % 4;
    d = (d + 1) % 4;
    e = (e + 1) % 4;
    f = (f + 1) % 4;
//  } while (1);
}
