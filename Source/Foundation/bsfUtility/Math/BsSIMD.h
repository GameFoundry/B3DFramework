//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector4.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"

#define SIMDPP_ARCH_X86_SSE4_1

#if BS_COMPILER == BS_COMPILER_MSVC
#pragma warning(disable: 4244)
#endif

#include "ThirdParty/simdpp/simd.h"

#if BS_COMPILER == BS_COMPILER_MSVC
#pragma warning(default: 4244)
#endif

namespace bs
{
	namespace simd
	{
		using namespace simdpp;

		/** @addtogroup Math
		 *  @{
		 */

		/** 
		 * Version of bs::AABox suitable for SIMD use. Takes up a bit more memory than standard AABox and is always 16-byte
		 * aligned.
		 */
		struct AABox
		{
			/** Center of the bounds, W component unused. */
			SIMDPP_ALIGN(16) Vector4 center;

			/** Extents (half-size) of the bounds, W component unused. */
			SIMDPP_ALIGN(16) Vector4 extents;

			AABox() = default;

			/** Initializes bounds from an AABox. */
			AABox(const bs::AABox& box)
			{
				center = Vector4(box.getCenter());
				extents = Vector4(box.getHalfSize());
			}

			/** Initializes bounds from a Sphere. */
			AABox(const Sphere& sphere)
			{
				center = Vector4(sphere.getCenter());

				float radius = sphere.getRadius();
				extents = Vector4(radius, radius, radius, 0.0f);
			}

			/** Initializes bounds from a vector representing the center and equal extents in all directions. */
			AABox(const Vector3& center, float extent)
			{
				this->center = Vector4(center);
				extents = Vector4(extent, extent, extent, 0.0f);
			}

			/** Returns true if the current bounds object intersects the provided object. */
			bool intersects(const AABox& other) const
			{
				auto myCenter = load<float32x4>(&center);
				auto otherCenter = load<float32x4>(&other.center);

				float32x4 diff = abs(sub(myCenter, otherCenter));

				auto myExtents = simd::load<float32x4>(&extents);
				auto otherExtents = simd::load<float32x4>(&other.extents);

				float32x4 extents = add(myExtents, otherExtents);

				return test_bits_any(bit_cast<uint32x4>(cmp_gt(diff, extents))) == false;
			}
		};

		/**
		* Version of bs::Rect2 suitable for SIMD use.
		*/
		struct Rect2
		{
			/** Center of the bounds. Z and W component unused.*/
			SIMDPP_ALIGN(16) Vector4 center;

			/** Extents (half-size) of the bounds. Z and W component unused. */
			SIMDPP_ALIGN(16) Vector4 extents;

			Rect2() = default;

			/** Initializes bounds from an Rect2. */
			Rect2(const bs::Rect2& rect)
			{
				center = Vector4(rect.getCenter().x, rect.getCenter().y, 0.0f, 0.0f);
				extents = Vector4(rect.getHalfSize().x, rect.getHalfSize().y, 0.0f, 0.0f);
			}

			/** Initializes bounds from a vector representing the center and equal extents in all directions. */
			Rect2(const Vector2& center, float extent)
			{
				this->center = Vector4(center.x, center.y, 0.0f, 0.0f);
				extents = Vector4(extent, extent, 0.0f, 0.0f);
			}

			/** Returns true if the current bounds object intersects the provided object. */
			bool overlaps(const Rect2& other) const
			{
				auto myCenter = load<float32x4>(&center);
				auto otherCenter = load<float32x4>(&other.center);

				float32x4 diff = abs(sub(myCenter, otherCenter));

				auto myExtents = simd::load<float32x4>(&extents);
				auto otherExtents = simd::load<float32x4>(&other.extents);

				float32x4 extents = add(myExtents, otherExtents);

				return test_bits_any(bit_cast<uint32x4>(cmp_gt(diff, extents))) == false;
			}
		};

		/** @} */
	}
}
