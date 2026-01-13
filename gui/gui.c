#include "gui.h"
#include "../kernel/vga_manager.h"
#include <string.h>

// Function to initialize the GUI
void gui_init() {
    vga_manager_set_context(true);
    vga_manager_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_manager_clear();
}