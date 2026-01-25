#pragma once

namespace hellengine
{

	namespace graphics
	{

        template<typename T>
        class Singleton
        {
        public:
            static T* GetInstance()
            {
                static T instance;
                return &instance;
            }

            Singleton(const Singleton&) = delete;
            Singleton& operator=(const Singleton&) = delete;

        protected:
            Singleton() = default;
            ~Singleton() = default;
        };

	} // namespace graphics

} // namespace hellengine