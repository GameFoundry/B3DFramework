//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector3.h"

namespace b3d
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

		B3D_SCRIPT_EXPORT(Exclude(true))
		TSphere(const TVector3<T>& center, T radius)
			: Radius(radius), Center(center)
		{ }

		/** Merges the two spheres, creating a new sphere that encapsulates them both. */
		void Merge(const TSphere<T>& rhs);

		/** Expands the sphere so it includes the provided point. */
		void Merge(const TVector3<T>& point);

		/** Transforms the sphere by the given matrix. */
		void Transform(const TMatrix4<T>& matrix);

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
		 * @param	ray				Ray to intersect with the sphere.
		 * @param	discardInside	(optional) If true the intersection will be discarded if ray origin
		 * 								is located within the sphere.
		 */
		std::pair<bool, T> Intersects(const TRay<T>& ray, bool discardInside = true) const;

		static const TSphere<T> kEmpty;
		static const TSphere<T> kUnit;
		static const TSphere<T> kInfinite;
	};

	template<> const TSphere<float> TSphere<float>::kEmpty = TSphere(TVector3<float>(0.0f, 0.0f, 0.0f), 0.0f);
	template<> const TSphere<double> TSphere<double>::kEmpty = TSphere(TVector3<double>(0.0, 0.0, 0.0), 0.0);

	template<> const TSphere<float> TSphere<float>::kUnit = TSphere(TVector3<float>(0.0f, 0.0f, 0.0f), 0.5f);
	template<> const TSphere<double> TSphere<double>::kUnit = TSphere(TVector3<double>(0.0, 0.0, 0.0), 0.5);

	template<> const TSphere<float> TSphere<float>::kInfinite = TSphere(TVector3<float>(0.0f, 0.0f, 0.0f), std::numeric_limits<float>::infinity());
	template<> const TSphere<double> TSphere<double>::kInfinite = TSphere(TVector3<double>(0.0, 0.0, 0.0), std::numeric_limits<double>::infinity());

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Sphere)) TSphere<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(SphereD)) TSphere<double>;

	/** @} */
} // namespace b3d
