#ifndef GUI_H
#define GUI_H

#include "../kernel/vga.h"

void gui_init();

void gui_draw_logo();

void gui_draw_time();
void gui_draw_memory();
void gui_draw_uptime();
void gui_draw_cpu_info();

#endif
