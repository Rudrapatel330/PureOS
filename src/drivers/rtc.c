#include "rtc.h"
#include "../kernel/config.h"
#include "ports.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71

int rtc_update_in_progress() {
  outb(CMOS_ADDRESS, 0x0A);
  return (inb(CMOS_DATA) & 0x80);
}

uint8_t get_rtc_register(int reg) {
  outb(CMOS_ADDRESS, reg);
  return inb(CMOS_DATA);
}

void rtc_init() {
  // Nothing specific to init for basic reading
}

void rtc_read(rtc_time_t *time) {
  // Wait until update is not in progress
  while (rtc_update_in_progress())
    ; // prevent reading inconsistent data

  time->second = get_rtc_register(0x00);
  time->minute = get_rtc_register(0x02);
  time->hour = get_rtc_register(0x04);
  time->day = get_rtc_register(0x07);
  time->month = get_rtc_register(0x08);
  time->year = get_rtc_register(0x09);

  uint8_t registerB = get_rtc_register(0x0B);

  // Convert BCD to Binary if necessary
  if (!(registerB & 0x04)) {
    time->second = (time->second & 0x0F) + ((time->second / 16) * 10);
    time->minute = (time->minute & 0x0F) + ((time->minute / 16) * 10);
    time->hour = ((time->hour & 0x0F) + (((time->hour & 0x70) / 16) * 10)) |
                 (time->hour & 0x80);
    time->day = (time->day & 0x0F) + ((time->day / 16) * 10);
    time->month = (time->month & 0x0F) + ((time->month / 16) * 10);
    time->year = (time->year & 0x0F) + ((time->year / 16) * 10);
  }

  // Convert 12 hour clock to 24 hour clock if necessary
  if (!(registerB & 0x02) && (time->hour & 0x80)) {
    time->hour = ((time->hour & 0x7F) + 12) % 24;
  }

  // Apply Timezone Offset
  extern os_config_t global_config;
  int offset = global_config.timezone_offset_m;
  if (offset != 0) {
    int minutes = time->hour * 60 + time->minute + offset;
    int days_off = 0;

    while (minutes >= 1440) {
      minutes -= 1440;
      days_off++;
    }
    while (minutes < 0) {
      minutes += 1440;
      days_off--;
    }

    time->hour = minutes / 60;
    time->minute = minutes % 60;

    if (days_off != 0) {
      int d = time->day + days_off;
      int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
      if (time->year % 4 == 0)
        days_in_month[1] = 29;

      if (d > days_in_month[(time->month - 1) % 12]) {
        d = 1;
        time->month++;
        if (time->month > 12) {
          time->month = 1;
          time->year++;
        }
      } else if (d < 1) {
        time->month--;
        if (time->month < 1) {
          time->month = 12;
          time->year--;
        }
        d = days_in_month[(time->month - 1) % 12];
      }
      time->day = d;
    }
  }
}
