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

	/**
	 * The "positive side" of the plane is the half space to which the plane normal points. The "negative side" is the
	 * other half space. The flag "no side" indicates the plane itself.
	 */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Math)) PlaneSide
	{
		None,
		Positive,
		Negative,
		Both
	};

	/** A plane represented by a normal and a distance. */
	template<typename T>
	struct TPlane
	{
		B3D_SCRIPT_EXPORT(Exclude(true))
		TPlane()
			: Normal(kZeroTag), D((T)0.0)
		{ }

		B3D_SCRIPT_EXPORT(Exclude(true))
		TPlane(const TPlane<T>& copy) = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		TPlane(const TVector3<T>& normal, T d)
			: Normal(normal), D(d)
		{ }

		B3D_SCRIPT_EXPORT(Exclude(true))
		TPlane(T a, T b, T c, T d)
			:Normal(a, b, c), D(d)
		{ }

		B3D_SCRIPT_EXPORT(Exclude(true))
		TPlane(const TVector3<T>& normal, const TVector3<T>& point)
			:Normal(normal), D(normal.Dot(point))
		{ }

		B3D_SCRIPT_EXPORT(Exclude(true))
		TPlane(const TVector3<T>& point0, const TVector3<T>& point1, const TVector3<T>& point2);

		TPlane& operator=(const TPlane& rhs) = default;

		/**
		 * Returns the side of the plane where the point is located on.
		 *
		 * @note	NO_SIDE signifies the point is on the plane.
		 */
		PlaneSide GetSide(const TVector3<T>& point, T epsilon = (T)0.0) const;

		/**
		 * Returns the side where the alignedBox is. The flag BOTH_SIDE indicates an intersecting box.
		 * One corner ON the plane is sufficient to consider the box and the plane intersecting.
		 */
		PlaneSide GetSide(const TAABox<T>& box) const;

		/** Returns the side where the sphere is. The flag BOTH_SIDE indicates an intersecting sphere. */
		PlaneSide GetSide(const TSphere<T>& sphere) const;

		/**
		 * Returns a distance from point to plane.
		 *
		 * @note	The sign of the return value is positive if the point is on the
		 * 			positive side of the plane, negative if the point is on the negative
		 * 			side, and zero if the point is on the plane.
		 */
		T GetDistance(const TVector3<T>& point) const;

		/** Project a vector onto the plane. */
		TVector3<T> ProjectVector(const TVector3<T>& v) const;

		/** Normalizes the plane's normal and the length scale of d. */
		T Normalize();

		/** Box/plane intersection. */
		bool Intersects(const TAABox<T>& box) const;

		/** Sphere/plane intersection. */
		bool Intersects(const TSphere<T>& sphere) const;

		/** Ray/plane intersection, returns boolean result and distance to intersection point. */
		std::pair<bool, T> Intersects(const TRay<T>& ray) const;

		bool operator==(const TPlane& rhs) const
		{
			return (rhs.D == D && rhs.Normal == Normal);
		}

		bool operator!=(const TPlane& rhs) const
		{
			return (rhs.D != D || rhs.Normal != Normal);
		}

		TVector3<T> Normal;
		T D;
	};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(Plane)) TPlane<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(PlaneD)) TPlane<double>;

	/** @} */
} // namespace b3d
