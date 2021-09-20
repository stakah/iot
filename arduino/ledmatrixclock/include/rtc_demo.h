#ifndef __RTC_DEMO__
#define __RTC_DEMO__

#define RTC_STATUS_OK 0
#define RTC_STATUS_DEVICE_NOT_FOUND -1
#define RTC_STATUS_DEVICE_NOT_RUNNING -2

int setup_rtc();
void loop_rtc();
void get_date_time_rtc(int &year, int &month, int &day, int &hour, int &minute, int &second);
void adjust_rtc(int year, int month, int day, int hour, int minute, int second);
bool testLeapYear(int year);
#endif
