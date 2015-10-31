#include "hid_defines.h"

#ifdef NULL
#undef NULL
#endif
#define NULL (0x00)

KEYBOARD_MEM_TYPE const char keyboardLUT_ES[KEYBOARD_SIZE] =
{
        NULL,                           // 0x00
        NULL,                           // 0x01
        NULL,                           // 0x02
        NULL,                           // 0x03
        NULL,                           // 0x04
        NULL,                           // 0x05
        NULL,                           // 0x06
        NULL,                           // 0x07
        KEY_BACKSPACE,                  // 0x08
        KEY_TAB,                        // 0x09
        KEY_RETURN,                     // 0x0A
        NULL,                           // 0x0B
        NULL,                           // 0x0C
        KEY_RETURN,                     // 0x0D
        NULL,                           // 0x0E
        NULL,                           // 0x0F
        NULL,                           // 0x10
        NULL,                           // 0x11
        NULL,                           // 0x12
        NULL,                           // 0x13
        NULL,                           // 0x14
        NULL,                           // 0x15
        NULL,                           // 0x16
        NULL,                           // 0x17
        NULL,                           // 0x18
        NULL,                           // 0x19
        NULL,                           // 0x1A
        NULL,                           // 0x1B
        NULL,                           // 0x1C
        NULL,                           // 0x1D
        NULL,                           // 0x1E
        NULL,                           // 0x1F
        KEY_SPACE,                      // 0x20
        SHIFT_MASK|KEY_1,               // 0x21 !
        SHIFT_MASK|KEY_2,               // 0x22 "
        ALTGR_MASK|KEY_3,               // 0x23 #
        SHIFT_MASK|KEY_4,               // 0x24 $
        SHIFT_MASK|KEY_5,               // 0x25 %
        SHIFT_MASK|KEY_6,               // 0x26 &
        KEY_MINUS,                      // 0x27 '
        SHIFT_MASK|KEY_8,               // 0x28 (
        SHIFT_MASK|KEY_9,               // 0x29 )
        SHIFT_MASK|KEY_BRACKET_RIGHT,   // 0x2A *
        KEY_BRACKET_RIGHT,              // 0x2B +
        KEY_COMMA,                      // 0x2C ,
        KEY_SLASH,                      // 0x2D -
        KEY_PERIOD,                     // 0x2E .
        SHIFT_MASK|KEY_7,               // 0x2F /
        KEY_0,                          // 0x30 0
        KEY_1,                          // 0x31 1
        KEY_2,                          // 0x32 2
        KEY_3,                          // 0x33 3
        KEY_4,                          // 0x34 4
        KEY_5,                          // 0x35 5
        KEY_6,                          // 0x36 6
        KEY_7,                          // 0x37 7
        KEY_8,                          // 0x38 8
        KEY_9,                          // 0x39 9
        SHIFT_MASK|KEY_PERIOD,          // 0x3A :
        SHIFT_MASK|KEY_COMMA,           // 0x3B ;
        KEY_EUROPE_2,                   // 0x3C <
        SHIFT_MASK|KEY_0,               // 0x3D =
        SHIFT_MASK|KEY_EUROPE_2,        // 0x3E >
        SHIFT_MASK|KEY_MINUS,           // 0x3F ?
        ALTGR_MASK|KEY_2,               // 0x40 @
        SHIFT_MASK|KEY_A,               // 0x41 A
        SHIFT_MASK|KEY_B,               // 0x42 B
        SHIFT_MASK|KEY_C,               // 0x43 C
        SHIFT_MASK|KEY_D,               // 0x44 D
        SHIFT_MASK|KEY_E,               // 0x45 E
        SHIFT_MASK|KEY_F,               // 0x46 F
        SHIFT_MASK|KEY_G,               // 0x47 G
        SHIFT_MASK|KEY_H,               // 0x48 H
        SHIFT_MASK|KEY_I,               // 0x49 I
        SHIFT_MASK|KEY_J,               // 0x4A J
        SHIFT_MASK|KEY_K,               // 0x4B K
        SHIFT_MASK|KEY_L,               // 0x4C L
        SHIFT_MASK|KEY_M,               // 0x4D M
        SHIFT_MASK|KEY_N,               // 0x4E N
        SHIFT_MASK|KEY_O,               // 0x4F O
        SHIFT_MASK|KEY_P,               // 0x50 P
        SHIFT_MASK|KEY_Q,               // 0x51 Q
        SHIFT_MASK|KEY_R,               // 0x52 R
        SHIFT_MASK|KEY_S,               // 0x53 S
        SHIFT_MASK|KEY_T,               // 0x55 T
        SHIFT_MASK|KEY_U,               // 0x55 U
        SHIFT_MASK|KEY_V,               // 0x56 V
        SHIFT_MASK|KEY_W,               // 0x57 W
        SHIFT_MASK|KEY_X,               // 0x58 X
        SHIFT_MASK|KEY_Y,               // 0x59 Y
        SHIFT_MASK|KEY_Z,               // 0x5A Z
        ALTGR_MASK|KEY_BRACKET_LEFT,    // 0x5B [
        ALTGR_MASK|KEY_GRAVE,           // 0x5C '\'
        ALTGR_MASK|KEY_BRACKET_RIGHT,   // 0x5D ]
        SHIFT_MASK|KEY_BRACKET_LEFT,    // 0x5E ^
        SHIFT_MASK|KEY_SLASH,           // 0x5F _
        KEY_BRACKET_LEFT,               // 0x60 `
        KEY_A,                          // 0x61 a
        KEY_B,                          // 0x62 b
        KEY_C,                          // 0x63 c
        KEY_D,                          // 0x66 d
        KEY_E,                          // 0x65 e
        KEY_F,                          // 0x66 f
        KEY_G,                          // 0x67 g
        KEY_H,                          // 0x68 h
        KEY_I,                          // 0x69 i
        KEY_J,                          // 0x6A j
        KEY_K,                          // 0x6B k
        KEY_L,                          // 0x6C l
        KEY_M,                          // 0x6D m
        KEY_N,                          // 0x6E n
        KEY_O,                          // 0x6F o
        KEY_P,                          // 0x70 p
        KEY_Q,                          // 0x71 q
        KEY_R,                          // 0x72 r
        KEY_S,                          // 0x73 s
        KEY_T,                          // 0x75 t
        KEY_U,                          // 0x75 u
        KEY_V,                          // 0x76 v
        KEY_W,                          // 0x77 w
        KEY_X,                          // 0x78 x
        KEY_Y,                          // 0x79 y
        KEY_Z,                          // 0x7A z
        ALTGR_MASK|KEY_APOSTROPHE,      // 0x7B {
        ALTGR_MASK|KEY_1,               // 0x7C |
        ALTGR_MASK|KEY_BACKSLASH,       // 0x7D }
        ALTGR_MASK|KEY_4,               // 0x7E ~
        NULL                            // just to have 128 bytes
};
