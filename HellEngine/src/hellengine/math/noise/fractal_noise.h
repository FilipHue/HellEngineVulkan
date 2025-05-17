#pragma once

// Internal
#include "perlin_noise.h"

namespace hellengine
{
	namespace math
	{

		template <typename T>
		class FractalNoise
		{
		public:
			FractalNoise(const PerlinNoise<T>& perlinNoiseIn) :
				perlinNoise(perlinNoiseIn)
			{
				octaves = 6;
				persistence = (T)0.5;
			}

			T noise(T x, T y, T z)
			{
				T sum = 0;
				T frequency = (T)1;
				T amplitude = (T)1;
				T max = (T)0;
				for (uint32_t i = 0; i < octaves; i++)
				{
					sum += perlinNoise.noise(x * frequency, y * frequency, z * frequency) * amplitude;
					max += amplitude;
					amplitude *= persistence;
					frequency *= (T)2;
				}

				sum = sum / max;
				return (sum + (T)1.0) / (T)2.0;
			}

		private:
			PerlinNoise<T> perlinNoise;
			uint32_t octaves;
			T frequency;
			T amplitude;
			T persistence;
		};

	} // namespace math

} // namespace hellengine