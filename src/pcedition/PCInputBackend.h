#pragma once

// Zelda on PC - the raw input backend of the PC edition layer.
//
// The core of the input layer (PCInput.cpp) does not know how the input is read; it only knows that keys go down and
// up, that the mouse moves and that its buttons and the wheel are used. The backend delivers that.
//
// Windows: Windows Raw Input (a hidden message window receives WM_INPUT).
// Linux: evdev (the input devices of the machine, raw and unaccelerated) and, when the devices cannot be opened,
//        X11 (XInput2 raw events plus the pointer events of the window).

#include "pcedition/PCKeys.h"

namespace pce::Input
{
	// A key goes down or up (virtual key code).
	void BackendSetKey(uint16 vk, bool down);
	// A movement of the mouse (the raw delta of the device).
	void BackendMouseMove(sint32 dx, sint32 dy);
	// A mouse button (VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2).
	void BackendSetMouseButton(uint16 vk, bool down);
	// The wheel (the number of notches, positive is up).
	void BackendWheel(sint32 notches);
	// The game window got or lost the input focus.
	void BackendSetFocus(bool focused);
	// The focus state of the game window.
	bool IsFocused();
	// All keys are released (the window lost the focus).
	void ReleaseAllKeys();
	// The native handle of the main window (the backend needs it to know which window has the focus).
	void* GetMainWindowHandle();

	namespace Backend
	{
		// Starts the input thread. False when no input source could be opened.
		bool Start();
		// Stops the input thread.
		void Stop();
	}
}
