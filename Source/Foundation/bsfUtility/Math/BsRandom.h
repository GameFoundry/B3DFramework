//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "BsVector3.h"
#include "BsVector2.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Generates pseudo random numbers using the Xorshift128 algorithm. Suitable for high performance requirements. */
	class BS_SCRIPT_EXPORT(m:Math) Random
	{
	public:
		/** Initializes a new generator using the specified seed. */
		BS_SCRIPT_EXPORT()
		Random(uint32_t seed = 0)
		{
			setSeed(seed);
		}

		/** Changes the seed of the generator to the specified value. */
		BS_SCRIPT_EXPORT()
		void setSeed(uint32_t seed)
		{
			mSeed[0] = seed;
			mSeed[1] = seed * 0x72e0447c + 1; // Arbitrary random numbers
			mSeed[2] = seed * 0x352ad225 + 1;
			mSeed[3] = seed * 0x03c3629f + 1;
		}

		/** Returns a random value in range [0, std::numeric_limits<uint32_t>::max()]. */
		BS_SCRIPT_EXPORT()
		uint32_t get() const
		{
			// Using xorshift128 algorithm
			uint32_t t = mSeed[3];
			t ^= t << 11;
			t ^= t >> 8;

			mSeed[3] = mSeed[2];
			mSeed[2] = mSeed[1];
			mSeed[1] = mSeed[0];

			const uint32_t s = mSeed[0];
			t ^= s;
			t ^= s >> 19;

			mSeed[0] = t;
			return t;
		}

		/** Returns a random value in range [min, max]. */
		BS_SCRIPT_EXPORT()
		int32_t getRange(int32_t min, int32_t max) const
		{
			assert(max > min);

			// Note: Not using modulo for performance
			const int32_t range = max - min + 1;

			constexpr static float DELTA = 0e-5f;
			return min + (int32_t)(getUNorm() * ((float)range - DELTA));
		}

		/** Returns a random value in range [0, 1]. */
		BS_SCRIPT_EXPORT()
		float getUNorm() const
		{
			// Mask first 23 bits and divide by 2^23-1
			return float(get() & 0x007FFFFF) / 8388607.0f;
		}
		
		/** Returns a random value in range [-1, 1]. */
		BS_SCRIPT_EXPORT()
		float getSNorm() const
		{
			return getUNorm() * 2.0f - 1.0f;
		}

		/** Returns a random unit vector in three dimensions. */
		BS_SCRIPT_EXPORT()
		Vector3 getUnitVector() const
		{
			// Pick a random number on a unit cube and use the result only if squared size less than 1. This is faster
			// than most other methods, and generally not many iterations are required to get the required vector.

			Vector3 output;
			float sqrdSize;

			do
			{
				output.x = getSNorm();
				output.y = getSNorm();
				output.z = getSNorm();
				sqrdSize = output.squaredLength();
				
			} while (sqrdSize > 1.0f || sqrdSize < 0.001f);

			return Vector3::normalize(output);
		}

		/** Returns a random unit vector in two dimensions. */
		BS_SCRIPT_EXPORT()
		Vector2 getUnitVector2D() const
		{
			// Pick a random number on a unit square and use the result only if squared size less than 1. This is faster
			// than most other methods, and generally not many iterations are required to get the required vector.

			Vector2 output;
			float sqrdSize;

			do
			{
				output.x = getSNorm();
				output.y = getSNorm();
				sqrdSize = output.squaredLength();
				
			} while (sqrdSize > 1.0f || sqrdSize < 0.001f);

			return Vector2::normalize(output);
		}

		/** Returns a random point inside a unit sphere. */
		BS_SCRIPT_EXPORT()
		Vector3 getPointInSphere() const
		{
			const Vector3 dir = getUnitVector();
			return dir * std::pow(getUNorm(), 1.0f / 3.0f);
		}

		/** 
		 * Returns a random point inside the specified range in a sphere shell of unit radius, with the specified
		 * thickness, in range [0, 1]. Thickness of 0 will generate points on the sphere surface, while thickness of 1
		 * will generate points within the entire sphere volume. Intermediate values represent the shell, which is a volume
		 * between two concentric spheres.
		 */
		BS_SCRIPT_EXPORT()
		Vector3 getPointInSphereShell(float thickness) const
		{
			const float minRadius = 1.0f - thickness;

			const Vector3 dir = getUnitVector();
			return dir * (minRadius + thickness * std::pow(getUNorm(), 1.0f / 3.0f));
		}

		/** Returns a random point inside a unit circle. */
		BS_SCRIPT_EXPORT()
		Vector2 getPointInCircle() const
		{
			const Vector2 dir = getUnitVector2D();
			return dir * std::pow(getUNorm(), 1.0f / 2.0f);
		}

		/** 
		 * Returns a random point inside the specified range in a circle shell of unit radius, with the specified
		 * thickness, in range [0, 1]. Thickness of 0 will generate points on the circle edge, while thickness of 1 will
		 * generate points within the entire circle surface. Intermediate values represent the shell, which is the surface
		 * between two concentric circles.
		 */
		BS_SCRIPT_EXPORT()
		Vector2 getPointInCircleShell(float thickness) const
		{
			const float minRadius = 1.0f - thickness;

			const Vector2 dir = getUnitVector2D();
			return dir * (minRadius + thickness * std::pow(getUNorm(), 1.0f / 2.0f));
		}

		/** Returns a random point on a unit arc with the specified length (angle). Angle of 360 represents a circle. */
		BS_SCRIPT_EXPORT()
		Vector2 getPointInArc(Degree angle) const
		{
			float val = getUNorm() * angle.valueRadians();
			return Vector2(Math::cos(val), Math::sin(val));
		}

		/** 
		 * Returns a random point inside the specified range in an arc shell of unit radius, with the specified 
		 * length (angle) and thickness in range [0, 1]. Angle of 360 represents a circle shell. Thickness of 0 will
		 * generate points on the arc edge, while thickness of 1 will generate points on the entire arc 'slice'. 
		 * Intermediate vlaues represent the shell, which is the surface between two concentric circles.
		 */
		BS_SCRIPT_EXPORT()
		Vector2 getPointInArcShell(Degree angle, float thickness) const
		{
			const float minRadius = 1.0f - thickness;

			const float val = getUNorm() * angle.valueRadians();
			const Vector2 dir(Math::cos(val), Math::sin(val));

			return dir * (minRadius + thickness * std::pow(getUNorm(), 1.0f / 2.0f));
		}

		/** Returns a random set of Barycentric coordinates that may be used for generating random points on a triangle. */
		BS_SCRIPT_EXPORT()
		Vector3 getBarycentric() const
		{
			float u = getUNorm();
			float v = getUNorm();

			if((u + v) > 1.0f)
			{
				u = 1.0f - u;
				v = 1.0f - v;
			}

			const float w = 1.0f - u - v;
			return Vector3(u, v, w);
		}

	private:
		mutable uint32_t mSeed[4];
	};

	/** @} */
}
