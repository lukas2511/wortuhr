/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include "tusb.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "wortuhr.h"
#include "timefoo.h"
#include "clock.c"
#include "oled.c"

char *weekdays[] = {
  "Sonntag",
  "Montag",
  "Dienstag",
  "Mittwoch",
  "Donnerstag",
  "Freitag",
  "Samstag"
};

int setter_selection = 0;

void draw_display(tmElements_t *tm, char* dstflag) {
  char buf[50];
  sprintf(buf, "%s\n%02d.%02d.%04d\n%02d:%02d:%02d %s", weekdays[tm->Wday], tm->Day, tm->Month, tmYearToCalendar(tm->Year), tm->Hour, tm->Minute, tm->Second, dstflag);
  printf("%s\n", buf);

  oled_clear();

  if (setter_selection == 0) {
    oled_draw_string(0, 0, "Klick = Aus\nwahl, Halte\nn = Plus");
  } else {
    oled_draw_string(0, 0, buf);
  }

  // weekday
  switch (setter_selection) {
    case 1: oled_draw_line(0, 8, strlen(weekdays[tm->Wday])*12-6, 8); break; // weekday
    case 2: oled_draw_line(0, 19, 18, 19); break; // day
    case 3: oled_draw_line(36, 19, 54, 19); break; // month
    case 4: oled_draw_line(72, 19, 114, 19); break; // year
    case 5: oled_draw_line(0, 31, 18, 31); break; // hour
    case 6: oled_draw_line(36, 31, 54, 31); break; // minute
    case 7: oled_draw_line(72, 31, 90, 31); break; // second
  }

  oled_display();

  int minute = tm->Minute;
  int hour = tm->Hour;

	// clock logic
  for(int i=2; i<=28; i++) {
    if(i == 7 || i==23 || i==24) continue;
    gpio_put(i, 0);
  }
	ES IST

	if(minute <= 2) { UHR }
	if(minute >=  3 && minute <=  7) { FUENF_1 NACH }
	if(minute >=  8 && minute <= 12) { ZEHN_1 NACH }
	if(minute >= 13 && minute <= 17) { VIER_1 TEL NACH }

	if(minute >= 18) hour++;

	if(minute >= 18 && minute <= 22) { ZEHN_1 VOR_1 HALB }
	if(minute >= 23 && minute <= 27) { FUENF_1 VOR_1 HALB }
	if(minute >= 28 && minute <= 32) { HALB }
	if(minute >= 33 && minute <= 37) { FUENF_1 NACH HALB }
	if(minute >= 38 && minute <= 42) { ZEHN_1 NACH HALB }
	if(minute >= 43 && minute <= 47) { VIER_1 TEL VOR_2 }
	if(minute >= 48 && minute <= 52) { ZEHN_1 VOR_1 }
	if(minute >= 53 && minute <= 57) { FUENF_1 VOR_1 }
	if(minute >= 58) { UHR }

	hour = hour % 12;

	if(hour == 0) { ZWOELF }
	if(hour == 1) { EIN if(minute >= 3 && minute <= 58) S } // "ES IST EIN UHR" vs "ES IST FUENF VOR EINS"
	if(hour == 2) { ZWEI }
	if(hour == 3) { DREI_2 }
	if(hour == 4) { VIER_2 }
	if(hour == 5) { FUENF_2 }
	if(hour == 6) { SECHS }
	if(hour == 7) { SIEBEN }
	if(hour == 8) { ACHT }
	if(hour == 9) { NEUN }
	if(hour == 10) { ZEHN_2 }
	if(hour == 11) { ELF }
}

int main() {
    stdio_usb_init();

    i2c_init(i2c0, 100*1000);
    gpio_set_function(0, GPIO_FUNC_I2C);
    gpio_set_function(1, GPIO_FUNC_I2C);

    oled_init();
    oled_clear();
    oled_display();

    for(int i=2; i<=28; i++) {
      if (i==7) continue;
      gpio_init(i);
      gpio_set_dir(i, GPIO_OUT);
    }

    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    tmElements_t tm;
    time_t time;
    bool clock_set = false;
    bool dstflag;

    while (true) {
      clock_set = false;
      read_clock(&tm);
      time = makeTime(&tm);
      if (is_dst(&tm, true)) {
        dstflag = true;
        time += 7200;
      } else {
        dstflag = false;
        time += 3600;
      }
      breakTime(time, &tm);

      if (!gpio_get(BUTTON)) {
        sleep_ms(300);
        if (gpio_get(BUTTON)) {
          setter_selection = (setter_selection + 1) % 8;
          if (setter_selection == 1) setter_selection = 2;
        } else {
          while (true) {
            if (gpio_get(BUTTON)) break;
            int delay = 300;
            switch (setter_selection) {
              case 1: tm.Wday = (tm.Wday + 1) % 7; break;
              case 2: tm.Day = 1 + (tm.Day % 31); break;
              case 3: tm.Month = 1 + (tm.Month % 12); break;
              case 4: tm.Year = 48 + ((tm.Year-48) + 1) % 30; delay = 200; break;
              case 5: tm.Hour = (tm.Hour + 1) % 24; break;
              case 6: tm.Minute = (tm.Minute + 1) % 60; delay = 200; break;
              case 7: tm.Second = (tm.Second + 1) % 60; delay = 10; break;
            }
            draw_display(&tm, "");
            sleep_ms(delay);
          }

          time = makeTime(&tm);
          if (is_dst(&tm, false)) {
            time -= 7200;
          } else {
            time -= 3600;
          }
          breakTime(time, &tm);
          set_clock(&tm);
          printf("Clock set!\n");
          clock_set = true;
        }
      }

      // Set clock via USB-CDC
      // `date -u +"S%Y-%m-%d %H:%M:%SF" > /dev/ttyACM2`
      if (tud_cdc_connected() && tud_cdc_available()) {
        int year = 0;
        int month = 0, day = 0, hour = 0, minute = 0, second = 0;
        if(getc(stdin) == 'S') {
          scanf("%d-%d-%d %d:%d:%dF", &year, &month, &day, &hour, &minute, &second);
          tm.Year = CalendarYrToTm(year);
          tm.Month = (uint8_t)month;
          tm.Day = (uint8_t)day;
          tm.Hour = (uint8_t)hour;
          tm.Minute = (uint8_t)minute;
          tm.Second = (uint8_t)second;
          time = makeTime(&tm);
          breakTime(time, &tm);

          set_clock(&tm);
          printf("Clock set!\n");
          clock_set = true;
        }
      }

      if (clock_set) continue;

      draw_display(&tm, dstflag ? "SZ" : "WZ");
      for(int i=0; i<10; i++) {
        if(!gpio_get(BUTTON)) break;
        sleep_ms(10);
      }
    }
}
