//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsUtilityPrerequisites.h"
#include "Math/BsVector3.h"
#include "Math/BsAABox.h"
#include "Math/BsSphere.h"
#include "Math/BsMatrix4.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Bounds represented by an axis aligned box and a sphere. */
	template<typename T>
	struct B3D_UTILITY_EXPORT TBounds
	{
		TBounds() = default;
		TBounds(const TAABox<T>& box, const TSphere<T>& sphere);
		~TBounds() = default;

		/** Returns the axis aligned box representing the bounds. */
		const TAABox<T>& GetBox() const { return mBox; }

		/** Returns the sphere representing the bounds. */
		const TSphere<T>& GetSphere() const { return mSphere; }

		/** Updates the bounds by setting the new bounding box and sphere. */
		void SetBounds(const TAABox<T>& box, const TSphere<T>& sphere);

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
		 */
		void Transform(const TMatrix4<T>& matrix);

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

	protected:
		TAABox<T> mBox;
		TSphere<T> mSphere;
	};

	/** @} */
} // namespace b3d
