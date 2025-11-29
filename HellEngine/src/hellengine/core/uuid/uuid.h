#pragma once

// Internal
#include <hellengine/math/random/random.h>

namespace hellengine
{

	using namespace math;
	namespace core
	{

		class UUID
		{
		public:
			UUID() : m_uuid(Random::GetUInt64()) {}
			UUID(u64 uuid) : m_uuid(uuid) {}

			operator u64() const { return m_uuid; }
			operator void* () const { return (void*)(uintptr_t)m_uuid; }

			static UUID Generate() { return UUID(); }

		private:
			u64 m_uuid;
		};
		 
	} // namespace core

} // namespace hellengine

namespace std {

	template<>
	struct hash<hellengine::core::UUID>
	{
		std::size_t operator()(const hellengine::core::UUID& uuid) const
		{
			return hash<uint64_t>()(uuid);
		}
	};
}