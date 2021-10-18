// check if given time should be dst (timezones didn't work so i have to use this ugly workaround...)
int is_dst(tmElements_t *tm, bool utc) {
  if(tm->Month > 3 && tm->Month < 10) return 1; // april - september is dst
  if(tm->Month == 10 && tm->Day < 25) return 1; // october up to last week is dst

  if(tm->Month < 3 || tm->Month > 10) return 0; // november - february is not dst
  if(tm->Month == 3 && tm->Day < 25) return 0; // march up to last week is not dst

  // idea: calculate remaining days of week and remaining days of month
  // if remaining days of week > remaining days of month we are on or after the last sunday of the month
  // for sunday we also check if it is >=3 hours after midnight in utc
  int rem_days_of_month = 31 - tm->Day;
  int rem_days_of_week = (6 - tm->Wday);

  if(tm->Month == 3) {
    if (rem_days_of_week >= rem_days_of_month) {
      if (tm->Wday == 0) {
        return tm->Hour >= ((utc) ? 1 : 3);
      } else {
        return 1;
      }
    }
    return 0;
  }

  if (tm->Month == 10) {
    if (rem_days_of_week >= rem_days_of_month) {
      if(tm->Wday == 0) {
        return tm->Hour < ((utc) ? 1 : 3);
      } else {
        return 0;
      }
    }
    return 1;
  }

  return 0; // should not happen
}

void set_clock(tmElements_t *tm) {
  uint8_t timefoo[10];
  timefoo[0] = 0x07;
  timefoo[1] = 0x00;
  i2c_write_blocking(i2c0, 0x68, timefoo, 8, false);

  timefoo[0] = 0x00;
  timefoo[1] = 0x80;
  timefoo[2] = dec2bcd(tm->Minute);
  timefoo[3] = dec2bcd(tm->Hour);
  timefoo[4] = dec2bcd(tm->Wday);
  timefoo[5] = dec2bcd(tm->Day);
  timefoo[6] = dec2bcd(tm->Month);
  timefoo[7] = dec2bcd(tmYearToY2k(tm->Year));
  i2c_write_blocking(i2c0, 0x68, timefoo, 8, false);

  timefoo[0] = 0x07;
  timefoo[1] = 0x90;
  i2c_write_blocking(i2c0, 0x68, timefoo, 2, false);

  timefoo[0] = 0x00;
  timefoo[1] = dec2bcd(tm->Second);
  i2c_write_blocking(i2c0, 0x68, timefoo, 2, false);
}

void read_clock(tmElements_t *tm) {
  uint8_t timefoo[10];
  timefoo[0] = 0x00;
  i2c_write_blocking(i2c0, 0x68, timefoo, 1, false);

  uint8_t buf[10];
  for(int i=0; i<10; i++) buf[i] = 0;
  i2c_read_blocking(i2c0, 0x68, buf, 7, false);
  printf("RTC: %02x %02x %02x %02x %02x %02x %02x\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
  tm->Second = bcd2dec(buf[0]);
  tm->Minute = bcd2dec(buf[1]);
  tm->Hour = bcd2dec(buf[2]);
  tm->Wday = bcd2dec(buf[3]);
  tm->Day = bcd2dec(buf[4]);
  tm->Month = bcd2dec(buf[5]);
  tm->Year = y2kYearToTm(bcd2dec(buf[6]));
}

