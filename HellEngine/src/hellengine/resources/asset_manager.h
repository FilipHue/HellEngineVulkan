#pragma once

// Internal
#include <hellengine/resources/file_manager.h>

namespace hellengine
{

	namespace resources
	{

		class AssetManager
		{
		public:
			AssetManager();
			~AssetManager();

			HE_API static void LoadAsset(File& file);
		};

	} // namespace resources

} // namespace hellengine