#pragma once

// Zelda on PC - the few things the PC edition layer needs from the operating system.
//
// Everything that is not plain C++ lives here, so that the rest of the layer stays portable: the Windows build uses
// the Win32 API, the Linux build uses the X11 / evdev interfaces of the system.

#include <cstdint>
#include <string>

namespace pce::Platform
{
	// The folder where the user's own Cemu installation keeps its settings (settings.xml, keys.txt).
	// Windows: %APPDATA%. Linux: the XDG config folder (~/.config), where Cemu keeps its "Cemu" folder.
	fs::path GetUserConfigDir();

	// The folder where the user's own Cemu installation keeps its data (mlc01 with the saves).
	// Windows: %APPDATA%. Linux: the XDG data folder (~/.local/share).
	fs::path GetUserDataDir();

	// A modal error message: the game cannot start.
	void ShowFatalError(const std::string& message);

	// A monotonic clock: ticks of MonotonicFrequency() units since an arbitrary point in time.
	sint64 MonotonicTicks();
	double MonotonicFrequency();

	// Opens a folder with the file manager of the desktop.
	void OpenFolder(const fs::path& folder);

	// True when the user can write into the folder.
	bool IsDirectoryWritable(const fs::path& folder);
}
