/*
 * Adapted from https://github.com/kachok/arduino-libraries/blob/master/Time/Time.h
 */

typedef enum {timeNotSet, timeNeedsSync, timeSet
}  timeStatus_t ;

typedef enum {
  dowInvalid, dowSunday, dowMonday, dowTuesday, dowWednesday, dowThursday, dowFriday, dowSaturday
} timeDayOfWeek_t;

typedef enum {
  tmSecond, tmMinute, tmHour, tmWday, tmDay,tmMonth, tmYear, tmNbrFields
} tmByteFields;

typedef struct  { 
  uint8_t Second; 
  uint8_t Minute; 
  uint8_t Hour; 
  uint8_t Wday;   // day of week, sunday is day 1
  uint8_t Day;
  uint8_t Month; 
  uint8_t Year;   // offset from 1970; 
} tmElements_t, TimeElements, *tmElementsPtr_t;

//convenience macros to convert to and from tm years
#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)

/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000  (946684800UL) // the time at the start of y2k

uint8_t dec2bcd(uint8_t num) { return ((num/10 * 16) + (num % 10)); }
uint8_t bcd2dec(uint8_t num) { return ((num/16 * 10) + (num % 16)); }

#define LEAP_YEAR(Y)     ( ((1970+Y)>0) && !((1970+Y)%4) && ( ((1970+Y)%100) || !((1970+Y)%400) ) )
static const uint8_t monthDays[]={31,28,31,30,31,30,31,31,30,31,30,31}; // API starts months from 1, this array starts from 0

void breakTime(time_t time, tmElements_t *tm){
// break the given time_t into time components
// this is a more compact version of the C library localtime function
// note that year is offset from 1970 !!!

  uint8_t year;
  uint8_t month, monthLength;
  unsigned long days;

  tm->Second = time % 60;
  time /= 60; // now it is minutes
  tm->Minute = time % 60;
  time /= 60; // now it is hours
  tm->Hour = time % 24;
  time /= 24; // now it is days
  tm->Wday = ((time + 4) % 7);  // Sunday is day 0

  year = 0;
  days = 0;
  while((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time) {
    year++;
  }
  tm->Year = year; // year is offset from 1970

  days -= LEAP_YEAR(year) ? 366 : 365;
  time  -= days; // now it is days in this year, starting at 0

  days=0;
  month=0;
  monthLength=0;
  for (month=0; month<12; month++) {
    if (month==1) { // february
      if (LEAP_YEAR(year)) {
        monthLength=29;
      } else {
        monthLength=28;
      }
    } else {
      monthLength = monthDays[month];
    }

    if (time >= monthLength) {
      time -= monthLength;
    } else {
        break;
    }
  }
  tm->Month = month + 1;  // jan is month 1
  tm->Day = time + 1;     // day of month
}

time_t makeTime(tmElements_t *tm){
// assemble time elements into time_t
// note year argument is offset from 1970 (see macros in time.h to convert to other formats)
// previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

  int i;
  time_t seconds;

  // seconds from 1970 till 1 jan 00:00:00 of the given year
  seconds= tm->Year*(SECS_PER_DAY * 365);
  for (i = 0; i < tm->Year; i++) {
    if (LEAP_YEAR(i)) {
      seconds +=  SECS_PER_DAY;   // add extra days for leap years
    }
  }

  // add days for this year, months start from 1
  for (i = 1; i < tm->Month; i++) {
    if ( (i == 2) && LEAP_YEAR(tm->Year)) {
      seconds += SECS_PER_DAY * 29;
    } else {
      seconds += SECS_PER_DAY * monthDays[i-1];  //monthDay array starts from 0
    }
  }
  seconds+= (tm->Day-1) * SECS_PER_DAY;
  seconds+= tm->Hour * SECS_PER_HOUR;
  seconds+= tm->Minute * SECS_PER_MIN;
  seconds+= tm->Second;
  return seconds;
}
