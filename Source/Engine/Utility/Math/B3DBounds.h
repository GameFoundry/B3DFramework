//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector3.h"
#include "Math/B3DAABox.h"
#include "Math/B3DSphere.h"
#include "Math/B3DMatrix4.h"
#include "Math/B3DTransform.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Bounds represented by an axis aligned box and a sphere. */
	template<typename T>
	struct B3D_EXPORT TBounds
	{
		constexpr TBounds() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr TBounds(ZeroTag)
			: mCenter(kZeroTag), mBoxExtents(kZeroTag), mSphereRadius((T)0.0)
		{}

		constexpr TBounds(const TVector3<T>& center, const TVector3<T>& boxExtents, T sphereRadius)
			: mCenter(center), mBoxExtents(boxExtents), mSphereRadius(sphereRadius)
		{ }

		TBounds(const TAABox<T>& box, const TSphere<T>& sphere);
		TBounds(const TAABox<T>& box);
		TBounds(const TSphere<T>& sphere);
		~TBounds() = default;

		/** Returns the axis aligned box representing the bounds. */
		TAABox<T> GetBox() const { return TAABox(mCenter - mBoxExtents, mCenter + mBoxExtents); }

		/** Returns the sphere representing the bounds. */
		TSphere<T> GetSphere() const { return TSphere(mCenter, mSphereRadius); }

		/** Merges the two bounds, creating a new bounds that encapsulates them both. */
		void Merge(const TBounds& rhs);

		/** Expands the bounds so it includes the provided point. */
		void Merge(const TVector3<T>& point);

		/**
		 * Transforms the bounds by the given matrix.
		 *
		 * @note
		 * As the resulting box will no longer be axis aligned, an axis align box
		 * is instead created by encompassing the transformed oriented bounding box.
		 * Retrieving the value as an actual OBB would provide a tighter fit.
		 *
		 * @note
		 * Provided matrix must be affine.
		 */
		void TransformAffine(const TMatrix4<T>& matrix);

		/**
		 * Transforms the bounds by the given transform.
		 *
		 * @note
		 * As the resulting box will no longer be axis aligned, an axis align box
		 * is instead created by encompassing the transformed oriented bounding box.
		 * Retrieving the value as an actual OBB would provide a tighter fit.
		 */
		void TransformAffine(const TTransform<T>& transform);

		static const TBounds<T> kEmpty;
		static const TBounds<T> kUnit;
		static const TBounds<T> kInfinite;
	protected:
		TVector3<T> mCenter = kZeroTag;
		TVector3<T> mBoxExtents = kZeroTag;
		T mSphereRadius = (T)0.0;
	};

	template<> const TBounds<float> TBounds<float>::kEmpty = TBounds(kZeroTag);
	template<> const TBounds<double> TBounds<double>::kEmpty = TBounds(kZeroTag);

	template<> const TBounds<float> TBounds<float>::kUnit = TBounds(TVector3<float>::kZero, TVector3<float>::kOne / 2.0f, 1.0f);
	template<> const TBounds<double> TBounds<double>::kUnit = TBounds(TVector3<double>::kZero, TVector3<double>::kOne / 2.0, 1.0);

	template<> const TBounds<float> TBounds<float>::kInfinite = TBounds(TVector3<float>::kZero, TVector3<float>::kInfinite, std::numeric_limits<float>::infinity());
	template<> const TBounds<double> TBounds<double>::kInfinite = TBounds(TVector3<double>::kZero, TVector3<double>::kInfinite, std::numeric_limits<double>::infinity());

	/** @} */
} // namespace b3d
