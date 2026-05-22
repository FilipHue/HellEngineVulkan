#pragma once

// Internal
#include <hellengine/core/core.h>
#include <hellengine/core/uuid/rid.h>

// STL
#include <string>

namespace hellengine
{

	using namespace core;
	namespace graphics
	{

		enum ResourceType
		{
			ResourceType_None,
			ResourceType_Texture2D,

			ResourceType_Count
		};

		enum ResourceAccess
		{
			ResourceAccess_None,
			ResourceAccess_Read,
			ResourceAccess_Write,
			ResourceAccess_ReadWrite,

			ResourceAccess_Count
		};

		enum ResourceUsage
		{
			Undefined,
			ShaderRead,
			ColorAttachment,
			DepthStencilAttachment,
			TransferSrc,
			TransferDst,
			Present
		};

		struct TextureDescriptor
		{
			u32 width = 0;
			u32 height = 0;
			u32 depth = 1;
			u32 mipLevels = 1;
			u32 arrayLayers = 1;
			u32 format = 0;
			u32 sampleCount = 1;
		};

		struct ResourceDescriptor
		{
			ResourceType type = ResourceType_None;
			std::string name;

			union
			{
				TextureDescriptor texture;
			};
		};

		struct Resource
		{
			ResourceType type = ResourceType_None;
			std::string name;
			RID id = 0;
		};

	} // namespace graphics

} // namespace hellengine
