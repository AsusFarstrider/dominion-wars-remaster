#include <atomic>
#include <string>

#include <Windows.h>
#include <Common/Hook.h>
#include <Common/Log.h>
#include <Gdi/DominionWarsFocus.h>
#include <Gdi/DominionWarsFocusPolicy.h>
#include <Gdi/WinProc.h>

// Private NA 1.04 experiment only. All state except the presentation hold is
// owned by the game window's thread. No worker thread changes display modes.
namespace
{
	constexpr UINT_PTR TIMER_ID = 0x44574647;
	std::atomic<bool> g_hold = false;
	bool g_initialized = false;
	bool g_enabled = false;
	bool g_suspended = false;
	bool g_transition = false;
	bool g_timer = false;
	ULONGLONG g_lossTime = 0;
	std::wstring g_device;
	std::wstring g_identity;
	DEVMODEW g_original = {};
	DEVMODEW g_temporary = {};

	std::wstring environment(const wchar_t* name)
	{
		wchar_t value[2048] = {};
		const auto count = GetEnvironmentVariableW(name, value, ARRAYSIZE(value));
		return count && count < ARRAYSIZE(value) ? value : L"";
	}

	bool sameDisplay()
	{
		for (DWORD i = 0; ; ++i)
		{
			DISPLAY_DEVICEW adapter = {};
			adapter.cb = sizeof(adapter);
			if (!CALL_ORIG_FUNC(EnumDisplayDevicesW)(nullptr, i, &adapter, 0)) { return false; }
			if (g_device != adapter.DeviceName) { continue; }
			if ((adapter.StateFlags & (DISPLAY_DEVICE_ATTACHED_TO_DESKTOP | DISPLAY_DEVICE_PRIMARY_DEVICE)) !=
				(DISPLAY_DEVICE_ATTACHED_TO_DESKTOP | DISPLAY_DEVICE_PRIMARY_DEVICE)) { return false; }
			DISPLAY_DEVICEW monitor = {};
			monitor.cb = sizeof(monitor);
			if (!CALL_ORIG_FUNC(EnumDisplayDevicesW)(g_device.c_str(), 0, &monitor, EDD_GET_DEVICE_INTERFACE_NAME))
			{
				return false;
			}
			return g_identity == std::wstring(adapter.DeviceID) + L"|" + monitor.DeviceID;
		}
	}

	bool readMode(DEVMODEW& mode)
	{
		mode = {};
		mode.dmSize = sizeof(mode);
		return !!CALL_ORIG_FUNC(EnumDisplaySettingsExW)(g_device.c_str(), ENUM_CURRENT_SETTINGS, &mode, 0);
	}

	void disable(const char* reason)
	{
		g_enabled = false;
		LOG_INFO << "DW focus desktop: switching disabled; " << reason;
	}

	void initialize()
	{
		g_initialized = true;
		if (environment(L"DW_EXPERIMENTAL_FOCUS_DESKTOP") != L"1") { return; }
		g_device = environment(L"DW_FOCUS_DEVICE");
		g_identity = environment(L"DW_FOCUS_DEVICE_IDENTITY");
		if (g_device.empty() || g_identity.empty() ||
			!Gdi::DominionWarsFocusPolicy::decodeMode(environment(L"DW_FOCUS_ORIGINAL_MODE").c_str(), g_original) ||
			!sameDisplay() || !readMode(g_temporary) || g_temporary.dmDriverExtra != 0 ||
			!Gdi::DominionWarsFocusPolicy::expectedGameMode(g_original, g_temporary))
		{
			disable("invalid snapshot, changed monitor, or unexpected initial game desktop");
			return;
		}
		g_enabled = true;
		LOG_INFO << "DW focus desktop: armed; original=" << g_original.dmPelsWidth << "x" <<
			g_original.dmPelsHeight << "@" << g_original.dmDisplayFrequency << "; game=" <<
			g_temporary.dmPelsWidth << "x" << g_temporary.dmPelsHeight << "@" << g_temporary.dmDisplayFrequency;
	}

	bool apply(DEVMODEW mode, const DEVMODEW& expected)
	{
		DEVMODEW current = {};
		if (!sameDisplay() || !readMode(current) || !Gdi::DominionWarsFocusPolicy::sameMode(current, expected))
		{
			return false;
		}
		if (Gdi::DominionWarsFocusPolicy::sameMode(current, mode)) { return true; }
		if (CALL_ORIG_FUNC(ChangeDisplaySettingsExW)(g_device.c_str(), &mode, nullptr, CDS_TEST, nullptr) != DISP_CHANGE_SUCCESSFUL)
		{
			return false;
		}
		if (!sameDisplay() || !readMode(current) || !Gdi::DominionWarsFocusPolicy::sameMode(current, expected))
		{
			return false;
		}
		const auto result = CALL_ORIG_FUNC(ChangeDisplaySettingsExW)(g_device.c_str(), &mode, nullptr, CDS_FULLSCREEN, nullptr);
		return result == DISP_CHANGE_SUCCESSFUL && sameDisplay() && readMode(current) &&
			Gdi::DominionWarsFocusPolicy::sameMode(current, mode);
	}

	void stopTimer(HWND hwnd)
	{
		if (g_timer) { KillTimer(hwnd, TIMER_ID); g_timer = false; }
	}

	bool isCaptureWindow(HWND foreground)
	{
		// Experimental exception, not a security boundary. These Windows screenshot
		// hosts may take focus without indicating that the user switched apps.
		DWORD pid = 0;
		GetWindowThreadProcessId(foreground, &pid);
		HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
		if (!process) { return false; }
		wchar_t path[32768] = {};
		DWORD count = ARRAYSIZE(path);
		const bool queried = !!QueryFullProcessImageNameW(process, 0, path, &count);
		CloseHandle(process);
		if (!queried) { return false; }
		const auto name = std::wcsrchr(path, L'\\');
		return name && (!_wcsicmp(name + 1, L"ScreenClippingHost.exe") || !_wcsicmp(name + 1, L"SnippingTool.exe"));
	}

	bool resume()
	{
		if (!g_suspended || g_transition) { return true; }
		if (!g_enabled) { return false; }
		DEVMODEW current = {};
		if (!sameDisplay() || !readMode(current))
		{
			disable("monitor changed while suspended; leaving desktop untouched");
			return false;
		}
		if (!Gdi::DominionWarsFocusPolicy::sameMode(current, g_original) &&
			!Gdi::DominionWarsFocusPolicy::sameMode(current, g_temporary))
		{
			disable("display was adjusted while suspended; leaving later settings untouched");
			return false;
		}
		g_transition = true;
		const bool applied = Gdi::DominionWarsFocusPolicy::sameMode(current, g_temporary) || apply(g_temporary, current);
		g_transition = false;
		if (!applied) { disable("driver rejected game desktop; keeping presentation held"); return false; }
		g_suspended = false;
		g_hold = false;
		LOG_INFO << "DW focus desktop: game mode reapplied before window return";
		return true;
	}

	void suspend(HWND hwnd)
	{
		if (!g_enabled || g_suspended || g_transition) { return; }
		DEVMODEW current = {};
		if (!sameDisplay() || !readMode(current) || !Gdi::DominionWarsFocusPolicy::sameMode(current, g_temporary))
		{
			disable("monitor or display changed before switching away; leaving settings untouched");
			return;
		}
		g_transition = true;
		g_hold = true;
		ShowWindow(hwnd, SW_SHOWMINNOACTIVE);
		if (!IsIconic(hwnd))
		{
			g_transition = false;
			g_hold = false;
			disable("window did not minimize; desktop left unchanged");
			return;
		}
		// Mark suspended before the driver call, which may reenter this window proc.
		g_suspended = true;
		const bool applied = apply(g_original, g_temporary);
		g_transition = false;
		if (!applied) { disable("desktop restoration failed; quit and inspect launcher logs"); return; }
		LOG_INFO << "DW focus desktop: minimized; original desktop restored";
	}
}

namespace Gdi::DominionWarsFocus
{
	bool isPresentationHeld() { return g_hold.load(); }

	bool beforeMessage(HWND hwnd, UINT message, WPARAM wParam)
	{
		if (!Gdi::WinProc::isDominionWarsFocusExperiment(hwnd)) { return false; }
		if (!g_initialized) { initialize(); }
		if (message == WM_TIMER && wParam == TIMER_ID && g_timer)
		{
			const auto foreground = GetForegroundWindow();
			DWORD pid = 0;
			if (foreground) { GetWindowThreadProcessId(foreground, &pid); }
			const auto elapsed = GetTickCount64() - g_lossTime;
			if (pid == GetCurrentProcessId() || !g_enabled || elapsed > 12000) { stopTimer(hwnd); return true; }
			if (!foreground || elapsed < 300 || (GetAsyncKeyState(VK_MENU) & 0x8000) || isCaptureWindow(foreground))
			{
				return true;
			}
			stopTimer(hwnd);
			suspend(hwnd);
			return true;
		}
		if ((message == WM_ACTIVATE && LOWORD(wParam) != WA_INACTIVE) || (message == WM_ACTIVATEAPP && wParam))
		{
			stopTimer(hwnd);
			if (!resume() && !g_transition) { ShowWindow(hwnd, SW_SHOWMINNOACTIVE); }
		}
		else if ((message == WM_ACTIVATE || message == WM_ACTIVATEAPP) && !g_transition && g_enabled && !g_suspended)
		{
			g_lossTime = GetTickCount64();
			g_timer = !!SetTimer(hwnd, TIMER_ID, 100, nullptr);
		}
		else if (message == WM_SYSCOMMAND && (wParam & 0xfff0) == SC_RESTORE)
		{
			stopTimer(hwnd);
			if (!resume()) { return true; }
		}
		else if (message == WM_NCDESTROY) { stopTimer(hwnd); }
		return false;
	}
}
