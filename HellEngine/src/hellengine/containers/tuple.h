#pragma once

// Internal
#include <hellengine/core/api.h>

namespace hellengine
{
	namespace containers
	{

		template<typename TyA, typename TyB>
		class Tuple
		{
			using FirstTy = TyA;
			using SecondTy = TyB;

		public:
			HE_API Tuple() = default;
			HE_API Tuple(TyA first, TyB second) : m_first(first), m_second(second) {}
			HE_API Tuple(const Tuple& other) : m_first(other.m_first), m_second(other.m_second) {}
			HE_API Tuple(const Tuple&& other) : m_first(other.m_first), m_second(other.m_second) {}


			HE_API TyA GetFirst() const { return m_first; }
			HE_API TyB GetSecond() const { return m_second; }

		private:
			TyA m_first;
			TyB m_second;
		};

	} // namespace containers

} // namespace hellengine