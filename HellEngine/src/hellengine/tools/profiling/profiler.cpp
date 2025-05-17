#include "hepch.h"
#include "profiler.h"

namespace hellengine
{

	namespace tools
	{

		HE_API ProfilerManager& ProfilerManager::GetInstance()
		{
			static ProfilerManager instance;

			return instance;
		}

	} // namespace tools

} // namespace hellengine