#pragma once

#include <cwchar>
#include <Windows.h>

namespace Gdi::DominionWarsFocusPolicy
{
	inline bool sameMode(const DEVMODEW& a, const DEVMODEW& b)
	{
		return a.dmPelsWidth == b.dmPelsWidth && a.dmPelsHeight == b.dmPelsHeight &&
			a.dmBitsPerPel == b.dmBitsPerPel && a.dmDisplayFrequency == b.dmDisplayFrequency &&
			a.dmPosition.x == b.dmPosition.x && a.dmPosition.y == b.dmPosition.y &&
			a.dmDisplayOrientation == b.dmDisplayOrientation && a.dmDisplayFlags == b.dmDisplayFlags &&
			a.dmDisplayFixedOutput == b.dmDisplayFixedOutput;
	}

	inline bool expectedGameMode(const DEVMODEW& original, const DEVMODEW& current)
	{
		// Stock DDrawCompat resets the physical display to the registry desktop
		// during startup. Accept that exact original mode OR the launcher bootstrap,
		// never an arbitrary later resolution, monitor position or orientation.
		return sameMode(original, current) || (current.dmPelsWidth == 1280 && current.dmPelsHeight == 960 &&
			current.dmBitsPerPel == 32 && current.dmDisplayFrequency > 0 && current.dmDisplayFrequency <= 1000 &&
			current.dmPosition.x == original.dmPosition.x && current.dmPosition.y == original.dmPosition.y &&
			current.dmDisplayOrientation == original.dmDisplayOrientation);
	}

	inline bool decodeMode(const wchar_t* hex, DEVMODEW& mode)
	{
		if (!hex || std::wcslen(hex) != sizeof(mode) * 2) { return false; }
		DEVMODEW decoded = {};
		auto bytes = reinterpret_cast<unsigned char*>(&decoded);
		for (size_t i = 0; i < sizeof(decoded); ++i)
		{
			unsigned value = 0;
			for (size_t j = 0; j < 2; ++j)
			{
				const auto c = hex[i * 2 + j];
				const int digit = c >= L'0' && c <= L'9' ? c - L'0' :
					c >= L'A' && c <= L'F' ? c - L'A' + 10 :
					c >= L'a' && c <= L'f' ? c - L'a' + 10 : -1;
				if (digit < 0) { return false; }
				value = value * 16 + digit;
			}
			bytes[i] = static_cast<unsigned char>(value);
		}
		const DWORD fields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL | DM_DISPLAYFREQUENCY;
		if (decoded.dmSize != sizeof(decoded) || decoded.dmDriverExtra != 0 ||
			(decoded.dmFields & fields) != fields || decoded.dmBitsPerPel != 32 ||
			decoded.dmPelsWidth < 640 || decoded.dmPelsWidth > 32768 ||
			decoded.dmPelsHeight < 480 || decoded.dmPelsHeight > 32768 ||
			decoded.dmDisplayFrequency < 1 || decoded.dmDisplayFrequency > 1000)
		{
			return false;
		}
		mode = decoded;
		return true;
	}
}
