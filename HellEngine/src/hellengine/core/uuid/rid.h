#pragma once

// Internal
#include <hellengine/core/uuid/uuid.h>

namespace hellengine
{

	namespace core
	{

		class ResourceId
		{
		public:
			ResourceId() = default;
			explicit ResourceId(const UUID& uuid) : m_uuid(uuid) {}

			b8 IsValid() const { return m_uuid != 0; }
			UUID GetUUID() const { return m_uuid; }
			bool operator ==(const ResourceId& other) const { return m_uuid == other.m_uuid; }
			bool operator !=(const ResourceId& other) const { return m_uuid != other.m_uuid; }

			static ResourceId Generate() { return ResourceId(UUID::Generate()); }
			static ResourceId Invalid() { return ResourceId(UUID(0)); }

		private:
			UUID m_uuid;
		};

	} // namespace core

} // namespace hellengine

namespace std
{
	template<>
	struct hash<hellengine::core::ResourceId>
	{
		size_t operator()(const hellengine::core::ResourceId& id) const noexcept
		{
			return std::hash<u64>()((u64)id.GetUUID());
		}
	};
}