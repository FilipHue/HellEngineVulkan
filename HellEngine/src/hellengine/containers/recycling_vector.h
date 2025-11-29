#pragma once

// Internal
#include "hellengine/core/core.h"

// STL
#include <vector>
#include <queue>
#include <optional>

namespace hellengine
{

	namespace containers
	{

        template<typename T>
        class RecyclingVector {
        public:
            RecyclingVector() = default;

            size_t insert(const T& value) {
                if (!m_free_indices.empty()) {
                    size_t index = m_free_indices.front();
                    m_free_indices.pop();
                    m_data[index] = value;
                    return index;
                }

                m_data.emplace_back(value);
                return m_data.size() - 1;
            }

            void erase(size_t index) {
                HE_ASSERT(index < m_data.size(), "Index out of bounds");
				HE_ASSERT(m_data[index].has_value(), "Attempted to erase an invalid index");

                m_data[index].reset();
                m_free_indices.push(index);
            }

            T& operator[](size_t index) {
                return *m_data[index];
            }

            const T& operator[](size_t index) const {
                return *m_data[index];
            }

            std::optional<T>& at(size_t index) {
				HE_ASSERT(index < m_data.size(), "Index out of bounds");
                return m_data[index];
            }

            size_t size() const {
                return m_data.size();
            }

            bool empty(size_t index) const {
                HE_ASSERT(index < m_data.size(), "Index out of bounds");
                return !m_data[index].has_value();
            }

			auto begin() {
				return m_data.begin();
			}

			auto end() {
				return m_data.end();
			}

			const auto begin() const {
				return m_data.begin();
			}

			const auto end() const {
				return m_data.end();
			}

        private:
            std::vector<std::optional<T>> m_data;
            std::queue<size_t> m_free_indices;
        };

	} // namespace containers

} // namespace hellengine