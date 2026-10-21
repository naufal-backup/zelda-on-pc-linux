// Zelda on PC - the input of the GUI layer (Linux).
//
// The raw input of the machine (evdev) is the best source: the movement of the mouse is the movement of the device,
// without the acceleration of the desktop. It needs access to the input devices of the machine (the group "input" or
// the rule of udev).
//
// When the devices cannot be opened, the events of the GUI layer are used instead: the keyboard and the mouse of the
// window of the game. The movement of the mouse is then the movement of the pointer of the desktop (with the
// acceleration of the desktop), everything else is the same.

#include "wxgui/wxPCInput.h"

#if !BOOST_OS_WINDOWS

#include <wx/window.h>

#include <gdk/gdkkeysyms.h>

#include "pcedition/PCInput.h"
#include "pcedition/PCVK.h"

namespace pce::Gui
{
	namespace
	{
		// The virtual key code of a key of the keyboard (the key codes of the GDK are the ones of the X server).
		uint16 GdkKeyToVK(int keyval)
		{
			switch (keyval)
			{
			case GDK_KEY_Escape: return VK_ESCAPE;
			case GDK_KEY_Return: return VK_RETURN;
			case GDK_KEY_KP_Enter: return VK_RETURN;
			case GDK_KEY_Tab: return VK_TAB;
			case GDK_KEY_BackSpace: return VK_BACK;
			case GDK_KEY_space: return VK_SPACE;
			case GDK_KEY_Shift_L: return VK_LSHIFT;
			case GDK_KEY_Shift_R: return VK_RSHIFT;
			case GDK_KEY_Control_L: return VK_LCONTROL;
			case GDK_KEY_Control_R: return VK_RCONTROL;
			case GDK_KEY_Alt_L: return VK_LMENU;
			case GDK_KEY_Alt_R: return VK_RMENU;
			case GDK_KEY_Super_L: return VK_LWIN;
			case GDK_KEY_Super_R: return VK_RWIN;
			case GDK_KEY_Caps_Lock: return VK_CAPITAL;
			case GDK_KEY_Num_Lock: return VK_NUMLOCK;
			case GDK_KEY_Scroll_Lock: return VK_SCROLL;
			case GDK_KEY_Pause: return VK_PAUSE;
			case GDK_KEY_Print: return VK_SNAPSHOT;
			case GDK_KEY_Up: return VK_UP;
			case GDK_KEY_Down: return VK_DOWN;
			case GDK_KEY_Left: return VK_LEFT;
			case GDK_KEY_Right: return VK_RIGHT;
			case GDK_KEY_Home: return VK_HOME;
			case GDK_KEY_End: return VK_END;
			case GDK_KEY_Prior: return VK_PRIOR;
			case GDK_KEY_Next: return VK_NEXT;
			case GDK_KEY_Insert: return VK_INSERT;
	case GDK_KEY_Delete: return VK_DELETE;
			case GDK_KEY_semicolon: return VK_OEM_1;
			case GDK_KEY_equal: return VK_OEM_PLUS;
			case GDK_KEY_comma: return VK_OEM_COMMA;
			case GDK_KEY_minus: return VK_OEM_MINUS;
			case GDK_KEY_period: return VK_OEM_PERIOD;
			case GDK_KEY_slash: return VK_OEM_2;
			case GDK_KEY_grave: return VK_OEM_3;
			case GDK_KEY_bracketleft: return VK_OEM_4;
			case GDK_KEY_backslash: return VK_OEM_5;
			case GDK_KEY_bracketright: return VK_OEM_6;
			case GDK_KEY_apostrophe: return VK_OEM_7;
			case GDK_KEY_KP_Add: return VK_ADD;
			case GDK_KEY_KP_Subtract: return VK_SUBTRACT;
			case GDK_KEY_KP_Multiply: return VK_MULTIPLY;
			case GDK_KEY_KP_Divide: return VK_DIVIDE;
			case GDK_KEY_KP_Decimal: return VK_DECIMAL;
			default: break;
			}
			if (keyval >= GDK_KEY_a && keyval <= GDK_KEY_z)
				return (uint16)('A' + (keyval - GDK_KEY_a));
			if (keyval >= GDK_KEY_A && keyval <= GDK_KEY_Z)
				return (uint16)('A' + (keyval - GDK_KEY_A));
			if (keyval >= GDK_KEY_0 && keyval <= GDK_KEY_9)
				return (uint16)('0' + (keyval - GDK_KEY_0));
			if (keyval >= GDK_KEY_F1 && keyval <= GDK_KEY_F12)
				return (uint16)(VK_F1 + (keyval - GDK_KEY_F1));
			if (keyval >= GDK_KEY_KP_0 && keyval <= GDK_KEY_KP_9)
				return (uint16)(VK_NUMPAD0 + (keyval - GDK_KEY_KP_0));
			return 0;
		}
	}

	void OnKey(wxKeyEvent& event, bool down)
	{
		if (Input::HasRawInput())
			return;
		const uint16 vk = GdkKeyToVK(event.GetRawKeyCode());
		if (vk)
			Input::BackendSetKey(vk, down);
	}

	void OnMouseButton(wxMouseEvent& event, bool down)
	{
		if (Input::HasRawInput())
			return;
		uint16 vk = 0;
		switch (event.GetButton())
		{
		case wxMOUSE_BTN_LEFT: vk = VK_LBUTTON; break;
		case wxMOUSE_BTN_RIGHT: vk = VK_RBUTTON; break;
		case wxMOUSE_BTN_MIDDLE: vk = VK_MBUTTON; break;
		case wxMOUSE_BTN_DOUBLE_CLICK: vk = VK_LBUTTON; break;
		case wxMOUSE_BTN_AUX1: vk = VK_XBUTTON1; break;
		case wxMOUSE_BTN_AUX2: vk = VK_XBUTTON2; break;
		default: break;
		}
		if (vk)
			Input::BackendSetMouseButton(vk, down);
	}

	void OnWheel(wxMouseEvent& event)
	{
		if (Input::HasRawInput())
			return;
		Input::BackendWheel(event.GetWheelRotation());
	}

	void OnMotion(wxMouseEvent& event)
	{
		if (Input::HasRawInput())
			return;
		// the movement of the pointer of the desktop (the camera of the game uses the movement of the device itself
		// when the raw input of the machine is available)
		static int lastX = 0, lastY = 0;
		static bool haveLast = false;
		const int x = event.GetX();
		const int y = event.GetY();
		if (!haveLast)
		{
			lastX = x;
			lastY = y;
			haveLast = true;
			return;
		}
		const sint32 dx = (sint32)x - (sint32)lastX;
		const sint32 dy = (sint32)y - (sint32)lastY;
		lastX = x;
		lastY = y;
		if (dx || dy)
			Input::BackendMouseMove(dx, dy);
	}
}

#endif
