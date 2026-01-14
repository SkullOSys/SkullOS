#include "rtc.h"
#include "../../kernel/kernel.h"

#define CMOS_ADDRESS 0x70
#define CMOS_DATA    0x71

// Helper function to convert BCD to decimal
static uint8_t bcd_to_dec(uint8_t bcd) {
    return (bcd / 16 * 10) + (bcd & 0x0F);
}

// Function to get the current time from the RTC
void rtc_get_time(rtc_time_t *time) {
    outb(CMOS_ADDRESS, 0x00);
    time->second = bcd_to_dec(inb(CMOS_DATA));

    outb(CMOS_ADDRESS, 0x02);
    time->minute = bcd_to_dec(inb(CMOS_DATA));

    outb(CMOS_ADDRESS, 0x04);
    time->hour = bcd_to_dec(inb(CMOS_DATA));
}
