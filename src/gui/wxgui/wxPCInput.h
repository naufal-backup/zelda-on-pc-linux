#pragma once

// Zelda on PC - the input of the GUI layer (Linux).
//
// The events of the window of the game are forwarded to the PC edition layer when the raw input of the machine
// (evdev) is not available. See wxPCInput.cpp.

#include <cstdint>

class wxKeyEvent;
class wxMouseEvent;

namespace pce::Gui
{
	void OnKey(wxKeyEvent& event, bool down);
	void OnMouseButton(wxMouseEvent& event, bool down);
	void OnWheel(wxMouseEvent& event);
	void OnMotion(wxMouseEvent& event);
}
