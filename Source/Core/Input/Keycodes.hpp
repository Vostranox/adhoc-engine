#pragma once
#include <cstdint>
#include <string_view>

using Keycode = std::uint64_t;

#define ADH_KEY_COUNT 512
#if defined(ADH_WINDOWS)
#    define ADH_KEY_A 0x41
#    define ADH_KEY_B 0x42
#    define ADH_KEY_C 0x43
#    define ADH_KEY_D 0x44
#    define ADH_KEY_E 0x45
#    define ADH_KEY_F 0x46
#    define ADH_KEY_G 0x47
#    define ADH_KEY_H 0x48
#    define ADH_KEY_I 0x49
#    define ADH_KEY_J 0x4A
#    define ADH_KEY_K 0x4B
#    define ADH_KEY_L 0x4C
#    define ADH_KEY_M 0x4D
#    define ADH_KEY_N 0x4E
#    define ADH_KEY_O 0x4F
#    define ADH_KEY_P 0x50
#    define ADH_KEY_Q 0x51
#    define ADH_KEY_R 0x52
#    define ADH_KEY_S 0x53
#    define ADH_KEY_T 0x54
#    define ADH_KEY_U 0x55
#    define ADH_KEY_V 0x56
#    define ADH_KEY_W 0x57
#    define ADH_KEY_X 0x58
#    define ADH_KEY_Y 0x59
#    define ADH_KEY_Z 0x5A

#    define ADH_KEY_0 0x30
#    define ADH_KEY_1 0x31
#    define ADH_KEY_2 0x32
#    define ADH_KEY_3 0x33
#    define ADH_KEY_4 0x34
#    define ADH_KEY_5 0x35
#    define ADH_KEY_6 0x36
#    define ADH_KEY_7 0x37
#    define ADH_KEY_8 0x38
#    define ADH_KEY_9 0x39

#    define ADH_F1 0x70
#    define ADH_F2 0x71
#    define ADH_F3 0x72
#    define ADH_F4 0x73
#    define ADH_F5 0x74
#    define ADH_F6 0x75
#    define ADH_F7 0x76
#    define ADH_F8 0x77
#    define ADH_F9 0x78
#    define ADH_F10 0x79
#    define ADH_F11 0x7A
#    define ADH_F12 0x7B
#    define ADH_F13 0x7C
#    define ADH_F14 0x7D
#    define ADH_F15 0x7E
#    define ADH_F16 0x7F
#    define ADH_F17 0x80
#    define ADH_F18 0x81
#    define ADH_F19 0x82
#    define ADH_F20 0x83
#    define ADH_F21 0x84
#    define ADH_F22 0x85
#    define ADH_F23 0x86
#    define ADH_F24 0x87

#    define ADH_NUMPAD0 0x60
#    define ADH_NUMPAD1 0x61
#    define ADH_NUMPAD2 0x62
#    define ADH_NUMPAD3 0x63
#    define ADH_NUMPAD4 0x64
#    define ADH_NUMPAD5 0x65
#    define ADH_NUMPAD6 0x66
#    define ADH_NUMPAD7 0x67
#    define ADH_NUMPAD8 0x68
#    define ADH_NUMPAD9 0x69

#    define ADH_SPACE 0x20
#    define ADH_RETURN 0x0D
#    define ADH_TAB 0x09
#    define ADH_ESCAPE 0x1B
#    define ADH_BACK 0x08
#    define ADH_INSERT 0x2D
#    define ADH_DELETE 0x2E
#    define ADH_HOME 0x24
#    define ADH_END 0x23
#    define ADH_PRIOR 0x21
#    define ADH_NEXT 0x22
#    define ADH_LEFT 0x25
#    define ADH_UP 0x26
#    define ADH_RIGHT 0x27
#    define ADH_DOWN 0x28

#    define ADH_PAUSE 0x13
#    define ADH_CAPITAL 0x14
#    define ADH_SNAPSHOT 0x2C
#    define ADH_APPS 0x5D

#    define ADH_SHIFT 0xA0
#    define ADH_RSHIFT 0xA1
#    define ADH_CONTROL 0xA2
#    define ADH_RCONTROL 0xA3
#    define ADH_MENU 0xA4
#    define ADH_RMENU 0xA5
#    define ADH_SUPER 0x5B
#    define ADH_RSUPER 0x5C

#    define ADH_MULTIPLY 0x6A
#    define ADH_ADD 0x6B
#    define ADH_SUBTRACT 0x6D
#    define ADH_DECIMAL 0x6E
#    define ADH_DIVIDE 0x6F
#    define ADH_NUMPAD_ENTER 0x100
#    define ADH_NUMLOCK 0x90
#    define ADH_SCROLLLOCK 0x91

#    define ADH_APOSTROPHE 0xDE
#    define ADH_COMMA 0xBC
#    define ADH_MINUS 0xBD
#    define ADH_PERIOD 0xBE
#    define ADH_SLASH 0xBF
#    define ADH_SEMICOLON 0xBA
#    define ADH_EQUAL 0xBB
#    define ADH_LBRACKET 0xDB
#    define ADH_BACKSLASH 0xDC
#    define ADH_RBRACKET 0xDD
#    define ADH_GRAVE 0xC0
#elif defined(ADH_APPLE)
#    include "AppleKeycodes.hpp"
#    define ADH_KEY_A kVK_ANSI_A
#    define ADH_KEY_B kVK_ANSI_B
#    define ADH_KEY_C kVK_ANSI_C
#    define ADH_KEY_D kVK_ANSI_D
#    define ADH_KEY_E kVK_ANSI_E
#    define ADH_KEY_F kVK_ANSI_F
#    define ADH_KEY_G kVK_ANSI_G
#    define ADH_KEY_H kVK_ANSI_H
#    define ADH_KEY_I kVK_ANSI_I
#    define ADH_KEY_J kVK_ANSI_J
#    define ADH_KEY_K kVK_ANSI_K
#    define ADH_KEY_L kVK_ANSI_L
#    define ADH_KEY_M kVK_ANSI_M
#    define ADH_KEY_N kVK_ANSI_N
#    define ADH_KEY_O kVK_ANSI_O
#    define ADH_KEY_P kVK_ANSI_P
#    define ADH_KEY_Q kVK_ANSI_Q
#    define ADH_KEY_R kVK_ANSI_R
#    define ADH_KEY_S kVK_ANSI_S
#    define ADH_KEY_T kVK_ANSI_T
#    define ADH_KEY_U kVK_ANSI_U
#    define ADH_KEY_V kVK_ANSI_V
#    define ADH_KEY_W kVK_ANSI_W
#    define ADH_KEY_X kVK_ANSI_X
#    define ADH_KEY_Y kVK_ANSI_Y
#    define ADH_KEY_Z kVK_ANSI_Z

#    define ADH_KEY_0 kVK_ANSI_0
#    define ADH_KEY_1 kVK_ANSI_1
#    define ADH_KEY_2 kVK_ANSI_2
#    define ADH_KEY_3 kVK_ANSI_3
#    define ADH_KEY_4 kVK_ANSI_4
#    define ADH_KEY_5 kVK_ANSI_5
#    define ADH_KEY_6 kVK_ANSI_6
#    define ADH_KEY_7 kVK_ANSI_7
#    define ADH_KEY_8 kVK_ANSI_8
#    define ADH_KEY_9 kVK_ANSI_9

#    define ADH_F1 kVK_F1
#    define ADH_F2 kVK_F2
#    define ADH_F3 kVK_F3
#    define ADH_F4 kVK_F4
#    define ADH_F5 kVK_F5
#    define ADH_F6 kVK_F6
#    define ADH_F7 kVK_F7
#    define ADH_F8 kVK_F8
#    define ADH_F9 kVK_F9
#    define ADH_F10 kVK_F10
#    define ADH_F11 kVK_F11
#    define ADH_F12 kVK_F12
#    define ADH_F13 kVK_F13
#    define ADH_F14 kVK_F14
#    define ADH_F15 kVK_F15
#    define ADH_F16 kVK_F16
#    define ADH_F17 kVK_F17
#    define ADH_F18 kVK_F18
#    define ADH_F19 kVK_F19
#    define ADH_F20 kVK_F20
#    define ADH_F21 277
#    define ADH_F22 278
#    define ADH_F23 279
#    define ADH_F24 280

#    define ADH_NUMPAD0 kVK_ANSI_Keypad0
#    define ADH_NUMPAD1 kVK_ANSI_Keypad1
#    define ADH_NUMPAD2 kVK_ANSI_Keypad2
#    define ADH_NUMPAD3 kVK_ANSI_Keypad3
#    define ADH_NUMPAD4 kVK_ANSI_Keypad4
#    define ADH_NUMPAD5 kVK_ANSI_Keypad5
#    define ADH_NUMPAD6 kVK_ANSI_Keypad6
#    define ADH_NUMPAD7 kVK_ANSI_Keypad7
#    define ADH_NUMPAD8 kVK_ANSI_Keypad8
#    define ADH_NUMPAD9 kVK_ANSI_Keypad9

#    define ADH_SPACE kVK_Space
#    define ADH_RETURN kVK_Return
#    define ADH_TAB kVK_Tab
#    define ADH_ESCAPE kVK_Escape
#    define ADH_BACK kVK_Delete
#    define ADH_INSERT kVK_Help
#    define ADH_DELETE kVK_ForwardDelete
#    define ADH_HOME kVK_Home
#    define ADH_END kVK_End
#    define ADH_PRIOR kVK_PageUp
#    define ADH_NEXT kVK_PageDown
#    define ADH_LEFT kVK_LeftArrow
#    define ADH_UP kVK_UpArrow
#    define ADH_RIGHT kVK_RightArrow
#    define ADH_DOWN kVK_DownArrow

#    define ADH_PAUSE 281
#    define ADH_CAPITAL kVK_CapsLock
#    define ADH_SNAPSHOT 282
#    define ADH_APPS 283

#    define ADH_SHIFT kVK_Shift
#    define ADH_RSHIFT kVK_RightShift
#    define ADH_CONTROL kVK_Control
#    define ADH_RCONTROL kVK_RightControl
#    define ADH_MENU kVK_Option
#    define ADH_RMENU kVK_RightOption
#    define ADH_SUPER kVK_Command
#    define ADH_RSUPER 54

#    define ADH_MULTIPLY kVK_ANSI_KeypadMultiply
#    define ADH_ADD kVK_ANSI_KeypadPlus
#    define ADH_SUBTRACT kVK_ANSI_KeypadMinus
#    define ADH_DECIMAL kVK_ANSI_KeypadDecimal
#    define ADH_DIVIDE kVK_ANSI_KeypadDivide
#    define ADH_NUMPAD_ENTER kVK_ANSI_KeypadEnter
#    define ADH_NUMLOCK kVK_ANSI_KeypadClear
#    define ADH_SCROLLLOCK 284

#    define ADH_APOSTROPHE kVK_ANSI_Quote
#    define ADH_COMMA kVK_ANSI_Comma
#    define ADH_MINUS kVK_ANSI_Minus
#    define ADH_PERIOD kVK_ANSI_Period
#    define ADH_SLASH kVK_ANSI_Slash
#    define ADH_SEMICOLON kVK_ANSI_Semicolon
#    define ADH_EQUAL kVK_ANSI_Equal
#    define ADH_LBRACKET kVK_ANSI_LeftBracket
#    define ADH_BACKSLASH kVK_ANSI_Backslash
#    define ADH_RBRACKET kVK_ANSI_RightBracket
#    define ADH_GRAVE kVK_ANSI_Grave
#elif defined(ADH_LINUX)
#    define ADH_KEY_Q 24
#    define ADH_KEY_W 25
#    define ADH_KEY_E 26
#    define ADH_KEY_R 27
#    define ADH_KEY_T 28
#    define ADH_KEY_Y 29
#    define ADH_KEY_U 30
#    define ADH_KEY_I 31
#    define ADH_KEY_O 32
#    define ADH_KEY_P 33

#    define ADH_KEY_A 38
#    define ADH_KEY_S 39
#    define ADH_KEY_D 40
#    define ADH_KEY_F 41
#    define ADH_KEY_G 42
#    define ADH_KEY_H 43
#    define ADH_KEY_J 44
#    define ADH_KEY_K 45
#    define ADH_KEY_L 46

#    define ADH_KEY_Z 52
#    define ADH_KEY_X 53
#    define ADH_KEY_C 54
#    define ADH_KEY_V 55
#    define ADH_KEY_B 56
#    define ADH_KEY_N 57
#    define ADH_KEY_M 58

#    define ADH_KEY_1 10
#    define ADH_KEY_2 11
#    define ADH_KEY_3 12
#    define ADH_KEY_4 13
#    define ADH_KEY_5 14
#    define ADH_KEY_6 15
#    define ADH_KEY_7 16
#    define ADH_KEY_8 17
#    define ADH_KEY_9 18
#    define ADH_KEY_0 19

#    define ADH_F1 67
#    define ADH_F2 68
#    define ADH_F3 69
#    define ADH_F4 70
#    define ADH_F5 71
#    define ADH_F6 72
#    define ADH_F7 73
#    define ADH_F8 74
#    define ADH_F9 75
#    define ADH_F10 76
#    define ADH_F11 95
#    define ADH_F12 96
#    define ADH_F13 191
#    define ADH_F14 192
#    define ADH_F15 193
#    define ADH_F16 194
#    define ADH_F17 195
#    define ADH_F18 196
#    define ADH_F19 197
#    define ADH_F20 198
#    define ADH_F21 199
#    define ADH_F22 200
#    define ADH_F23 201
#    define ADH_F24 202

#    define ADH_NUMPAD0 90
#    define ADH_NUMPAD1 87
#    define ADH_NUMPAD2 88
#    define ADH_NUMPAD3 89
#    define ADH_NUMPAD4 83
#    define ADH_NUMPAD5 84
#    define ADH_NUMPAD6 85
#    define ADH_NUMPAD7 79
#    define ADH_NUMPAD8 80
#    define ADH_NUMPAD9 81

#    define ADH_SPACE 65
#    define ADH_RETURN 36
#    define ADH_TAB 23
#    define ADH_ESCAPE 9
#    define ADH_BACK 22
#    define ADH_INSERT 118
#    define ADH_DELETE 119
#    define ADH_HOME 110
#    define ADH_END 115
#    define ADH_PRIOR 112
#    define ADH_NEXT 117
#    define ADH_LEFT 113
#    define ADH_UP 111
#    define ADH_RIGHT 114
#    define ADH_DOWN 116

#    define ADH_PAUSE 127
#    define ADH_CAPITAL 66
#    define ADH_SNAPSHOT 107
#    define ADH_APPS 135

#    define ADH_SHIFT 50
#    define ADH_RSHIFT 62
#    define ADH_CONTROL 37
#    define ADH_RCONTROL 105
#    define ADH_MENU 64
#    define ADH_RMENU 108
#    define ADH_SUPER 133
#    define ADH_RSUPER 134

#    define ADH_MULTIPLY 63
#    define ADH_ADD 86
#    define ADH_SUBTRACT 82
#    define ADH_DECIMAL 91
#    define ADH_DIVIDE 106
#    define ADH_NUMPAD_ENTER 104
#    define ADH_NUMLOCK 77
#    define ADH_SCROLLLOCK 78

#    define ADH_APOSTROPHE 48
#    define ADH_COMMA 59
#    define ADH_MINUS 20
#    define ADH_PERIOD 60
#    define ADH_SLASH 61
#    define ADH_SEMICOLON 47
#    define ADH_EQUAL 21
#    define ADH_LBRACKET 34
#    define ADH_BACKSLASH 51
#    define ADH_RBRACKET 35
#    define ADH_GRAVE 49
#endif

#define ADH_BUTTON_A 0
#define ADH_BUTTON_B 1
#define ADH_BUTTON_X 2
#define ADH_BUTTON_Y 3
#define ADH_BUTTON_BACK 4
#define ADH_BUTTON_START 5
#define ADH_BUTTON_LTHUMB_PRESS 6
#define ADH_BUTTON_RTHUMB_PRESS 7
#define ADH_BUTTON_LSHOULDER 8
#define ADH_BUTTON_RSHOULDER 9
#define ADH_BUTTON_DPAD_UP 10
#define ADH_BUTTON_DPAD_DOWN 11
#define ADH_BUTTON_DPAD_LEFT 12
#define ADH_BUTTON_DPAD_RIGHT 13
#define ADH_BUTTON_LTRIGGER 14
#define ADH_BUTTON_RTRIGGER 15
#define ADH_BUTTON_COUNT 16

namespace adh {
    struct KeyName {
        std::string_view name;
        Keycode code;
    };

    inline constexpr KeyName keyNames[]{
        { "a", ADH_KEY_A },
        { "b", ADH_KEY_B },
        { "c", ADH_KEY_C },
        { "d", ADH_KEY_D },
        { "e", ADH_KEY_E },
        { "f", ADH_KEY_F },
        { "g", ADH_KEY_G },
        { "h", ADH_KEY_H },
        { "i", ADH_KEY_I },
        { "j", ADH_KEY_J },
        { "k", ADH_KEY_K },
        { "l", ADH_KEY_L },
        { "m", ADH_KEY_M },
        { "n", ADH_KEY_N },
        { "o", ADH_KEY_O },
        { "p", ADH_KEY_P },
        { "q", ADH_KEY_Q },
        { "r", ADH_KEY_R },
        { "s", ADH_KEY_S },
        { "t", ADH_KEY_T },
        { "u", ADH_KEY_U },
        { "v", ADH_KEY_V },
        { "w", ADH_KEY_W },
        { "x", ADH_KEY_X },
        { "y", ADH_KEY_Y },
        { "z", ADH_KEY_Z },
        { "enter", ADH_RETURN },
        { "space", ADH_SPACE },
        { "shift", ADH_SHIFT },
        { "rshift", ADH_RSHIFT },
        { "control", ADH_CONTROL },
        { "rcontrol", ADH_RCONTROL },
        { "command", ADH_SUPER },
        { "option", ADH_MENU },
        { "left", ADH_LEFT },
        { "up", ADH_UP },
        { "right", ADH_RIGHT },
        { "down", ADH_DOWN }
    };

    inline constexpr KeyName buttonNames[]{
        { "a", ADH_BUTTON_A },
        { "b", ADH_BUTTON_B },
        { "x", ADH_BUTTON_X },
        { "y", ADH_BUTTON_Y },
        { "rshoulder", ADH_BUTTON_RSHOULDER },
        { "lshoulder", ADH_BUTTON_LSHOULDER },
        { "ltrigger", ADH_BUTTON_LTRIGGER },
        { "rtrigger", ADH_BUTTON_RTRIGGER },
        { "dpad_up", ADH_BUTTON_DPAD_UP },
        { "dpad_down", ADH_BUTTON_DPAD_DOWN },
        { "dpad_left", ADH_BUTTON_DPAD_LEFT },
        { "dpad_right", ADH_BUTTON_DPAD_RIGHT },
        { "start", ADH_BUTTON_START },
        { "back", ADH_BUTTON_BACK },
        { "lthumb_press", ADH_BUTTON_LTHUMB_PRESS },
        { "rthumb_press", ADH_BUTTON_RTHUMB_PRESS }
    };
} // namespace adh
