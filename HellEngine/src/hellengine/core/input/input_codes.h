#pragma once

#define USE_GLFW
#if defined(USE_GLFW)
#include <Windows.h>
#include <GLFW/glfw3.h>
#endif

namespace hellengine
{

	namespace core
	{

#if defined USE_GLFW
		enum mouse_buttons {
			MOUSE_BUTTON_LEFT =								GLFW_MOUSE_BUTTON_LEFT,
			MOUSE_BUTTON_RIGHT =							GLFW_MOUSE_BUTTON_RIGHT,
			MOUSE_BUTTON_MIDDLE =							GLFW_MOUSE_BUTTON_MIDDLE,

			MOUSE_BUTTON_LAST =								MOUSE_BUTTON_MIDDLE,

			MAX_MOUSE_BUTTON_COUNT =						MOUSE_BUTTON_LAST
		};

		enum keys {
			KEY_SPACE =										GLFW_KEY_SPACE,
			KEY_APOSTROPHE =								GLFW_KEY_APOSTROPHE,
			KEY_COMMA =										GLFW_KEY_COMMA,
			KEY_MINUS =										GLFW_KEY_MINUS,
			KEY_PERIOD =									GLFW_KEY_PERIOD,
			KEY_SLASH =										GLFW_KEY_SLASH,
			KEY_0 =											GLFW_KEY_0,
			KEY_1 =											GLFW_KEY_1,
			KEY_2 =											GLFW_KEY_2,
			KEY_3 =											GLFW_KEY_3,
			KEY_4 =											GLFW_KEY_4,
			KEY_5 =											GLFW_KEY_5,
			KEY_6 =											GLFW_KEY_6,
			KEY_7 =											GLFW_KEY_7,
			KEY_8 =											GLFW_KEY_8,
			KEY_9 =											GLFW_KEY_9,
			KEY_SEMICOLON =									GLFW_KEY_SEMICOLON,
			KEY_EQUAL =										GLFW_KEY_EQUAL,
			KEY_A =											GLFW_KEY_A,
			KEY_B =											GLFW_KEY_B,
			KEY_C =											GLFW_KEY_C,
			KEY_D =											GLFW_KEY_D,
			KEY_E =											GLFW_KEY_E,
			KEY_F =											GLFW_KEY_F,
			KEY_G =											GLFW_KEY_G,
			KEY_H =											GLFW_KEY_H,
			KEY_I =											GLFW_KEY_I,
			KEY_J =											GLFW_KEY_J,
			KEY_K =											GLFW_KEY_K,
			KEY_L =											GLFW_KEY_L,
			KEY_M =											GLFW_KEY_M,
			KEY_N =											GLFW_KEY_N,
			KEY_O =											GLFW_KEY_O,
			KEY_P =											GLFW_KEY_P,
			KEY_Q =											GLFW_KEY_Q,
			KEY_R =											GLFW_KEY_R,
			KEY_S =											GLFW_KEY_S,
			KEY_T =											GLFW_KEY_T,
			KEY_U =											GLFW_KEY_U,
			KEY_V =											GLFW_KEY_V,
			KEY_W =											GLFW_KEY_W,
			KEY_X =											GLFW_KEY_X,
			KEY_Y =											GLFW_KEY_Y,
			KEY_Z =											GLFW_KEY_Z,
			KEY_LEFT_BRACKET =								GLFW_KEY_LEFT_BRACKET,
			KEY_BACKSLASH =									GLFW_KEY_BACKSLASH,
			KEY_RIGHT_BRACKET =								GLFW_KEY_RIGHT_BRACKET,
			KEY_GRAVE_ACCENT =								GLFW_KEY_GRAVE_ACCENT,
			KEY_WORLD_1 =									GLFW_KEY_WORLD_1,
			KEY_WORLD_2 =									GLFW_KEY_WORLD_2,
			KEY_ESCAPE =									GLFW_KEY_ESCAPE,
			KEY_ENTER =										GLFW_KEY_ENTER,
			KEY_TAB =										GLFW_KEY_TAB,
			KEY_BACKSPACE =									GLFW_KEY_BACKSPACE,
			KEY_INSERT =									GLFW_KEY_INSERT,
			KEY_DELETE =									GLFW_KEY_DELETE,
			KEY_RIGHT =										GLFW_KEY_RIGHT,
			KEY_LEFT =										GLFW_KEY_LEFT,
			KEY_DOWN =										GLFW_KEY_DOWN,
			KEY_UP =										GLFW_KEY_UP,
			KEY_PAGE_UP =									GLFW_KEY_PAGE_UP,
			KEY_PAGE_DOWN =									GLFW_KEY_PAGE_DOWN,
			KEY_HOME =										GLFW_KEY_HOME,
			KEY_END =										GLFW_KEY_END,
			KEY_CAPS_LOCK =									GLFW_KEY_CAPS_LOCK,
			KEY_SCROLL_LOCK =								GLFW_KEY_SCROLL_LOCK,
			KEY_NUM_LOCK =									GLFW_KEY_NUM_LOCK,
			KEY_PRINT_SCREEN =								GLFW_KEY_PRINT_SCREEN,
			KEY_PAUSE =										GLFW_KEY_PAUSE,
			KEY_F1 =										GLFW_KEY_F1,
			KEY_F2 =										GLFW_KEY_F2,
			KEY_F3 =										GLFW_KEY_F3,
			KEY_F4 =										GLFW_KEY_F4,
			KEY_F5 =										GLFW_KEY_F5,
			KEY_F6 =										GLFW_KEY_F6,
			KEY_F7 =										GLFW_KEY_F7,
			KEY_F8 =										GLFW_KEY_F8,
			KEY_F9 =										GLFW_KEY_F9,
			KEY_F10 =										GLFW_KEY_F10,
			KEY_F11 =										GLFW_KEY_F11,
			KEY_F12 =										GLFW_KEY_F12,
			KEY_F13 =										GLFW_KEY_F13,
			KEY_F14 =										GLFW_KEY_F14,
			KEY_F15 =										GLFW_KEY_F15,
			KEY_F16 =										GLFW_KEY_F16,
			KEY_F17 =										GLFW_KEY_F17,
			KEY_F18 =										GLFW_KEY_F18,
			KEY_F19 =										GLFW_KEY_F19,
			KEY_F20 =										GLFW_KEY_F20,
			KEY_F21 =										GLFW_KEY_F21,
			KEY_F22 =										GLFW_KEY_F22,
			KEY_F23 =										GLFW_KEY_F23,
			KEY_F24 =										GLFW_KEY_F24,
			KEY_F25 =										GLFW_KEY_F25,
			KEY_KP_0 =										GLFW_KEY_KP_0,
			KEY_KP_1 =										GLFW_KEY_KP_1,
			KEY_KP_2 =										GLFW_KEY_KP_2,
			KEY_KP_3 =										GLFW_KEY_KP_3,
			KEY_KP_4 =										GLFW_KEY_KP_4,
			KEY_KP_5 =										GLFW_KEY_KP_5,
			KEY_KP_6 =										GLFW_KEY_KP_6,
			KEY_KP_7 =										GLFW_KEY_KP_7,
			KEY_KP_8 =										GLFW_KEY_KP_8,
			KEY_KP_9 =										GLFW_KEY_KP_9,
			KEY_KP_DECIMAL =								GLFW_KEY_KP_DECIMAL,
			KEY_KP_DIVIDE =									GLFW_KEY_KP_DIVIDE,
			KEY_KP_MULTIPLY =								GLFW_KEY_KP_MULTIPLY,
			KEY_KP_SUBTRACT =								GLFW_KEY_KP_SUBTRACT,
			KEY_KP_ADD =									GLFW_KEY_KP_ADD,
			KEY_KP_ENTER =									GLFW_KEY_KP_ENTER,
			KEY_KP_EQUAL =									GLFW_KEY_KP_EQUAL,
			KEY_LEFT_SHIFT =								GLFW_KEY_LEFT_SHIFT,
			KEY_LEFT_CONTROL =								GLFW_KEY_LEFT_CONTROL,
			KEY_LEFT_ALT =									GLFW_KEY_LEFT_ALT,
			KEY_LEFT_SUPER =								GLFW_KEY_LEFT_SUPER,
			KEY_RIGHT_SHIFT =								GLFW_KEY_RIGHT_SHIFT,
			KEY_RIGHT_CONTROL =								GLFW_KEY_RIGHT_CONTROL,
			KEY_RIGHT_ALT =									GLFW_KEY_RIGHT_ALT,
			KEY_RIGHT_SUPER =								GLFW_KEY_RIGHT_SUPER,
			KEY_MENU =										GLFW_KEY_MENU,

			KEY_LAST =										KEY_MENU,

			MAX_KEY_COUNT =									KEY_LAST
		};

		enum mods {
			MOD_KEY_SHIFT =									GLFW_MOD_SHIFT,
			MOD_KEY_CONTROL =								GLFW_MOD_CONTROL,
			MOD_KEY_ALT =									GLFW_MOD_ALT,
			MOD_KEY_SUPER =									GLFW_MOD_SUPER,
			MOD_KEY_CAPS_LOCK =								GLFW_MOD_CAPS_LOCK,
			MOD_KEY_NUM_LOCK =								GLFW_MOD_NUM_LOCK
		};
#else
		enum mouse_buttons {
			MOUSE_BUTTON_LEFT =								0x01,
			MOUSE_BUTTON_RIGHT =							0x02,
			MOUSE_BUTTON_MIDDLE =							0x04,

			MOUSE_BUTTON_LAST =								MOUSE_BUTTON_MIDDLE,

			MAX_MOUSE_BUTTON_COUNT =						MOUSE_BUTTON_LAST
		};

		enum keys {
			// Arrow keys
			KEY_LEFT = 0x25,
			KEY_UP = 0x26,
			KEY_RIGHT = 0x27,
			KEY_DOWN = 0x28,

			// Number keys
			KEY_0 = 0x30,
			KEY_1 = 0x31,
			KEY_2 = 0x32,
			KEY_3 = 0x33,
			KEY_4 = 0x34,
			KEY_5 = 0x35,
			KEY_6 = 0x36,
			KEY_7 = 0x37,
			KEY_8 = 0x38,
			KEY_9 = 0x39,

			// Alphabet keys
			KEY_A = 0x41,
			KEY_B = 0x42,
			KEY_C = 0x43,
			KEY_D = 0x44,
			KEY_E = 0x45,
			KEY_F = 0x46,
			KEY_G = 0x47,
			KEY_H = 0x48,
			KEY_I = 0x49,
			KEY_J = 0x4A,
			KEY_K = 0x4B,
			KEY_L = 0x4C,
			KEY_M = 0x4D,
			KEY_N = 0x4E,
			KEY_O = 0x4F,
			KEY_P = 0x50,
			KEY_Q = 0x51,
			KEY_R = 0x52,
			KEY_S = 0x53,
			KEY_T = 0x54,
			KEY_U = 0x55,
			KEY_V = 0x56,
			KEY_W = 0x57,
			KEY_X = 0x58,
			KEY_Y = 0x59,
			KEY_Z = 0x5A,

			// Numpad keys
			KEY_NUMPAD_0 = 0x60,
			KEY_NUMPAD_1 = 0x61,
			KEY_NUMPAD_2 = 0x62,
			KEY_NUMPAD_3 = 0x63,
			KEY_NUMPAD_4 = 0x64,
			KEY_NUMPAD_5 = 0x65,
			KEY_NUMPAD_6 = 0x66,
			KEY_NUMPAD_7 = 0x67,
			KEY_NUMPAD_8 = 0x68,
			KEY_NUMPAD_9 = 0x69,
			KEY_NUMPAD_ADD = 0x6B,
			KEY_NUMPAD_SUBTRACT = 0x6D,
			KEY_NUMPAD_MULTIPLY = 0x6A,
			KEY_NUMPAD_DIVIDE = 0x6F,
			KEY_NUMPAD_DECIMAL = 0x6E,
			KEY_NUMPAD_SEPARATOR = 0x6C,

			// Function keys
			KEY_F1 = 0x70,
			KEY_F2 = 0x71,
			KEY_F3 = 0x72,
			KEY_F4 = 0x73,
			KEY_F5 = 0x74,
			KEY_F6 = 0x75,
			KEY_F7 = 0x76,
			KEY_F8 = 0x77,
			KEY_F9 = 0x78,
			KEY_F10 = 0x79,
			KEY_F11 = 0x7A,
			KEY_F12 = 0x7B,

			// Special keys
			KEY_BACKSPACE = 0x08,
			KEY_TAB = 0x09,
			KEY_ENTER = 0x0D,
			KEY_SHIFT = 0x10,
			KEY_CTRL = 0x11,
			KEY_ALT = 0x12,
			KEY_PAUSE = 0x13,
			KEY_CAPS_LOCK = 0x14,
			KEY_ESCAPE = 0x1B,
			KEY_SPACE = 0x20,
			KEY_PAGE_UP = 0x21,
			KEY_PAGE_DOWN = 0x22,
			KEY_END = 0x23,
			KEY_HOME = 0x24,
			KEY_PRINT_SCREEN = 0x2C,
			KEY_INSERT = 0x2D,
			KEY_DELETE = 0x2E,
			KEY_MENU = 0x5D,
			KEY_APPS = 0x5D,
			KEY_SCROLL_LOCK = 0x91,
			KEY_NUM_LOCK = 0x90,

			// Symbols
			KEY_SEMICOLON = 0xBA,
			KEY_EQUALS = 0xBB,
			KEY_COMMA = 0xBC,
			KEY_MINUS = 0xBD,
			KEY_PERIOD = 0xBE,
			KEY_SLASH = 0xBF,
			KEY_TILDE = 0xC0,
			KEY_LBRACKET = 0xDB,
			KEY_BACKSLASH = 0xDC,
			KEY_RBRACKET = 0xDD,
			KEY_APOSTROPHE = 0xDE,

			KEY_LAST = KEY_APOSTROPHE,

			MAX_KEY_COUNT = KEY_LAST
		};
#endif

	} // namespace core

} // namespace hellengine