#pragma once

#include <Arduino.h>

struct TimeUtil {
    long epochTime;
    struct tm *ptm;
    String formattedTime;
    int currentHour;
    int currentMinute;
    int currentSecond;
    String weekDay;
    int currentWDay;
    int currentMonthDay;
    int currentMonth;
    String currentMonthName;
    int currentYear;
    String formattedDate;
};

class Util {
    //Week Days
    String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    //Month names
    String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

    public:
    struct TimeUtil * currentDateTime(long currentEpoch, struct TimeUtil *timeUtil);
};

