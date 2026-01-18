#ifndef KERNEL_TIMER_H
#define KERNEL_TIMER_H

#include <stdint.h>

void timer_init(void);
uint32_t timer_get_uptime_seconds(void);

#endif // KERNEL_TIMER_H
