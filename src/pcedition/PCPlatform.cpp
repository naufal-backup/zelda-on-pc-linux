#include "pcedition/PCPlatform.h"

#include "pcedition/PCEdition.h"

#include "gui/interface/WindowSystem.h"

#include "Cemu/Logging/CemuLogging.h"
#include "util/helpers/helpers.h"

#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#else
#endif

namespace pce::Platform
{
	namespace
	{
#ifdef _WIN32
		fs::path KnownFolder(const GUID& id)
		{
			PWSTR path = nullptr;
			fs::path r;
			if (SUCCEEDED(SHGetKnownFolderPath(id, 0, nullptr, &path)))
				r = path;
			CoTaskMemFree(path);
			return r;
		}
#else
		fs::path XdgDir(const char* envName, const char* fallback)
		{
			if (const char* v = std::getenv(envName); v && *v)
				return fs::path(v);
			if (const char* home = std::getenv("HOME"); home && *home)
				return fs::path(home) / fallback;
			return fs::current_path();
		}
#endif
	}

	fs::path GetUserConfigDir()
	{
#ifdef _WIN32
		return KnownFolder(FOLDERID_RoamingAppData);
#else
		return XdgDir("XDG_CONFIG_HOME", ".config");
#endif
	}

	fs::path GetUserDataDir()
	{
#ifdef _WIN32
		return KnownFolder(FOLDERID_RoamingAppData);
#else
		return XdgDir("XDG_DATA_HOME", ".local/share");
#endif
	}

	void ShowFatalError(const std::string& message)
	{
		cemuLog_log(LogType::Force, "PCE: {}", message);
		// the GUI is up when this is called (the game cannot start)
		WindowSystem::ShowErrorDialog(message, kGameTitle);
	}

	sint64 MonotonicTicks()
	{
#ifdef _WIN32
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		return li.QuadPart;
#else
		return (sint64)std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
#endif
	}

	double MonotonicFrequency()
	{
#ifdef _WIN32
		static double f = []() {
			LARGE_INTEGER li;
			QueryPerformanceFrequency(&li);
			return (double)li.QuadPart;
		}();
		return f;
#else
		return 1000000000.0; // nanoseconds
#endif
	}

	void OpenFolder(const fs::path& folder)
	{
#ifdef _WIN32
		ShellExecuteW(nullptr, L"open", folder.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
		std::error_code ec;
		const std::string path = _pathToUtf8(folder);
		// the file manager of the desktop
		if (std::system(("xdg-open '" + path + "' >/dev/null 2>&1 &").c_str()) != 0)
			cemuLog_log(LogType::Force, "PCE: xdg-open failed for {}", path);
#endif
	}
}
