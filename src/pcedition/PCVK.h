#pragma once

// Zelda on PC - virtual key codes.
//
// The PC edition layer identifies keys by their Windows virtual key code (the VK_ constants). The codes are part of
// the settings.ini format and of the key bindings, so they are kept on every platform: a settings.ini written on
// Windows keeps working on Linux and the other way round.
//
// On Windows the values come from the system header. On the other platforms they are defined here (the values are
// fixed by Microsoft and never change).

#ifdef _WIN32
#include <windows.h>
#else

#include <cstdint>

// The value of a wheel notch (the wheel of a device that reports finer steps uses a fraction of it).
constexpr int WHEEL_DELTA = 120;

constexpr uint16 VK_LBUTTON = 0x01;
constexpr uint16 VK_RBUTTON = 0x02;
constexpr uint16 VK_CANCEL = 0x03;
constexpr uint16 VK_MBUTTON = 0x04;
constexpr uint16 VK_XBUTTON1 = 0x05;
constexpr uint16 VK_XBUTTON2 = 0x06;
constexpr uint16 VK_BACK = 0x08;
constexpr uint16 VK_TAB = 0x09;
constexpr uint16 VK_CLEAR = 0x0C;
constexpr uint16 VK_RETURN = 0x0D;
constexpr uint16 VK_SHIFT = 0x10;
constexpr uint16 VK_CONTROL = 0x11;
constexpr uint16 VK_MENU = 0x12;
constexpr uint16 VK_PAUSE = 0x13;
constexpr uint16 VK_CAPITAL = 0x14;
constexpr uint16 VK_ESCAPE = 0x1B;
constexpr uint16 VK_SPACE = 0x20;
constexpr uint16 VK_PRIOR = 0x21;
constexpr uint16 VK_NEXT = 0x22;
constexpr uint16 VK_END = 0x23;
constexpr uint16 VK_HOME = 0x24;
constexpr uint16 VK_LEFT = 0x25;
constexpr uint16 VK_UP = 0x26;
constexpr uint16 VK_RIGHT = 0x27;
constexpr uint16 VK_DOWN = 0x28;
constexpr uint16 VK_SELECT = 0x29;
constexpr uint16 VK_PRINT = 0x2A;
constexpr uint16 VK_EXECUTE = 0x2B;
constexpr uint16 VK_SNAPSHOT = 0x2C;
constexpr uint16 VK_INSERT = 0x2D;
constexpr uint16 VK_DELETE = 0x2E;
constexpr uint16 VK_HELP = 0x2F;
constexpr uint16 VK_LWIN = 0x5B;
constexpr uint16 VK_RWIN = 0x5C;
constexpr uint16 VK_APPS = 0x5D;
constexpr uint16 VK_NUMPAD0 = 0x60;
constexpr uint16 VK_NUMPAD1 = 0x61;
constexpr uint16 VK_NUMPAD2 = 0x62;
constexpr uint16 VK_NUMPAD3 = 0x63;
constexpr uint16 VK_NUMPAD4 = 0x64;
constexpr uint16 VK_NUMPAD5 = 0x65;
constexpr uint16 VK_NUMPAD6 = 0x66;
constexpr uint16 VK_NUMPAD7 = 0x67;
constexpr uint16 VK_NUMPAD8 = 0x68;
constexpr uint16 VK_NUMPAD9 = 0x69;
constexpr uint16 VK_MULTIPLY = 0x6A;
constexpr uint16 VK_ADD = 0x6B;
constexpr uint16 VK_SEPARATOR = 0x6C;
constexpr uint16 VK_SUBTRACT = 0x6D;
constexpr uint16 VK_DECIMAL = 0x6E;
constexpr uint16 VK_DIVIDE = 0x6F;
constexpr uint16 VK_F1 = 0x70;
constexpr uint16 VK_F12 = 0x7B;
constexpr uint16 VK_NUMLOCK = 0x90;
constexpr uint16 VK_SCROLL = 0x91;
constexpr uint16 VK_LSHIFT = 0xA0;
constexpr uint16 VK_RSHIFT = 0xA1;
constexpr uint16 VK_LCONTROL = 0xA2;
constexpr uint16 VK_RCONTROL = 0xA3;
constexpr uint16 VK_LMENU = 0xA4;
constexpr uint16 VK_RMENU = 0xA5;
constexpr uint16 VK_OEM_1 = 0xBA;
constexpr uint16 VK_OEM_PLUS = 0xBB;
constexpr uint16 VK_OEM_COMMA = 0xBC;
constexpr uint16 VK_OEM_MINUS = 0xBD;
constexpr uint16 VK_OEM_PERIOD = 0xBE;
constexpr uint16 VK_OEM_2 = 0xBF;
constexpr uint16 VK_OEM_3 = 0xC0;
constexpr uint16 VK_OEM_4 = 0xDB;
constexpr uint16 VK_OEM_5 = 0xDC;
constexpr uint16 VK_OEM_6 = 0xDD;
constexpr uint16 VK_OEM_7 = 0xDE;

#endif
