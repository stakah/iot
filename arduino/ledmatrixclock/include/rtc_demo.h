#ifndef __RTC_DEMO__
#define __RTC_DEMO__
void setup_rtc();
void loop_rtc();
void get_date_time_rtc(int &year, int &month, int &day, int &hour, int &minute, int &second);
void adjust_rtc(int year, int month, int day, int hour, int minute, int second);
bool testLeapYear(int year);
#endif
