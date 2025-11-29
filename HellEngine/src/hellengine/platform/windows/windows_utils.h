#pragma once

// Standard
#include <string>

namespace hellengine
{

	namespace platform
	{

		// const char* -> const wchar_t*
		const wchar_t* CharToWideChar(const char* str)
		{
			const size_t size = strlen(str) + 1;
			wchar_t* wstr = new wchar_t[size];
			size_t outSize;
			mbstowcs_s(&outSize, wstr, size, str, size - 1);
			return wstr;
		}

		// const wchar_t* -> const char*
		const char* WideCharToChar(const wchar_t* wstr)
		{
			const size_t size = wcslen(wstr) + 1;
			char* str = new char[size];
			size_t outSize;
			wcstombs_s(&outSize, str, size, wstr, size - 1);
			return str;
		}

	} // namespace platform

} // namespace hellengine