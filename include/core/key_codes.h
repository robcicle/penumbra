#pragma once

#undef DELETE

namespace penumbra
{
	using KeyCode = uint32_t;

	namespace Key
	{
		enum : KeyCode
		{
			// Alphanumeric keys (A-Z, 0-9)
			A = 'A',
			B = 'B',
			C = 'C',
			D = 'D',
			E = 'E',
			F = 'F',
			G = 'G',
			H = 'H',
			I = 'I',
			J = 'J',
			K = 'K',
			L = 'L',
			M = 'M',
			N = 'N',
			O = 'O',
			P = 'P',
			Q = 'Q',
			R = 'R',
			S = 'S',
			T = 'T',
			U = 'U',
			V = 'V',
			W = 'W',
			X = 'X',
			Y = 'Y',
			Z = 'Z',

			D0 = '0',
			D1 = '1',
			D2 = '2',
			D3 = '3',
			D4 = '4',
			D5 = '5',
			D6 = '6',
			D7 = '7',
			D8 = '8',
			D9 = '9',

			// Function keys
			F1 = VK_F1,
			F2 = VK_F2,
			F3 = VK_F3,
			F4 = VK_F4,
			F5 = VK_F5,
			F6 = VK_F6,
			F7 = VK_F7,
			F8 = VK_F8,
			F9 = VK_F9,
			F10 = VK_F10,
			F11 = VK_F11,
			F12 = VK_F12,

			// Control keys
			ESCAPE = VK_ESCAPE,
			ENTER = VK_RETURN,
			TAB = VK_TAB,
			BACKSPACE = VK_BACK,
			INSERT = VK_INSERT,
			DELETE = VK_DELETE,
			HOME = VK_HOME,
			END = VK_END,
			PAGE_UP = VK_PRIOR,
			PAGE_DOWN = VK_NEXT,
			PAUSE = VK_PAUSE,
			PRINT_SCREEN = VK_SNAPSHOT,

			// Modifiers
			LEFT_SHIFT = VK_LSHIFT,
			RIGHT_SHIFT = VK_RSHIFT,
			LEFT_CONTROL = VK_LCONTROL,
			RIGHT_CONTROL = VK_RCONTROL,
			LEFT_ALT = VK_LMENU,
			RIGHT_ALT = VK_RMENU,
			CAPS_LOCK = VK_CAPITAL,
			NUM_LOCK = VK_NUMLOCK,
			SCROLL_LOCK = VK_SCROLL,

			// Navigation
			LEFT = VK_LEFT,
			RIGHT = VK_RIGHT,
			UP = VK_UP,
			DOWN = VK_DOWN,

			// Symbols / Punctuation
			SPACE = VK_SPACE,
			SEMICOLON = VK_OEM_1,         // ';:' key
			EQUAL = VK_OEM_PLUS,          // '=' key
			COMMA = VK_OEM_COMMA,         // ',' key
			MINUS = VK_OEM_MINUS,         // '-' key
			PERIOD = VK_OEM_PERIOD,       // '.' key
			SLASH = VK_OEM_2,             // '/?' key
			GRAVE_ACCENT = VK_OEM_3,      // '`~' key
			LEFT_BRACKET = VK_OEM_4,      // '[{' key
			BACKSLASH = VK_OEM_5,         // '\|' key
			RIGHT_BRACKET = VK_OEM_6,     // ']}' key
			APOSTROPHE = VK_OEM_7,        // ''"' key

			// Keypad
			KP0 = VK_NUMPAD0,
			KP1 = VK_NUMPAD1,
			KP2 = VK_NUMPAD2,
			KP3 = VK_NUMPAD3,
			KP4 = VK_NUMPAD4,
			KP5 = VK_NUMPAD5,
			KP6 = VK_NUMPAD6,
			KP7 = VK_NUMPAD7,
			KP8 = VK_NUMPAD8,
			KP9 = VK_NUMPAD9,
			KP_DECIMAL = VK_DECIMAL,
			KP_DIVIDE = VK_DIVIDE,
			KP_MULTIPLY = VK_MULTIPLY,
			KP_SUBTRACT = VK_SUBTRACT,
			KP_ADD = VK_ADD,
			KP_ENTER = VK_RETURN, // same as enter on most keyboards
			KP_EQUAL = VK_OEM_PLUS,

			// System
			MENU = VK_MENU // same as ALT
		};
	}
}