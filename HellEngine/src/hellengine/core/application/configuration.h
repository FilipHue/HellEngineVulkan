#pragma once

// Internal
#include <hellengine/core/typedefs.h>

namespace hellengine
{

	namespace core
	{

		struct ApplicationConfiguration
		{
			const char* working_directory = "";

			const char* title = "";
			u16 width = 0;
			u16 height = 0;
			b8 vsync = true;

			ApplicationConfiguration(const char* title, u32 width, u32 height, b8 vsync, const char* working_directory)
				: title(title), width(width), height(height), vsync(vsync), working_directory(working_directory)
			{
			}
		};

	} // namespace core

} // namespace hellengine