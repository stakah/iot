#include "util.h"

struct TimeUtil * Util::currentDateTime(long currentEpoch, struct TimeUtil *timeUtil) {
    if (timeUtil == NULL) return NULL;

    timeUtil->epochTime = currentEpoch;

    struct tm *ptm = gmtime ((time_t *)&(timeUtil->epochTime)); 
    timeUtil->ptm = ptm;
    char buf[11];
    strftime(buf, 11, "%T", ptm);
    timeUtil->formattedTime = buf;
    timeUtil->currentHour = ptm->tm_hour;
    timeUtil->currentMinute = ptm->tm_min;
    timeUtil->currentSecond = ptm->tm_sec;
    timeUtil->currentWDay = ptm->tm_wday;
    timeUtil->weekDay = weekDays[ptm->tm_wday];
    timeUtil->currentMonthDay = ptm->tm_mday;
    timeUtil->currentMonth = ptm->tm_mon+1;
    timeUtil->currentMonthName = months[timeUtil->currentMonth-1];
    timeUtil->currentYear = ptm->tm_year+1900;
    strftime(buf, 11, "%F", ptm);
    timeUtil->formattedDate = buf;
    return timeUtil;
}
