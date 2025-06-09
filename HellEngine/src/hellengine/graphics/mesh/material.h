#pragma once

//Internal
#include <hellengine/math/core.h>
#include <hellengine/graphics/objects/types.h>

namespace hellengine
{

	namespace graphics
	{

		class VulkanTexture2D;
		class VulkanUniformBuffer;

		enum TextureType
		{
			TextureType_None =				0,

			TextureType_Diffuse =			1 << 0,
			TextureType_Specular =			1 << 1,
			TextureType_Ambient =			1 << 2,
			TextureType_Emissive =			1 << 3,
			TextureType_Height =			1 << 4,
			TextureType_Normals =			1 << 5,
			TextureType_Shininess =			1 << 6,
			TextureType_Opacity =			1 << 7,
			TextureType_Displacement =		1 << 8,
			TextureType_Lightmap =			1 << 9,
			TextureType_Reflection =		1 << 10,

			TextureType_BaseColor =			1 << 11,
			TextureType_NormalCamera =		1 << 12,
			TextureType_EmissionColor =		1 << 13,
			TextureType_Metalness =			1 << 14,
			TextureType_DiffuseRoughness =	1 << 15,
			TextureType_AmbientOcclusion =	1 << 16,
			TextureType_Sheen =				1 << 17,
			TextureType_Clearcoat =			1 << 18,
			TextureType_Transmission =		1 << 19,

			TextureType_Unknown =			1 << 20,

			TextureType_Count
		};

		INLINE TextureType operator|(TextureType a, TextureType b)
		{
			return static_cast<TextureType>(static_cast<u32>(a) | static_cast<u32>(b));
		}

		INLINE TextureType operator&(TextureType a, TextureType b)
		{
			return static_cast<TextureType>(static_cast<u32>(a) & static_cast<u32>(b));
		}

		INLINE TextureType& operator|=(TextureType& a, TextureType b)
		{
			a = a | b;
			return a;
		}

		INLINE TextureType& operator&=(TextureType& a, TextureType b)
		{
			a = a & b;
			return a;
		}

		INLINE TextureType operator~(TextureType a)
		{
			return static_cast<TextureType>(~static_cast<u32>(a));
		}

		INLINE bool HasFlag(TextureType value, TextureType flag)
		{
			return (static_cast<u32>(value) & static_cast<u32>(flag)) != 0;
		}

		INLINE std::string GetTextureTypeToString(TextureType type)
		{
			switch (type)
			{
			case TextureType_None:				return "None";

			case TextureType_Diffuse:			return "Diffuse";
			case TextureType_Specular:			return "Specular";
			case TextureType_Ambient:			return "Ambient";
			case TextureType_Emissive:			return "Emissive";
			case TextureType_Height:			return "Height";
			case TextureType_Normals:			return "Normals";
			case TextureType_Shininess:			return "Shininess";
			case TextureType_Opacity:			return "Opacity";
			case TextureType_Displacement:		return "Displacement";
			case TextureType_Lightmap:			return "Lightmap";
			case TextureType_Reflection:		return "Reflection";

			case TextureType_BaseColor:			return "BaseColor";
			case TextureType_NormalCamera:		return "NormalCamera";
			case TextureType_EmissionColor:		return "EmissionColor";
			case TextureType_Metalness:			return "Metalness";
			case TextureType_DiffuseRoughness:	return "DiffuseRoughness";
			case TextureType_AmbientOcclusion:	return "AmbientOcclusion";
			case TextureType_Sheen:				return "Sheen";
			case TextureType_Clearcoat:			return "Clearcoat";
			case TextureType_Transmission:		return "Transmission";

			case TextureType_Unknown:			return "Unknown";

			default:							return "Invalid";
			}
		}

		struct MaterialInfo
		{
			std::map<TextureType, i32> texture_indices;
			TextureType used_textures_flag_bit;

			void Set(TextureType type, i32 index)
			{
				texture_indices[type] = index;
				used_textures_flag_bit |= type;
			}

			i32 Get(TextureType type) const
			{
				auto it = texture_indices.find(type);
				return (it != texture_indices.end()) ? it->second : 0;
			}

			std::map<TextureType, i32>& GetTextureInfo() { return texture_indices; }

			b8 Uses(TextureType type) const
			{
				return (used_textures_flag_bit & type) == type;
			}
		};

		ALIGN_AS(16) struct MaterialGPUInfo
		{
			i32 diffuse_index;
			i32 specular_index;
			i32 ambient_index;
			i32 emissive_index;
			i32 height_index;
			i32 normal_index;
			i32 shininess_index;
			i32 opacity_index;
			i32 displacement_index;
			i32 lightmap_index;
			i32 reflection_index;

			i32 base_color_index;
			i32 normal_camera_index;
			i32 emission_color_index;
			i32 metalness_index;
			i32 diffuse_roughness_index;
			i32 ambient_occlusion_index;
			i32 sheen_index;
			i32 clearcoat_index;
			i32 transmission_index;
		};

	} // namespace graphics

} // namespace hellengine