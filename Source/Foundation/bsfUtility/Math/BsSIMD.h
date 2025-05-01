//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"
#include "Math/BsVector4.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "Math/BsArea2.h"

#if B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_32 || B3D_ARCHITECTURE == B3D_ARCHITECTURE_ID_X86_64
#define SIMDPP_ARCH_X86_AVX 1
#elif B3D_ARCHITECTURE_ID_ARM64
#define SIMDPP_ARCH_ARM_NEON 1
#endif

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
				Center = Vector4(sphere.Center);

				float radius = sphere.Radius;
				Extents = Vector4(radius, radius, radius, 0.0f);
			}

			/** Initializes bounds from a vector representing the center and equal extents in all directions. */
			AABox(const Vector3& center, float extent)
			{
				this->Center = Vector4(center);
				Extents = Vector4(extent, extent, extent, 0.0f);
			}

			/** Returns true if the current bounds object intersects the provided object. */
			bool Overlaps(const AABox& other) const
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

		/** Version of bs::Area2 suitable for SIMD use. */
		struct Area2
		{
			/** Center of the bounds. Z and W component unused.*/
			SIMDPP_ALIGN(16)
			Vector4 Center;

			/** Extents (half-size) of the bounds. Z and W component unused. */
			SIMDPP_ALIGN(16)
			Vector4 Extents;

			Area2() = default;

			/** Initializes bounds from an Area2. */
			Area2(const bs::Area2& area)
			{
				const Size2 halfSize = area.GetSize() * 0.5f;

				Center = Vector4(area.GetCenter().X, area.GetCenter().Y, 0.0f, 0.0f);
				Extents = Vector4(halfSize.Width, halfSize.Height, 0.0f, 0.0f);
			}

			/** Initializes bounds from a vector representing the center and equal extents in all directions. */
			Area2(const Vector2& center, float extent)
			{
				this->Center = Vector4(center.X, center.Y, 0.0f, 0.0f);
				Extents = Vector4(extent, extent, 0.0f, 0.0f);
			}

			/** Returns true if the current bounds object intersects the provided object. */
			bool Overlaps(const Area2& other) const
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

		/** Version of bs::Range suitable for SIMD use. */
		struct Range
		{
			/** Center of the bounds. Y, Z and W component unused.*/
			SIMDPP_ALIGN(16)
			Vector4 Center;

			/** Extents (half-size) of the bounds. Y, Z and W component unused. */
			SIMDPP_ALIGN(16)
			Vector4 Extents;

			Range() = default;

			/** Initializes bounds from an Area2. */
			Range(const bs::Range& range)
			{
				Center = Vector4(range.Center, 0.0f, 0.0f, 0.0f);
				Extents = Vector4(range.Extent, 0.0f, 0.0f, 0.0f);
			}

			/** Initializes bounds from a vector representing the center and equal extents in all directions. */
			Range(float center, float extent)
			{
				Center = Vector4(center, 0.0f, 0.0f, 0.0f);
				Extents = Vector4(extent, 0.0f, 0.0f, 0.0f);
			}

			/** Returns true if the current bounds object intersects the provided object. */
			bool Overlaps(const Range& other) const
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
