#pragma once

#include <Windows.h>

namespace Gdi::DominionWarsFocus
{
	bool beforeMessage(HWND hwnd, UINT message, WPARAM wParam);
	bool isPresentationHeld();
}
