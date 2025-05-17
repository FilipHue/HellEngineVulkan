#pragma once

// Windows
#include <Windows.h>
#include <windowsx.h>

namespace hellengine
{

	namespace platform
	{

		struct WindowsInternalState
		{
			HINSTANCE instance_handle;
			HWND window_handle;
		};

	} // namespace core

} // namespace hellengine