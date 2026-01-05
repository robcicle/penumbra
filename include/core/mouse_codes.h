#pragma once

namespace penumbra
{
	using MouseCode = uint16_t;

	namespace Mouse
	{
		enum : MouseCode
		{
            BUTTON_LEFT = VK_LBUTTON, // 0x01
            BUTTON_RIGHT = VK_RBUTTON, // 0x02
            BUTTON_MIDDLE = VK_MBUTTON, // 0x04
            BUTTON_X1 = VK_XBUTTON1, // 0x05
            BUTTON_X2 = VK_XBUTTON2, // 0x06

            BUTTON_ONE = BUTTON_LEFT,
            BUTTON_TWO = BUTTON_RIGHT,
            BUTTON_THREE = BUTTON_MIDDLE,
            BUTTON_FOUR = BUTTON_X1,
            BUTTON_FIVE = BUTTON_X2,

            BUTTON_BACKWARD = BUTTON_X1,
            BUTTON_FORWARD = BUTTON_X2
		};
	}
}