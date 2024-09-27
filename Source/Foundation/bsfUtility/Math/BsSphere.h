//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A sphere represented by a center point and a radius. */
	template<typename T>
	struct TSphere
	{
		T Radius;
		TVector3<T> Center;

		/** Default constructor. Creates a unit sphere around the origin. */
		TSphere()
			: Radius((T)1.0), Center(TVector3<T>::kZero)
		{ }

		TSphere(const TVector3<T>& center, T radius)
			: Radius(radius), Center(center)
		{ }

		/** Merges the two spheres, creating a new sphere that encapsulates them both. */
		void Merge(const TSphere<T>& rhs);

		/** Expands the sphere so it includes the provided point. */
		void Merge(const TVector3<T>& point);

		/** Transforms the sphere by the given matrix. */
		template<typename Condition = T, std::enable_if_t<std::is_same_v<Condition, float>, int> = 0> // TODO - Temporarily enabled for float only, until we get TMatrix3<T>
		void Transform(const Matrix4& matrix);

		/** Returns whether or not this sphere contains the provided point. */
		bool Contains(const TVector3<T>& v) const;

		/** Returns whether or not this sphere intersects another sphere. */
		bool Intersects(const TSphere<T>& s) const;

		/** Returns whether or not this sphere intersects a box. */
		bool Intersects(const TAABox<T>& box) const;

		/** Returns whether or not this sphere intersects a plane. */
		bool Intersects(const TPlane<T>& plane) const;

		/**
		 * Ray/sphere intersection, returns boolean result and distance to nearest intersection.
		 *
		 * @param[in]	ray				Ray to intersect with the sphere.
		 * @param[in]	discardInside	(optional) If true the intersection will be discarded if ray origin
		 * 								is located within the sphere.
		 */
		std::pair<bool, T> Intersects(const TRay<T>& ray, bool discardInside = true) const;
	};

	/** @} */
} // namespace bs
