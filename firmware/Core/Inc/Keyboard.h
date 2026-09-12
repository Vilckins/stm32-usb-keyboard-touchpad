/*
  Keyboard.h

  Copyright (c) 2015, Arduino LLC
  Original code (pre-library): Copyright (c) 2011, Peter Barrett

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

//================================================================================
//================================================================================
//  Keyboard
// #define KEY_FN          0x8F   // give Fn key a HID code
#define KEY_ESC 0x29 // ESC key
#define KEY_BACKSPACE 0x2A // BACKSPACE key
#define KEY_TAB 0x2B
#define KEY_ENTER 0x28
#define KEY_SHIFT_LEFT 0x02 // modificator
#define KEY_SHIFT_RIGHT 0x20 // modificator
#define KEY_CTRL_LEFT 0x01 // modificator
#define KEY_CTRL_RIGHT 0x10 // modificator
#define KEY_ALT_LEFT 0x04 // modificator
#define KEY_ALT_RIGHT 0x40 // modificator
#define KEY_WIN 0x08 // modificator
#define KEY_CAPSLOCK 0x39
#define KEY_NUMLOCK 0x53
#define KEY_END 0x4D
#define KEY_HOME 0x4A
#define KEY_SPACE 0x2C
#define KEY_PAGEUP 0x4B
#define KEY_PAGEDOWN 0x4E
#define KEY_LEFT 0x50
#define KEY_UP 0x52
#define KEY_RIGHT 0x4F
#define KEY_DOWN 0x51
#define KEY_PRTSCR 0x46
#define KEY_INSERT 0x49
#define KEY_DELETE 0x4C
#define KEY_0 0x27
#define KEY_1 0x1E
#define KEY_2 0x1F
#define KEY_3 0x20
#define KEY_4 0x21
#define KEY_5 0x22
#define KEY_6 0x23
#define KEY_7 0x24
#define KEY_8 0x25
#define KEY_9 0x26
#define KEY_A 0x04
#define KEY_B 0x05
#define KEY_C 0x06
#define KEY_D 0x07
#define KEY_E 0x08
#define KEY_F 0x09
#define KEY_G 0x0A
#define KEY_H 0x0B
#define KEY_I 0x0C
#define KEY_J 0x0D
#define KEY_K 0x0E
#define KEY_L 0x0F
#define KEY_M 0x10
#define KEY_N 0x11
#define KEY_O 0x12
#define KEY_P 0x13
#define KEY_Q 0x14
#define KEY_R 0x15
#define KEY_S 0x16
#define KEY_T 0x17
#define KEY_U 0x18
#define KEY_V 0x19
#define KEY_W 0x1A
#define KEY_X 0x1B
#define KEY_Y 0x1C
#define KEY_Z 0x1D
#define KEY_MENU 0x65
#define KEY_F1 0x3A
#define KEY_F2 0x3B
#define KEY_F3 0x3C
#define KEY_F4 0x3D
#define KEY_F5 0x3E
#define KEY_F6 0x3F
#define KEY_F7 0x40
#define KEY_F8 0x41
#define KEY_F9 0x42
#define KEY_F10 0x43
#define KEY_F11 0x44
#define KEY_F12 0x45
#define KEY_SEMICOLON 0x33 // Точка с запятой
#define KEY_MINUS 0x2D
#define KEY_EQUAL 0x2E
#define KEY_COMMA 0x36 // Запятая
#define KEY_DECIMAL 0x37 // Точка
#define KEY_SLASH 0x38 // Слеш "/"
#define KEY_TILDA 0x35
#define KEY_BRACKET_LEFT 0x2F
#define KEY_BRACKET_RIGHT 0x30
#define KEY_QUOTE 0x34 // ' (одинарная кавычка)
#define KEY_BACKSLASH 0x31 // обратный слеш

#define KEY_FN 0x00 // give Fn key a HID code

extern volatile uint8_t fn_pressed;

extern uint8_t current_modifiers;
extern uint8_t last_modifiers;

uint8_t keyboard_scan(uint8_t* keys, uint8_t max_keys);
uint8_t keyboard_has_changed(uint8_t* current, uint8_t* previous, uint8_t size);
void keyboard_build_report(uint8_t* keys, uint8_t count, uint8_t* report);

#endif
