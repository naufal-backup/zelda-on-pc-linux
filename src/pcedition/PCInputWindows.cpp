// Zelda on PC - the Windows raw input backend.
//
// A hidden message window receives Windows Raw Input (WM_INPUT): the mouse deltas, its buttons, the wheel and the
// keyboard. The keyboard's raw input gives the virtual key code of every key (left and right modifiers apart), the
// mouse gives the unaccelerated movement of the device.

#include "pcedition/PCInputBackend.h"

#ifdef _WIN32

#include <windows.h>

#include "pcedition/PCVK.h"

#include "Cemu/Logging/CemuLogging.h"

namespace pce::Input
{
	namespace
	{
		std::thread sThread;
		std::atomic<bool> sQuit{false};
		std::atomic<HWND> sMsgWnd{nullptr};
		std::atomic<DWORD> sThreadId{0};

		void LogFocusChange(bool focused)
		{
			HWND fg = GetForegroundWindow();
			wchar_t cls[128]{}, title[128]{};
			if (fg)
			{
				GetClassNameW(fg, cls, 127);
				GetWindowTextW(fg, title, 127);
			}
			DWORD pid = 0;
			if (fg)
				GetWindowThreadProcessId(fg, &pid);
			cemuLog_log(LogType::Force, "PCE input: focus {} (foreground hwnd={:x} pid={} class='{}' title='{}')", focused ? "gained" : "lost",
						(uintptr_t)fg, pid, boost::nowide::narrow(cls), boost::nowide::narrow(title));
		}

		bool ComputeFocus()
		{
			HWND main = (HWND)GetMainWindowHandle();
			if (!main)
				return false;
			HWND fg = GetForegroundWindow();
			if (!fg)
				return false;
			return fg == main || IsChild(main, fg) || GetAncestor(fg, GA_ROOTOWNER) == main;
		}

		// the focus state of the game window (updated when it changes)
		bool EnsureFocus()
		{
			const bool focused = ComputeFocus();
			if (focused != IsFocused())
			{
				LogFocusChange(focused);
				BackendSetFocus(focused);
			}
			return focused;
		}

		void HandleRawInput(HRAWINPUT hri)
		{
			alignas(8) BYTE buffer[256];
			UINT size = sizeof(buffer);
			if (GetRawInputData(hri, RID_INPUT, buffer, &size, sizeof(RAWINPUTHEADER)) == (UINT)-1)
				return;
			const RAWINPUT* ri = (const RAWINPUT*)buffer;
			if (!EnsureFocus())
				return;
			if (ri->header.dwType == RIM_TYPEMOUSE)
			{
				const RAWMOUSE& m = ri->data.mouse;
				if ((m.usFlags & MOUSE_MOVE_ABSOLUTE) == 0 && (m.lLastX != 0 || m.lLastY != 0))
					BackendMouseMove((sint32)m.lLastX, (sint32)m.lLastY);
				const USHORT f = m.usButtonFlags;
				if (f & RI_MOUSE_LEFT_BUTTON_DOWN) BackendSetMouseButton(VK_LBUTTON, true);
				if (f & RI_MOUSE_LEFT_BUTTON_UP) BackendSetMouseButton(VK_LBUTTON, false);
				if (f & RI_MOUSE_RIGHT_BUTTON_DOWN) BackendSetMouseButton(VK_RBUTTON, true);
				if (f & RI_MOUSE_RIGHT_BUTTON_UP) BackendSetMouseButton(VK_RBUTTON, false);
				if (f & RI_MOUSE_MIDDLE_BUTTON_DOWN) BackendSetMouseButton(VK_MBUTTON, true);
				if (f & RI_MOUSE_MIDDLE_BUTTON_UP) BackendSetMouseButton(VK_MBUTTON, false);
				if (f & RI_MOUSE_BUTTON_4_DOWN) BackendSetMouseButton(VK_XBUTTON1, true);
				if (f & RI_MOUSE_BUTTON_4_UP) BackendSetMouseButton(VK_XBUTTON1, false);
				if (f & RI_MOUSE_BUTTON_5_DOWN) BackendSetMouseButton(VK_XBUTTON2, true);
				if (f & RI_MOUSE_BUTTON_5_UP) BackendSetMouseButton(VK_XBUTTON2, false);
				if (f & RI_MOUSE_WHEEL)
					BackendWheel((sint32)(SHORT)m.usButtonData);
			}
			else if (ri->header.dwType == RIM_TYPEKEYBOARD)
			{
				const RAWKEYBOARD& kb = ri->data.keyboard;
				USHORT vk = kb.VKey;
				if (vk == 0 || vk >= 255)
					return;
				const bool down = (kb.Flags & RI_KEY_BREAK) == 0;
				const bool e0 = (kb.Flags & RI_KEY_E0) != 0;
				switch (vk)
				{
				case VK_SHIFT: vk = (kb.MakeCode == 0x36) ? VK_RSHIFT : VK_LSHIFT; break;
				case VK_CONTROL: vk = e0 ? VK_RCONTROL : VK_LCONTROL; break;
				case VK_MENU: vk = e0 ? VK_RMENU : VK_LMENU; break;
				default: break;
				}
				BackendSetKey(vk, down);
			}
		}

		LRESULT CALLBACK MsgWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
		{
			if (msg == WM_INPUT)
				HandleRawInput((HRAWINPUT)lp);
			return DefWindowProcW(hwnd, msg, wp, lp);
		}

		void InputThread()
		{
			SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
			sThreadId = GetCurrentThreadId();
			WNDCLASSEXW wc{};
			wc.cbSize = sizeof(wc);
			wc.lpfnWndProc = MsgWndProc;
			wc.hInstance = GetModuleHandleW(nullptr);
			wc.lpszClassName = L"BotWPCInput";
			RegisterClassExW(&wc);
			HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, wc.hInstance, nullptr);
			sMsgWnd = hwnd;
			RAWINPUTDEVICE rid[2]{};
			rid[0].usUsagePage = 0x01;
			rid[0].usUsage = 0x02; // mouse
			rid[0].dwFlags = RIDEV_INPUTSINK;
			rid[0].hwndTarget = hwnd;
			rid[1].usUsagePage = 0x01;
			rid[1].usUsage = 0x06; // keyboard
			rid[1].dwFlags = RIDEV_INPUTSINK;
			rid[1].hwndTarget = hwnd;
			if (!RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE)))
				cemuLog_log(LogType::Force, "PCInput: RegisterRawInputDevices failed ({})", GetLastError());
			MSG msg;
			while (!sQuit.load())
			{
				// wake up periodically to refresh focus state even without input
				MsgWaitForMultipleObjects(0, nullptr, FALSE, 50, QS_ALLINPUT);
				while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
				{
					if (msg.message == WM_QUIT)
						sQuit = true;
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
				const bool focused = EnsureFocus();
				(void)focused;
			}
			rid[0].dwFlags = RIDEV_REMOVE;
			rid[0].hwndTarget = nullptr;
			rid[1].dwFlags = RIDEV_REMOVE;
			rid[1].hwndTarget = nullptr;
			RegisterRawInputDevices(rid, 2, sizeof(RAWINPUTDEVICE));
			DestroyWindow(hwnd);
			sMsgWnd = nullptr;
		}
	}

	namespace Backend
	{
		bool Start()
		{
			sQuit = false;
			sThread = std::thread(InputThread);
			return true;
		}

		void Stop()
		{
			if (!sThread.joinable())
				return;
			sQuit = true;
			if (DWORD tid = sThreadId.load())
				PostThreadMessageW(tid, WM_QUIT, 0, 0);
			sThread.join();
		}
	}
}

#endif
