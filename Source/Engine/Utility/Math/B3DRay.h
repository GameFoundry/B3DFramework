//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector3.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** A ray in 3D space represented with an origin and direction. */
	template<typename T>
	struct TRay
	{
		TVector3<T> Origin;
		TVector3<T> Direction;

		TRay()
			: Origin(TVector3<T>::kZero), Direction(TVector3<T>::kUnitZ)
		{ }

		B3D_SCRIPT_EXPORT(Exclude(true))
		TRay(const TVector3<T>& origin, const TVector3<T>& direction)
			: Origin(origin), Direction(direction)
		{}

		/** Gets the position of a point t units along the ray. */
		TVector3<T> GetPoint(T t) const
		{
			return TVector3<T>(Origin + (Direction * t));
		}

		/** Gets the position of a point t units along the ray. */
		TVector3<T> operator*(T t) const
		{
			return GetPoint(t);
		}

		/** Transforms the ray by the given matrix. */
		void Transform(const TMatrix4<T>& matrix);

		/**
		 * Transforms the ray by the given matrix.
		 *
		 * @note	Provided matrix must be affine.
		 */
		void TransformAffine(const TMatrix4<T>& matrix);

		/** Ray/plane intersection, returns boolean result and distance to intersection point. */
		std::pair<bool, T> Intersects(const TPlane<T>& p) const;

		/** Ray/sphere intersection, returns boolean result and distance to nearest intersection point. */
		std::pair<bool, T> Intersects(const TSphere<T>& s) const;

		/** Ray/axis aligned box intersection, returns boolean result and distance to nearest intersection point. */
		std::pair<bool, T> Intersects(const TAABox<T>& box) const;

		/**
		 * Ray/triangle intersection, returns boolean result and distance to intersection point.
		 *
		 * @param	a				Triangle first vertex.
		 * @param	b				Triangle second vertex.
		 * @param	c				Triangle third vertex.
		 * @param	normal			The normal of the triangle. Doesn't need to be normalized.
		 * @param	positiveSide	(optional) Should intersections with the positive side (normal facing) count.
		 * @param	negativeSide	(optional) Should intersections with the negative side (opposite of normal facing) count.
		 * @return						Boolean result if intersection happened and distance to intersection point.
		 */
		std::pair<bool, T> Intersects(const TVector3<T>& a, const TVector3<T>& b, const TVector3<T>& c, const TVector3<T>& normal, bool positiveSide = true, bool negativeSide = true) const;
	};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Ray)) TRay<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(RayD)) TRay<double>;

	/** @} */
} // namespace b3d
