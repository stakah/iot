#include <Arduino.h>

#include <SoftwareSerial.h>

//Mario's Ideas
//MAX7219 - using Led Control library to display 8x8 bitmap
#include <LedControl.h>

// btn 0
#define DISPLAY_TIME 0
#define SET_HOUR 1
#define SET_MINUTE 2

int DIN = 8;
int CS = 9;
int CLK = 10;
LedControl lc= LedControl(DIN, CLK, CS,4);
int h,m,s,ms,hs = 0;

const int debug = 0; // 1-debug messages, 0-no debug messages
int btnPin[4] = {2, 3, 4, 5};
int btnState[4] = {LOW, LOW, LOW, LOW};
int currentRead[4] = {0, 0, 0, 0};
int mode[4] = {DISPLAY_TIME, 0, 0, 0};
int btnModes[4] = {3, 2, 2, 2};


unsigned long debounce[4] = {0, 0, 0, 0};
unsigned long debounceDelay = 50;
unsigned long refreshInterval = 100L;

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
  }
  
  for (int i=0; i<4; i++) pinMode(btnPin[i], INPUT);

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
  // OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // set compare match register for 2hz increments
  OCR1A = 15624/2;// = (16*10^6) / (2*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);


    h = 12;
    m = 55;

    if (debug) {
    Serial.print("Clock started: ");
    Serial.print(h); Serial.print(":"); Serial.println(m);
    }

}

ISR(TIMER1_COMPA_vect){    //This is the interrupt request
  tikClock();
}

void drawNumber(int num, int col, int dots, int offset) {
  int d = num / 10;
  int u = num % 10;

  byte btnHLed = 1 << mode[col];
  byte btnLLed = 1 << mode[col-1];

  for (int row=0; row<8; row++) {
    int vd = digits[d][row] >> offset;
    int vu = digits[u][row] >> offset;
    if (dots > 0 && (row == 2 || row == 4)) {
      vu |= 1;
    }
    if (row == 7) {
      vd |= btnHLed;
      vu |= btnLLed;
    }

    lc.setRow(col, row, vd);
    lc.setRow(col-1, row, vu);

  }
}

void displayTime() {
    drawNumber(h, 3, hs, 0);
    drawNumber(m, 1, 0, 1);
}
void displaySetHour() {
    if (hs == 0) {
      lc.clearDisplay(3);
      lc.clearDisplay(2);
    } else {
      drawNumber(h, 3, hs, 0);
    }
    drawNumber(m, 1, 0, 1);
 
}
void displaySetMinute() {
    drawNumber(h, 3, hs, 0);
    if (hs == 0) {
      lc.clearDisplay(1);
      lc.clearDisplay(0);
    } else {
      drawNumber(m, 1, 0, 1);
    }

}

void refreshDisplay() {
  static unsigned long previousMillis = millis();
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= refreshInterval) {
    previousMillis = currentMillis;

    switch(mode[0]) {
      case DISPLAY_TIME:
        displayTime();
        break;
      case SET_HOUR:
        displaySetHour();
        break;
      case SET_MINUTE:
        displaySetMinute();
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

void tikClock2Hz() {
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
void tikClock1ms() {

  ms = (ms + 1) % 500;

  if (ms == 0) {
    tikClock2Hz();
  }
}

void tikClock() {
  tikClock2Hz();
}

void stateHandler() {
  switch (mode[0]) {
    case SET_HOUR:
        s = 0;
      if (btnState[3] == HIGH) {
        h = (h + 23) % 24;
      } else if (btnState[2] == HIGH) {
        h = (h + 1) % 24;
      }
      break;
    case SET_MINUTE:
        s = 0;
      if (btnState[3] == HIGH) {
        m = (m + 59) % 60;
      } else if (btnState[2] == HIGH) {
        m = (m + 1) % 60;
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
    if (currentRead[btn] == HIGH && btnState[btn] == LOW) {
      btnState[btn] = HIGH;
      // Serial.print("[A] mode:");Serial.println(mode[btn]);
        mode[btn] = (mode[btn] + 1) % btnModes[btn];
        Serial.print("[B] mode:");Serial.println(mode[btn]);
        stateHandler();
    } else
    if (currentRead[btn] == LOW && btnState[btn] == HIGH) {
      btnState[btn] = LOW;
    }
  }
}

void loop() {
  for (int btn=0; btn<4; btn++) {
    readBtn(btn);
    // Serial.print("btnState[");Serial.print(btn);Serial.print("]:");Serial.println(btnState[btn]);
  }

  
  refreshDisplay();
  delay(20);
}
