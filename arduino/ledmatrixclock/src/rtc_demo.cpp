// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include "RTClib.h"
#include "rtc_demo.h"

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


int setup_rtc () {
 while (!Serial); // for Leonardo/Micro/Zero
 Serial.begin(9600);
 if (! rtc.begin()) {
  //  Serial.println("Couldn't find RTC");
   return RTC_STATUS_DEVICE_NOT_FOUND;
 }
 Serial.println("setup_rtc ...");

 if (! rtc.isrunning()) {
   Serial.println("RTC is NOT running!");
   // following line sets the RTC to the date & time this sketch was compiled
   rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   // This line sets the RTC with an explicit date & time, for example to set
   // January 21, 2014 at 3am you would call:
   // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
 }

 return RTC_STATUS_OK;
}

void adjust_rtc(int year, int month, int day, int hour, int minute, int second) {
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
}

void get_date_time_rtc(int &year, int &month, int &day, int &hour, int &minute, int &second) {
    DateTime now = rtc.now();
    year = now.year(); month = now.month(); day = now.day();
    hour = now.hour(); minute = now.minute(); second = now.second();
}

void loop_rtc () {
 DateTime now = rtc.now();
 Serial.print(now.year(), DEC);
 Serial.print('/');
 Serial.print(now.month(), DEC);
 Serial.print('/');
 Serial.print(now.day(), DEC);
 Serial.print(" (");
 Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
 Serial.print(") ");
 Serial.print(now.hour(), DEC);
 Serial.print(':');
 Serial.print(now.minute(), DEC);
 Serial.print(':');
 Serial.print(now.second(), DEC);
 Serial.println();
 Serial.print(" since midnight 1/1/1970 = ");
 Serial.print(now.unixtime());
 Serial.print("s = ");
 Serial.print(now.unixtime() / 86400L);
 Serial.println("d");
 // calculate a date which is 7 days and 30 seconds into the future
 DateTime future (now + TimeSpan(7, 12, 30, 6));
 Serial.print(" now + 7d + 30s: ");
 Serial.print(future.year(), DEC);
 Serial.print('/');
 Serial.print(future.month(), DEC);
 Serial.print('/');
 Serial.print(future.day(), DEC);
 Serial.print(' ');
 Serial.print(future.hour(), DEC);
 Serial.print(':');
 Serial.print(future.minute(), DEC);
 Serial.print(':');
 Serial.print(future.second(), DEC);
 Serial.println();
 Serial.println();
 delay(3000);
}

bool testLeapYear(int year) {
  return (year % 400 == 0 || (year % 100 != 0 && year %4 == 0)); 
}