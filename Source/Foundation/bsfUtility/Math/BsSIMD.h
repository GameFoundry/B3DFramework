//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector4.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "Math/BsRect2.h"

#define B3D_SIMDPP_ARCH_X86_SS_E4_1

#if B3D_COMPILER == B3D_COMPILER_ID_MSVC
#	pragma warning(disable : 4244)
#endif

#include "ThirdParty/simdpp/simd.h"

#if B3D_COMPILER == B3D_COMPILER_ID_MSVC
#	pragma warning(default : 4244)
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
			SIMDPP_ALIGN(16)
			Vector4 Center;

			/** Extents (half-size) of the bounds, W component unused. */
			SIMDPP_ALIGN(16)
			Vector4 Extents;

			AABox() = default;

			/** Initializes bounds from an AABox. */
			AABox(const bs::AABox& box)
			{
				Center = Vector4(box.GetCenter());
				Extents = Vector4(box.GetHalfSize());
			}

			/** Initializes bounds from a Sphere. */
			AABox(const Sphere& sphere)
			{
				Center = Vector4(sphere.GetCenter());

				float radius = sphere.GetRadius();
				Extents = Vector4(radius, radius, radius, 0.0f);
			}

			/** Initializes bounds from a vector representing the center and equal extents in all directions. */
			AABox(const Vector3& center, float extent)
			{
				this->Center = Vector4(center);
				Extents = Vector4(extent, extent, extent, 0.0f);
			}

			/** Returns true if the current bounds object intersects the provided object. */
			bool Intersects(const AABox& other) const
			{
				auto myCenter = load<float32x4>(&Center);
				auto otherCenter = load<float32x4>(&other.Center);

				float32x4 diff = abs(sub(myCenter, otherCenter));

				auto myExtents = simd::load<float32x4>(&Extents);
				auto otherExtents = simd::load<float32x4>(&other.Extents);

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
			SIMDPP_ALIGN(16)
			Vector4 Center;

			/** Extents (half-size) of the bounds. Z and W component unused. */
			SIMDPP_ALIGN(16)
			Vector4 Extents;

			Rect2() = default;

			/** Initializes bounds from an Rect2. */
			Rect2(const bs::Rect2& rect)
			{
				Center = Vector4(rect.GetCenter().X, rect.GetCenter().Y, 0.0f, 0.0f);
				Extents = Vector4(rect.GetHalfSize().X, rect.GetHalfSize().Y, 0.0f, 0.0f);
			}

			/** Initializes bounds from a vector representing the center and equal extents in all directions. */
			Rect2(const Vector2& center, float extent)
			{
				this->Center = Vector4(center.X, center.Y, 0.0f, 0.0f);
				Extents = Vector4(extent, extent, 0.0f, 0.0f);
			}

			/** Returns true if the current bounds object intersects the provided object. */
			bool Overlaps(const Rect2& other) const
			{
				auto myCenter = load<float32x4>(&Center);
				auto otherCenter = load<float32x4>(&other.Center);

				float32x4 diff = abs(sub(myCenter, otherCenter));

				auto myExtents = simd::load<float32x4>(&Extents);
				auto otherExtents = simd::load<float32x4>(&other.Extents);

				float32x4 extents = add(myExtents, otherExtents);

				return test_bits_any(bit_cast<uint32x4>(cmp_gt(diff, extents))) == false;
			}
		};

		/** @} */
	} // namespace simd
} // namespace bs
