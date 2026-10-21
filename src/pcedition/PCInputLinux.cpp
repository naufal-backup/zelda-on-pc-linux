// Zelda on PC - the Linux raw input backend.
//
// evdev: the input devices of the machine are opened directly (/dev/input/event*). That gives the same kind of input
// as Windows Raw Input: the movement of the mouse is the raw movement of the device (no acceleration of the desktop,
// no scaling of the screen), the keyboard gives every key on its own (left and right modifiers apart) and the wheel
// is read as it is.
//
// When the devices cannot be opened (the user is not in the "input" group and no udev rule gives access to them), the
// X11 backend takes over: the pointer events of the game window and the raw keyboard events of the X server. That
// needs the window to be an X11 window, so it only works when the desktop runs X11 (or XWayland with GDK_BACKEND=x11).

#include "pcedition/PCInputBackend.h"

#if !defined(_WIN32)

#include <atomic>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include <linux/input.h>

#include "pcedition/PCVK.h"

#include "Cemu/Logging/CemuLogging.h"
#include "gui/interface/WindowSystem.h"

#ifdef HAS_X11
#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#endif

namespace pce::Input
{
	namespace
	{
		std::thread sThread;
		std::atomic<bool> sQuit{false};
		std::atomic<bool> sStarted{false};

		// ---------------- evdev ----------------
		struct EvDevice
		{
			int fd = -1;
			bool keyboard = false;
			bool mouse = false;
			bool wheel = false;
			std::string name;
		};

		bool HasBit(const unsigned char* bits, int code)
		{
			return (bits[code / 8] >> (code % 8)) & 1;
		}

		bool DeviceHasKey(int fd, int code)
		{
			unsigned char bits[64]{};
			if (ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(bits)), bits) < 0)
				return false;
			return HasBit(bits, code);
		}

		bool DeviceHasRel(int fd, int code)
		{
			unsigned char bits[16]{};
			if (ioctl(fd, EVIOCGBIT(EV_REL, sizeof(bits)), bits) < 0)
				return false;
			return HasBit(bits, code);
		}

		// The virtual key code of a key of the keyboard (the codes of the kernel).
		uint16 LinuxKeyToVK(int code)
		{
			if (code >= KEY_1 && code <= KEY_9)
				return (uint16)('1' + (code - KEY_1));
			if (code == KEY_0)
				return (uint16)'0';
			if (code >= KEY_Q && code <= KEY_P)
				return (uint16)('Q' + (code - KEY_Q));
			if (code >= KEY_A && code <= KEY_Z)
				return (uint16)('A' + (code - KEY_A));
			switch (code)
			{
			case KEY_ESC: return VK_ESCAPE;
			case KEY_ENTER: return VK_RETURN;
			case KEY_KPENTER: return VK_RETURN;
			case KEY_TAB: return VK_TAB;
			case KEY_BACKSPACE: return VK_BACK;
			case KEY_SPACE: return VK_SPACE;
			case KEY_LEFTSHIFT: return VK_LSHIFT;
			case KEY_RIGHTSHIFT: return VK_RSHIFT;
			case KEY_LEFTCTRL: return VK_LCONTROL;
			case KEY_RIGHTCTRL: return VK_RCONTROL;
			case KEY_LEFTALT: return VK_LMENU;
			case KEY_RIGHTALT: return VK_RMENU;
			case KEY_LEFTMETA: return VK_LWIN;
			case KEY_RIGHTMETA: return VK_RWIN;
			case KEY_CAPSLOCK: return VK_CAPITAL;
			case KEY_NUMLOCK: return VK_NUMLOCK;
			case KEY_SCROLLLOCK: return VK_SCROLL;
			case KEY_PAUSE: return VK_PAUSE;
			case KEY_SYSRQ: return VK_SNAPSHOT;
			case KEY_UP: return VK_UP;
			case KEY_DOWN: return VK_DOWN;
			case KEY_LEFT: return VK_LEFT;
			case KEY_RIGHT: return VK_RIGHT;
			case KEY_HOME: return VK_HOME;
			case KEY_END: return VK_END;
			case KEY_PAGEUP: return VK_PRIOR;
			case KEY_PAGEDOWN: return VK_NEXT;
			case KEY_INSERT: return VK_INSERT;
			case KEY_DELETE: return VK_DELETE;
			case KEY_MINUS: return VK_OEM_MINUS;
			case KEY_EQUAL: return VK_OEM_PLUS;
			case KEY_LEFTBRACE: return VK_OEM_4;
			case KEY_RIGHTBRACE: return VK_OEM_6;
			case KEY_SEMICOLON: return VK_OEM_1;
			case KEY_APOSTROPHE: return VK_OEM_7;
			case KEY_GRAVE: return VK_OEM_3;
			case KEY_BACKSLASH: return VK_OEM_5;
			case KEY_COMMA: return VK_OEM_COMMA;
			case KEY_DOT: return VK_OEM_PERIOD;
			case KEY_SLASH: return VK_OEM_2;
			case KEY_KPASTERISK: return VK_MULTIPLY;
			case KEY_KPMINUS: return VK_SUBTRACT;
			case KEY_KPPLUS: return VK_ADD;
			case KEY_KPDOT: return VK_DECIMAL;
			case KEY_KPSLASH: return VK_DIVIDE;
			case KEY_KP7: return VK_NUMPAD7;
			case KEY_KP8: return VK_NUMPAD8;
			case KEY_KP9: return VK_NUMPAD9;
			case KEY_KP4: return VK_NUMPAD4;
			case KEY_KP5: return VK_NUMPAD5;
			case KEY_KP6: return VK_NUMPAD6;
			case KEY_KP1: return VK_NUMPAD1;
			case KEY_KP2: return VK_NUMPAD2;
			case KEY_KP3: return VK_NUMPAD3;
			case KEY_KP0: return VK_NUMPAD0;
			case KEY_F1: return VK_F1;
			case KEY_F2: return VK_F1 + 1;
			case KEY_F3: return VK_F1 + 2;
			case KEY_F4: return VK_F1 + 3;
			case KEY_F5: return VK_F1 + 4;
			case KEY_F6: return VK_F1 + 5;
			case KEY_F7: return VK_F1 + 6;
			case KEY_F8: return VK_F1 + 7;
			case KEY_F9: return VK_F1 + 8;
			case KEY_F10: return VK_F1 + 9;
			case KEY_F11: return VK_F1 + 10;
			case KEY_F12: return VK_F1 + 11;
			default: return 0;
			}
		}

		// The virtual key code of a button of the mouse.
		uint16 LinuxButtonToVK(int code)
		{
			switch (code)
			{
			case BTN_LEFT: return VK_LBUTTON;
			case BTN_RIGHT: return VK_RBUTTON;
			case BTN_MIDDLE: return VK_MBUTTON;
			case BTN_SIDE: return VK_XBUTTON1;
			case BTN_EXTRA: return VK_XBUTTON2;
			default: return 0;
			}
		}

		std::vector<EvDevice> OpenEvDevices()
		{
			std::vector<EvDevice> devices;
			for (int i = 0; i < 64; i++)
			{
				const std::string path = fmt::format("/dev/input/event{}", i);
				const int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
				if (fd < 0)
					continue;
				EvDevice dev;
				dev.fd = fd;
				char name[128]{};
				if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0)
					dev.name = name;
				const bool isKeyboard = DeviceHasKey(fd, KEY_A) && DeviceHasKey(fd, KEY_Z) && DeviceHasKey(fd, KEY_ENTER);
				const bool isMouse = DeviceHasRel(fd, REL_X) && DeviceHasRel(fd, REL_Y) && DeviceHasKey(fd, BTN_LEFT);
				if (!isKeyboard && !isMouse)
				{
					close(fd);
					continue;
				}
				dev.keyboard = isKeyboard;
				dev.mouse = isMouse;
				dev.wheel = isMouse && DeviceHasRel(fd, REL_WHEEL);
				devices.push_back(dev);
			}
			// the mouse with a wheel is the one the player uses (a touchpad usually has none)
			auto better = [](const EvDevice& a, const EvDevice& b) {
				if (a.mouse != b.mouse)
					return a.mouse;
				if (a.wheel != b.wheel)
					return a.wheel;
				return a.name < b.name;
			};
			std::stable_sort(devices.begin(), devices.end(), better);
			return devices;
		}

		void HandleEvEvent(const EvDevice& dev, const input_event& ev)
		{
			if (ev.type == EV_KEY)
			{
				if (ev.value == 2) // the auto repeat of the kernel: the key is already down
					return;
				const bool down = ev.value != 0;
				if (dev.mouse && !dev.keyboard)
				{
					const uint16 vk = LinuxButtonToVK(ev.code);
					if (vk)
						BackendSetMouseButton(vk, down);
				}
				else
				{
					const uint16 vk = LinuxKeyToVK(ev.code);
					if (vk)
						BackendSetKey(vk, down);
				}
			}
			else if (ev.type == EV_REL && dev.mouse)
			{
				if (ev.code == REL_X)
					BackendMouseMove((sint32)ev.value, 0);
				else if (ev.code == REL_Y)
					BackendMouseMove(0, (sint32)ev.value);
				else if (ev.code == REL_WHEEL)
					BackendWheel((sint32)ev.value * 120);
			}
		}

		// ---------------- X11 ----------------
#ifdef HAS_X11
		struct X11State
		{
			Display* display = nullptr;
			Window window = 0;
			Atom netActiveWindow = 0;
		};

		X11State sX11;

		uint16 X11KeycodeToVK(unsigned int keycode)
		{
			if (!sX11.display || keycode == 0)
				return 0;
			KeyCode min = 0, max = 0;
			if (!XDisplayKeycodes(sX11.display, &min, &max) || keycode < min || keycode > max)
				return 0;
			int count = 0;
			KeySym* syms = XGetKeyboardMapping(sX11.display, (KeyCode)keycode, 1, &count);
			if (!syms || count <= 0)
				return 0;
			const KeySym sym = syms[0];
			XFree(syms);
			switch (sym)
			{
			case XK_Escape: return VK_ESCAPE;
			case XK_Return: return VK_RETURN;
			case XK_Tab: return VK_TAB;
			case XK_BackSpace: return VK_BACK;
			case XK_space: return VK_SPACE;
			case XK_Shift_L: return VK_LSHIFT;
			case XK_Shift_R: return VK_RSHIFT;
			case XK_Control_L: return VK_LCONTROL;
			case XK_Control_R: return VK_RCONTROL;
			case XK_Alt_L: return VK_LMENU;
			case XK_Alt_R: return VK_RMENU;
			case XK_Super_L: return VK_LWIN;
			case XK_Super_R: return VK_RWIN;
			case XK_Caps_Lock: return VK_CAPITAL;
			case XK_Num_Lock: return VK_NUMLOCK;
			case XK_Scroll_Lock: return VK_SCROLL;
			case XK_Pause: return VK_PAUSE;
			case XK_Print: return VK_SNAPSHOT;
			case XK_Up: return VK_UP;
			case XK_Down: return VK_DOWN;
			case XK_Left: return VK_LEFT;
			case XK_Right: return VK_RIGHT;
			case XK_Home: return VK_HOME;
			case XK_End: return VK_END;
			case XK_Prior: return VK_PRIOR;
			case XK_Next: return VK_NEXT;
			case XK_Insert: return VK_INSERT;
			case XK_Delete: return VK_DELETE;
			case XK_semicolon: return VK_OEM_1;
			case XK_equal: return VK_OEM_PLUS;
			case XK_comma: return VK_OEM_COMMA;
			case XK_minus: return VK_OEM_MINUS;
			case XK_period: return VK_OEM_PERIOD;
			case XK_slash: return VK_OEM_2;
			case XK_grave: return VK_OEM_3;
			case XK_bracketleft: return VK_OEM_4;
			case XK_backslash: return VK_OEM_5;
			case XK_bracketright: return VK_OEM_6;
			case XK_apostrophe: return VK_OEM_7;
			case XK_KP_Enter: return VK_RETURN;
			case XK_KP_Add: return VK_ADD;
			case XK_KP_Subtract: return VK_SUBTRACT;
			case XK_KP_Multiply: return VK_MULTIPLY;
			case XK_KP_Divide: return VK_DIVIDE;
			case XK_KP_Decimal: return VK_DECIMAL;
			default: break;
			}
			if (sym >= XK_F1 && sym <= XK_F12)
				return (uint16)(VK_F1 + (sym - XK_F1));
			if (sym >= XK_KP_0 && sym <= XK_KP_9)
				return (uint16)(VK_NUMPAD0 + (sym - XK_KP_0));
			if (sym >= XK_a && sym <= XK_z)
				return (uint16)('A' + (sym - XK_a));
			if (sym >= XK_A && sym <= XK_Z)
				return (uint16)('A' + (sym - XK_A));
			if (sym >= XK_0 && sym <= XK_9)
				return (uint16)('0' + (sym - XK_0));
			return 0;
		}

		bool X11IsFocused()
		{
			if (!sX11.display || !sX11.window)
				return false;
			// the window that has the focus (the one the keyboard goes to)
			Window focus = 0;
			int revert = 0;
			XGetInputFocus(sX11.display, &focus, &revert);
			if (focus == sX11.window)
				return true;
			// the window of the game may be a child of the focused window (or the other way round)
			Window root = 0, parent = 0, *children = nullptr;
			unsigned int nChildren = 0;
			Window current = focus;
			for (int i = 0; i < 32 && current != 0; i++)
			{
				if (current == sX11.window)
					return true;
				if (!XQueryTree(sX11.display, current, &root, &parent, &children, &nChildren))
					break;
				if (children)
					XFree(children);
				if (parent == root)
					break;
				current = parent;
			}
			return false;
		}

		bool X11Start()
		{
			const auto& info = WindowSystem::GetWindowInfo().window_main;
			if (info.backend != WindowSystem::WindowHandleInfo::Backend::X11 || !info.display || !info.surface)
				return false;
			// the events are read from a connection of our own: the connection of the GUI belongs to its thread
			sX11.display = XOpenDisplay(nullptr);
			if (!sX11.display)
				return false;
			int major = 0, minor = 0;
			if (XIQueryVersion(sX11.display, &major, &minor) != Success || major < 2)
			{
				cemuLog_log(LogType::Force, "PCInput: XInput2 is not available ({}.{})", major, minor);
				XCloseDisplay(sX11.display);
				sX11.display = nullptr;
				return false;
			}
			Window window = (Window)info.surface;
			// the raw keyboard events are needed: the plain key events do not tell left and right apart
			XIEventMask mask = XI_RawKeyPressMask | XI_RawKeyReleaseMask | XI_RawMotionMask;
			const int rc = XISelectEvents(sX11.display, window, mask, XIAllDevices);
			if (rc != Success)
			{
				cemuLog_log(LogType::Force, "PCInput: XISelectEvents failed ({})", rc);
				XCloseDisplay(sX11.display);
				sX11.display = nullptr;
				return false;
			}
			// the buttons of the mouse and the wheel are plain pointer events of the window
			XISelectEvents(sX11.display, window, XI_ButtonPressMask | XI_ButtonReleaseMask, XIAllDevices);
			sX11.window = window;
			return true;
		}

		void X11Stop()
		{
			if (sX11.display)
			{
				XCloseDisplay(sX11.display);
				sX11.display = nullptr;
			}
			sX11.window = 0;
		}

		void X11HandleEvent(XEvent& ev)
		{
			if (ev.type == GenericEvent)
			{
				XIEvent* xie = (XIEvent*)ev.xgeneric.data;
				if (!xie)
					return;
				if (xie->type == XI_RawKeyPress || xie->type == XI_RawKeyRelease)
				{
					const uint16 vk = X11KeycodeToVK(xie->raw_event.detail);
					if (vk)
						BackendSetKey(vk, xie->type == XI_RawKeyPress);
				}
				else if (xie->type == XI_RawMotion)
				{
					// the motion the server made itself (the pointer was warped) is not a movement of the device
					if (xie->raw_event.flags & XIPointerEmulated)
						return;
					BackendMouseMove((sint32)xie->raw_event.delta_x, (sint32)xie->raw_event.delta_y);
				}
			}
			else if (ev.type == ButtonPress || ev.type == ButtonRelease)
			{
				const bool down = ev.type == ButtonPress;
				const unsigned int button = ev.xbutton.button;
				if (button == Button1) BackendSetMouseButton(VK_LBUTTON, down);
				else if (button == Button2) BackendSetMouseButton(VK_MIDDLE, down);
				else if (button == Button3) BackendSetMouseButton(VK_RBUTTON, down);
				else if (button == Button4) BackendWheel(120);
				else if (button == Button5) BackendWheel(-120);
				else if (button == Button8) BackendSetMouseButton(VK_XBUTTON1, down);
				else if (button == Button9) BackendSetMouseButton(VK_XBUTTON2, down);
			}
		}
#endif

		// ---------------- the thread ----------------
		void InputThread()
		{
			std::vector<EvDevice> devices = OpenEvDevices();
			bool haveDevices = false;
			for (auto& d : devices)
			{
				if (d.keyboard || d.mouse)
				{
					haveDevices = true;
					cemuLog_log(LogType::Force, "PCInput: device '{}' ({}{})", d.name, d.keyboard ? "keyboard " : "", d.mouse ? "mouse" : "");
				}
			}
#ifdef HAS_X11
			const bool useX11 = !haveDevices && X11Start();
			if (!haveDevices && !useX11)
				cemuLog_log(LogType::Force, "PCInput: no input source: the input devices of the machine could not be opened and the "
										   "window is not an X11 window. Add the user to the group 'input' to play.");
#else
			const bool useX11 = false;
			if (!haveDevices)
				cemuLog_log(LogType::Force, "PCInput: the input devices of the machine could not be opened. "
										   "Add the user to the group 'input' to play.");
#endif
			while (!sQuit.load())
			{
				if (haveDevices)
				{
					std::vector<pollfd> fds;
					fds.reserve(devices.size());
					for (auto& d : devices)
						fds.push_back({d.fd, POLLIN, 0});
					const int r = poll(fds.data(), fds.size(), 50);
					if (r > 0)
					{
						for (size_t i = 0; i < devices.size(); i++)
						{
							if (!(fds[i].revents & POLLIN))
								continue;
							input_event ev{};
							while (read(devices[i].fd, &ev, sizeof(ev)) == (ssize_t)sizeof(ev))
								HandleEvEvent(devices[i], ev);
						}
					}
				}
#ifdef HAS_X11
				else if (useX11)
				{
					while (XPending(sX11.display))
					{
						XEvent ev;
						XNextEvent(sX11.display, &ev);
						X11HandleEvent(ev);
					}
					usleep(5000);
				}
#endif
				else
				{
					usleep(50000);
				}
				const bool focused = useX11 ? X11IsFocused() : WindowSystem::GetWindowInfo().app_active;
				if (focused != IsFocused())
				{
					BackendSetFocus(focused);
				}
			}
#ifdef HAS_X11
			if (useX11)
				X11Stop();
#endif
			for (auto& d : devices)
			{
				if (d.fd >= 0)
					close(d.fd);
			}
		}
	}

	namespace Backend
	{
		bool Start()
		{
			if (sStarted.exchange(true))
				return true;
			sQuit = false;
			sThread = std::thread(InputThread);
			return true;
		}

		void Stop()
		{
			if (!sThread.joinable())
				return;
			sQuit = true;
			sThread.join();
			sStarted = false;
		}
	}
}

#endif
