#include "gui.h"
#include "../kernel/vga_manager.h"
#include "../drivers/rtc/rtc.h"
#include "../kernel/util.h"
#include "../kernel/timer.h"
#include "../kernel/cpu.h"
#include <string.h>
#include "../kernel/memory.h"

// Helper function to format time with leading zeros
static void format_time_str(char* str, uint8_t hour, uint8_t minute, uint8_t second) {
    char hour_str[3];
    char min_str[3];
    char sec_str[3];
    
    itoa(hour, hour_str, 10);
    itoa(minute, min_str, 10);
    itoa(second, sec_str, 10);
    
    str[0] = '\0';
    if (hour < 10) {
        strcat(str, "0");
    }
    strcat(str, hour_str);
    strcat(str, ":");
    if (minute < 10) {
        strcat(str, "0");
    }
    strcat(str, min_str);
    strcat(str, ":");
    if (second < 10) {
        strcat(str, "0");
    }
    strcat(str, sec_str);
}

// Draw the title bar
static void gui_draw_title_bar() {
    vga_manager_fullscreen_set_color(VGA_COLOR_BLACK, VGA_COLOR_BLUE);
    for (int x = 40; x < 80; x++) {
        vga_manager_fullscreen_set_cursor_pos(x, 0);
        vga_manager_fullscreen_putchar(' ');
    }
    
    const char* title = "SkullOS";
    int title_len = strlen(title);
    int title_x = 40 + (40 - title_len) / 2;
    vga_manager_fullscreen_set_cursor_pos(title_x, 0);
    vga_manager_fullscreen_puts(title);
}

// Draw a horizontal line
static void gui_draw_hline(int y) {
    vga_manager_fullscreen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE);
    for (int x = 40; x < 80; x++) {
        vga_manager_fullscreen_set_cursor_pos(x, y);
        vga_manager_fullscreen_putchar('-');
    }
}

void gui_draw_memory() {
    size_t free_mem = get_free_memory() / 1024;
    char mem_str[32];
    char mem_val[10];
    
    itoa(free_mem, mem_val, 10);
    mem_str[0] = '\0';
    strcat(mem_str, "Memory: ");
    strcat(mem_str, mem_val);
    strcat(mem_str, " KB");
    
    // Clear the line first
    vga_manager_fullscreen_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    for (int x = 40; x < 80; x++) {
        vga_manager_fullscreen_set_cursor_pos(x, 5);
        vga_manager_fullscreen_putchar(' ');
    }
    
    // Draw memory info
    vga_manager_fullscreen_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(41, 5);
    vga_manager_fullscreen_puts(mem_str);
}

void gui_draw_uptime() {
    uint32_t uptime = timer_get_uptime_seconds();
    uint32_t hours = uptime / 3600;
    uint32_t minutes = (uptime % 3600) / 60;
    uint32_t seconds = uptime % 60;
    
    char uptime_str[32];
    char hour_str[10], min_str[10], sec_str[10];
    
    itoa(hours, hour_str, 10);
    itoa(minutes, min_str, 10);
    itoa(seconds, sec_str, 10);
    
    uptime_str[0] = '\0';
    strcat(uptime_str, "Uptime: ");
    if (hours < 10) strcat(uptime_str, "0");
    strcat(uptime_str, hour_str);
    strcat(uptime_str, ":");
    if (minutes < 10) strcat(uptime_str, "0");
    strcat(uptime_str, min_str);
    strcat(uptime_str, ":");
    if (seconds < 10) strcat(uptime_str, "0");
    strcat(uptime_str, sec_str);
    
    // Clear the line first
    vga_manager_fullscreen_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    for (int x = 40; x < 80; x++) {
        vga_manager_fullscreen_set_cursor_pos(x, 7);
        vga_manager_fullscreen_putchar(' ');
    }
    
    // Draw uptime info
    vga_manager_fullscreen_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(41, 7);
    vga_manager_fullscreen_puts(uptime_str);
}

void gui_draw_cpu_info() {
    cpu_info_t* cpu = cpu_get_info();
    char cpu_str[48];
    
    cpu_str[0] = '\0';
    strcat(cpu_str, "CPU: ");
    
    // Truncate vendor string if too long
    char vendor_display[20];
    int vendor_len = strlen(cpu->vendor);
    if (vendor_len > 15) {
        strncpy(vendor_display, cpu->vendor, 15);
        vendor_display[15] = '\0';
    } else {
        strcpy(vendor_display, cpu->vendor);
    }
    strcat(cpu_str, vendor_display);
    
    // Clear the line first
    vga_manager_fullscreen_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    for (int x = 40; x < 80; x++) {
        vga_manager_fullscreen_set_cursor_pos(x, 9);
        vga_manager_fullscreen_putchar(' ');
    }
    
    // Draw CPU info
    vga_manager_fullscreen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(41, 9);
    vga_manager_fullscreen_puts(cpu_str);
}

void gui_draw_time() {
    rtc_time_t time;
    rtc_get_time(&time);
    
    char time_str[16];
    format_time_str(time_str, time.hour, time.minute, time.second);
    
    // Clear the line first
    vga_manager_fullscreen_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    for (int x = 40; x < 80; x++) {
        vga_manager_fullscreen_set_cursor_pos(x, 3);
        vga_manager_fullscreen_putchar(' ');
    }
    
    // Draw time with label
    vga_manager_fullscreen_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(41, 3);
    vga_manager_fullscreen_puts("Time: ");
    vga_manager_fullscreen_puts(time_str);
}

void gui_init() {
    vga_manager_set_context(true);
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_manager_clear();
    
    // Draw title bar
    gui_draw_title_bar();
    
    // Draw separator
    gui_draw_hline(1);
    
    // Draw separator before memory
    gui_draw_hline(4);
    
    // Draw separator before uptime
    gui_draw_hline(6);
    
    // Draw separator before CPU info
    gui_draw_hline(8);
    
    // Draw separator at bottom
    gui_draw_hline(24);
    
    // Draw system info label
    vga_manager_fullscreen_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(41, 2);
    vga_manager_fullscreen_puts("System Status");
    
    // Initial draw of time, memory, uptime, and CPU info
    gui_draw_time();
    gui_draw_memory();
    gui_draw_uptime();
    gui_draw_cpu_info();
    
    // Draw footer
    vga_manager_fullscreen_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLUE);
    vga_manager_fullscreen_set_cursor_pos(41, 23);
    vga_manager_fullscreen_puts("SkullOS v0.1.0");
}
