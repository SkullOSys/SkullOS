#ifndef DRIVERS_RTC_H
#define DRIVERS_RTC_H

#include <stdint.h>

typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
} rtc_time_t;

void rtc_get_time(rtc_time_t *time);

#endif // DRIVERS_RTC_H
