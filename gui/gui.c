#include "gui.h"
#include "../kernel/vga_manager.h"
#include "../drivers/rtc/rtc.h"
#include "../kernel/util.h"
#include <string.h>

void gui_draw_time() {
    rtc_time_t time;
    rtc_get_time(&time);

    char time_str[9];
    char hour_str[3];
    char min_str[3];
    char sec_str[3];

    itoa(time.hour, hour_str, 10);
    itoa(time.minute, min_str, 10);
    itoa(time.second, sec_str, 10);

    time_str[0] = '\0';
    if (time.hour < 10) {
        strcat(time_str, "0");
    }
    strcat(time_str, hour_str);
    strcat(time_str, ":");
    if (time.minute < 10) {
        strcat(time_str, "0");
    }
    strcat(time_str, min_str);
    strcat(time_str, ":");
    if (time.second < 10) {
        strcat(time_str, "0");
    }
    strcat(time_str, sec_str);

    vga_manager_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(VGA_WIDTH - 8, 0);
    vga_manager_puts(time_str);
}

void gui_init() {
    vga_manager_set_context(true);
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_manager_clear();
    gui_draw_time();
}
