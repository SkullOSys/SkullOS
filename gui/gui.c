#include "gui.h"
#include "../kernel/vga_manager.h"
#include "../drivers/rtc/rtc.h"
#include "../kernel/util.h"
#include <string.h>
#include "../kernel/memory.h"

void gui_draw_memory() {
    size_t free_mem = get_free_memory() / 1024;
    char mem_str[16];
    char mem_val[6];

    itoa(free_mem, mem_val, 10);
    mem_str[0] = '\0';
    strcat(mem_str, "Mem: ");
    strcat(mem_str, mem_val);
    strcat(mem_str, " KB");

    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(0, 2);
    vga_manager_puts(mem_str);
}

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

    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(0, 1);
    vga_manager_puts(time_str);
}

void gui_init() {
    vga_manager_set_context(true);
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_manager_clear();
    vga_manager_fullscreen_set_cursor_pos(0, 0);
    vga_manager_puts(" \n");
    gui_draw_time();
    vga_manager_puts(" \n");
    gui_draw_memory();
}
