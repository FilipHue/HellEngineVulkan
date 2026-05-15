#pragma once

// Internal
#include <hellengine/core/typedefs.h>

namespace hellengine
{

	namespace core
	{

		class RID
		{
		public:
			RID() : m_id(0) {}
			RID(u64 id) : m_id(id) {}

			operator u64() const { return m_id; }
			operator void* () const { return (void*)(uintptr_t)m_id; }

			b8 operator ==(const RID& other) const { return m_id == other.m_id; }
			b8 operator !=(const RID& other) const { return m_id != other.m_id; }
			b8 operator <(const RID& other) const { return m_id < other.m_id; }
			b8 operator >(const RID& other) const { return m_id > other.m_id; }
			b8 operator <=(const RID& other) const { return m_id <= other.m_id; }
			b8 operator >=(const RID& other) const { return m_id >= other.m_id; }

			b8 IsValid() const { return m_id != 0; }
			void Invalidate() { m_id = 0; }

			u64 GetID() const { return m_id; }

		private:
			u64 m_id;
		};

	} // namespace core

} // namespace hellengine

namespace std {

	template<>
	struct hash<hellengine::core::RID>
	{
		std::size_t operator()(const hellengine::core::RID& rid) const
		{
			return hash<u64>()(rid.GetID());
		}
	};

} // namespace std
