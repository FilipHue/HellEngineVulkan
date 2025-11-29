#pragma once

// Internal
#include <hellengine/core/typedefs.h>

// STL
#include <numeric>
#include <random>

namespace hellengine
{

	namespace math
	{

		template <typename T>
		class PerlinNoise
		{
		public:
			PerlinNoise(bool applyRandomSeed)
			{
				// Generate random lookup for permutations containing all numbers from 0..255
				std::vector<u8> plookup;
				plookup.resize(256);
				std::iota(plookup.begin(), plookup.end(), 0);
				std::default_random_engine rndEngine(applyRandomSeed ? std::random_device{}() : 0);
				std::shuffle(plookup.begin(), plookup.end(), rndEngine);

				for (u32 i = 0; i < 256; i++)
				{
					permutations[i] = permutations[256 + i] = plookup[i];
				}
			}
			~PerlinNoise() = default;

			T noise(T x, T y, T z)
			{
				i32 X = (i32)floor(x) & 255;
				i32 Y = (i32)floor(y) & 255;
				i32 Z = (i32)floor(z) & 255;

				x -= floor(x);
				y -= floor(y);
				z -= floor(z);

				T u = fade(x);
				T v = fade(y);
				T w = fade(z);

				u32 A = permutations[X] + Y;
				u32 AA = permutations[A] + Z;
				u32 AB = permutations[A + 1] + Z;
				u32 B = permutations[X + 1] + Y;
				u32 BA = permutations[B] + Z;
				u32 BB = permutations[B + 1] + Z;

				T res = lerp(w, lerp(v,
					lerp(u, grad(permutations[AA], x, y, z), grad(permutations[BA], x - 1, y, z)), lerp(u, grad(permutations[AB], x, y - 1, z), grad(permutations[BB], x - 1, y - 1, z))),
					lerp(v, lerp(u, grad(permutations[AA + 1], x, y, z - 1), grad(permutations[BA + 1], x - 1, y, z - 1)), lerp(u, grad(permutations[AB + 1], x, y - 1, z - 1), grad(permutations[BB + 1], x - 1, y - 1, z - 1))));
				return res;
			}

		private:
			T fade(T t)
			{
				return t * t * t * (t * (t * (T)6 - (T)15) + (T)10);
			}
			T lerp(T t, T a, T b)
			{
				return a + t * (b - a);
			}
			T grad(int hash, T x, T y, T z)
			{
				int h = hash & 15;
				T u = h < 8 ? x : y;
				T v = h < 4 ? y : h == 12 || h == 14 ? x : z;
				return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
			}

		private:
			u32 permutations[512];
		};

	} // namespace math

} // namespace hellengine