#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

// Keyboard controller ports
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_CMD_PORT     0x64

// Keyboard commands
#define KEYBOARD_CMD_READ_CCB 0x20
#define KEYBOARD_CMD_WRITE_CCB 0x60
#define KEYBOARD_CMD_SELF_TEST 0xAA
#define KEYBOARD_CMD_TEST_CONTROLLER 0xAB
#define KEYBOARD_CMD_DISABLE_PORT2 0xA7
#define KEYBOARD_CMD_ENABLE_PORT2 0xA8
#define KEYBOARD_CMD_TEST_PORT2 0xA9
#define KEYBOARD_CMD_READ_OUTPUT 0xD0
#define KEYBOARD_CMD_WRITE_OUTPUT 0xD1

// Controller configuration byte bits
#define CCB_INTERRUPT_PORT1   (1 << 0)
#define CCB_INTERRUPT_PORT2   (1 << 1)
#define CCB_SYSTEM_FLAG       (1 << 2)
#define CCB_ZERO1             (1 << 3)
#define CCB_IGNORE_LOCK       (1 << 4)
#define CCB_PORT1_CLOCK       (1 << 5)
#define CCB_PORT2_CLOCK       (1 << 6)
#define CCB_PORT1_TRANSLATION (1 << 7)

// Status register bits
#define STATUS_OUTPUT_FULL    (1 << 0)
#define STATUS_INPUT_FULL     (1 << 1)
#define STATUS_SYSTEM         (1 << 2)
#define STATUS_COMMAND_DATA   (1 << 3)
#define STATUS_TIMEOUT_ERROR  (1 << 6)
#define STATUS_PARITY_ERROR   (1 << 7)

// Scancode set 1 (XT) keycodes (partial list)
enum keycode {
    KEY_UNKNOWN = 0,
    KEY_ESCAPE = 0x01,
    KEY_1 = 0x02,
    KEY_2 = 0x03,
    KEY_3 = 0x04,
    KEY_4 = 0x05,
    KEY_5 = 0x06,
    KEY_6 = 0x07,
    KEY_7 = 0x08,
    KEY_8 = 0x09,
    KEY_9 = 0x0A,
    KEY_0 = 0x0B,
    KEY_MINUS = 0x0C,
    KEY_EQUALS = 0x0D,
    KEY_BACKSPACE = 0x0E,
    KEY_TAB = 0x0F,
    KEY_Q = 0x10,
    KEY_W = 0x11,
    KEY_E = 0x12,
    KEY_R = 0x13,
    KEY_T = 0x14,
    KEY_Y = 0x15,
    KEY_U = 0x16,
    KEY_I = 0x17,
    KEY_O = 0x18,
    KEY_P = 0x19,
    KEY_LEFTBRACKET = 0x1A,
    KEY_RIGHTBRACKET = 0x1B,
    KEY_ENTER = 0x1C,
    KEY_LEFT_CTRL = 0x1D,
    KEY_A = 0x1E,
    KEY_S = 0x1F,
    KEY_D = 0x20,
    KEY_F = 0x21,
    KEY_G = 0x22,
    KEY_H = 0x23,
    KEY_J = 0x24,
    KEY_K = 0x25,
    KEY_L = 0x26,
    KEY_SEMICOLON = 0x27,
    KEY_APOSTROPHE = 0x28,
    KEY_BACKTICK = 0x29,
    KEY_LEFT_SHIFT = 0x2A,
    KEY_BACKSLASH = 0x2B,
    KEY_Z = 0x2C,
    KEY_X = 0x2D,
    KEY_C = 0x2E,
    KEY_V = 0x2F,
    KEY_B = 0x30,
    KEY_N = 0x31,
    KEY_M = 0x32,
    KEY_COMMA = 0x33,
    KEY_PERIOD = 0x34,
    KEY_SLASH = 0x35,
    KEY_RIGHT_SHIFT = 0x36,
    KEY_KEYPAD_MULTIPLY = 0x37,
    KEY_LEFT_ALT = 0x38,
    KEY_SPACE = 0x39,
    KEY_CAPS_LOCK = 0x3A,
    KEY_F1 = 0x3B,
    KEY_F2 = 0x3C,
    KEY_F3 = 0x3D,
    KEY_F4 = 0x3E,
    KEY_F5 = 0x3F,
    KEY_F6 = 0x40,
    KEY_F7 = 0x41,
    KEY_F8 = 0x42,
    KEY_F9 = 0x43,
    KEY_F10 = 0x44,
    KEY_NUM_LOCK = 0x45,
    KEY_SCROLL_LOCK = 0x46,
    KEY_HOME = 0x47,
    KEY_UP = 0x48,
    KEY_PAGE_UP = 0x49,
    KEY_KEYPAD_MINUS = 0x4A,
    KEY_LEFT = 0x4B,
    KEY_KEYPAD_5 = 0x4C,
    KEY_RIGHT = 0x4D,
    KEY_KEYPAD_PLUS = 0x4E,
    KEY_END = 0x4F,
    KEY_DOWN = 0x50,
    KEY_PAGE_DOWN = 0x51,
    KEY_INSERT = 0x52,
    KEY_DELETE = 0x53,
    KEY_F11 = 0x57,
    KEY_F12 = 0x58,
    
    // Special keys (not in scancode set 1, but useful for our purposes)
    KEY_LEFT_CTRL_PRESSED = 0x1D00,
    KEY_LEFT_SHIFT_PRESSED = 0x2A00,
    KEY_RIGHT_SHIFT_PRESSED = 0x3600,
    KEY_LEFT_ALT_PRESSED = 0x3800,
    KEY_LEFT_CTRL_RELEASED = 0x9D00,
    KEY_LEFT_SHIFT_RELEASED = 0xAA00,
    KEY_RIGHT_SHIFT_RELEASED = 0xB600,
    KEY_LEFT_ALT_RELEASED = 0xB800,
};

// Modifier keys
enum {
    MOD_NONE = 0,
    MOD_CTRL = (1 << 0),
    MOD_SHIFT = (1 << 1),
    MOD_ALT = (1 << 2),
    MOD_CAPS = (1 << 3),
    MOD_NUM = (1 << 4),
    MOD_SCROLL = (1 << 5)
};

// Function prototypes
void keyboard_init(void);
void keyboard_install(void);
void keyboard_handler(void);
char keyboard_getchar(void);
uint16_t keyboard_get_scancode(void);
bool keyboard_is_key_pressed(uint8_t scancode);
char keyboard_scancode_to_ascii(uint8_t scancode);

#endif // KEYBOARD_H
