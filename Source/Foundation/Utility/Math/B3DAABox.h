//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DVector3.h"
#include "Math/B3DMatrix4.h"

namespace b3d
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Axis aligned box represented by minimum and maximum point. */
	template<typename T>
	struct TAABox
	{
		/** Different corners of a box. */
		/*
		   1-----2
		  /|    /|
		 / |   / |
		5-----4  |
		|  0--|--3
		| /   | /
		|/    |/
		6-----7
		*/
		enum Corner
		{
			FAR_LEFT_BOTTOM = 0,
			FAR_LEFT_TOP = 1,
			FAR_RIGHT_TOP = 2,
			FAR_RIGHT_BOTTOM = 3,
			NEAR_RIGHT_BOTTOM = 7,
			NEAR_LEFT_BOTTOM = 6,
			NEAR_LEFT_TOP = 5,
			NEAR_RIGHT_TOP = 4
		};

		TVector3<T> Minimum;
		TVector3<T> Maximum;

		TAABox()
			: Minimum(TVector3<T>::kZero), Maximum(TVector3<T>::kZero)
		{ }

		B3D_SCRIPT_EXPORT(Exclude(true))
		TAABox(const TAABox& copy) = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		TAABox(const TVector3<T>& min, const TVector3<T>& max);

		~TAABox() = default;

		/** Sets the minimum and maximum corners. */
		void SetExtents(const TVector3<T>& min, const TVector3<T>& max);

		/** Scales the box around the center by multiplying its extents with the provided scale. */
		void Scale(const TVector3<T>& s);

		/** Returns the coordinates of a specific corner. */
		TVector3<T> GetCorner(Corner cornerToGet) const;

		/** Merges the two boxes, creating a new bounding box that encapsulates them both. */
		void Merge(const TAABox<T>& rhs);

		/** Expands the bounding box so it includes the provided point. */
		void Merge(const TVector3<T>& point);

		/**
		 * Transforms the bounding box by the given matrix.
		 *
		 * @note
		 * As the resulting box will no longer be axis aligned, an axis align box
		 * is instead created by encompassing the transformed oriented bounding box.
		 * Retrieving the value as an actual OBB would provide a tighter fit.
		 */
		void Transform(const TMatrix4<T>& matrix);

		/**
		 * Transforms the bounding box by the given matrix.
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

		/** Returns true if this and the provided box intersect. */
		bool Intersects(const TAABox<T>& b2) const;

		/** Returns true if the sphere intersects the bounding box. */
		bool Intersects(const TSphere<T>& s) const;

		/** Returns true if the plane intersects the bounding box. */
		bool Intersects(const TPlane<T>& p) const;

		/** Ray / box intersection, returns a boolean result and nearest distance to intersection. */
		std::pair<bool, T> Intersects(const TRay<T>& ray) const;

		/** Ray / box intersection, returns boolean result and near and far intersection distance. */
		bool Intersects(const TRay<T>& ray, T& d1, T& d2) const;

		/** Center of the box. */
		TVector3<T> GetCenter() const;

		/** Size of the box (difference between minimum and maximum corners) */
		TVector3<T> GetSize() const;

		/** Extents of the box (distance from center to one of the corners) */
		TVector3<T> GetExtents() const;

		/** Radius of a sphere that fully encompasses the box. */
		T GetRadius() const;

		/** Size of the volume in the box. */
		T GetVolume() const;

		/** Returns true if the provided point is inside the bounding box. */
		bool Contains(const TVector3<T>& v) const;

		/**
		 * Returns true if the provided point is inside the bounding box while expanding the bounds by @p extra in every
		 * direction.
		 */
		bool Contains(const TVector3<T>& v, T extra) const;

		/** Returns true if the provided bounding box is completely inside the bounding box. */
		bool Contains(const TAABox<T>& other) const;

		bool operator==(const TAABox<T>& rhs) const;
		bool operator!=(const TAABox<T>& rhs) const;

		static const TAABox<T> kEmpty;
		static const TAABox<T> kUnit;
		static const TAABox<T> kInfinite;

		/**
		 * Indices that can be used for rendering a box constructed from 8 corner vertices, using AABox::Corner for
		 * mapping.
		 */
		static const u32 kCubeIndices[36];
	};

	template<> const TAABox<float> TAABox<float>::kEmpty = TAABox(TVector3<float>(0.0f, 0.0f, 0.0f), TVector3<float>(0.0f, 0.0f, 0.0f));
	template<> const TAABox<double> TAABox<double>::kEmpty = TAABox(TVector3<double>(0.0, 0.0, 0.0), TVector3<double>(0.0, 0.0, 0.0));

	template<> const TAABox<float> TAABox<float>::kUnit = TAABox(TVector3<float>(-0.5f, -0.5f, -0.5f), TVector3<float>(0.5f, 0.5f, 0.5f));
	template<> const TAABox<double> TAABox<double>::kUnit = TAABox(TVector3<double>(-0.5, -0.5, -0.5), TVector3<double>(0.5, 0.5, 0.5));

	template<> const TAABox<float> TAABox<float>::kInfinite = TAABox(
		TVector3<float>(
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity(),
			std::numeric_limits<float>::infinity()),
		TVector3<float>(
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity(),
			-std::numeric_limits<float>::infinity()));

	template<> const TAABox<double> TAABox<double>::kInfinite = TAABox(
		TVector3<double>(
			std::numeric_limits<double>::infinity(),
			std::numeric_limits<double>::infinity(),
			std::numeric_limits<double>::infinity()),
		TVector3<double>(
			-std::numeric_limits<double>::infinity(),
			-std::numeric_limits<double>::infinity(),
			-std::numeric_limits<double>::infinity()));

	template<> const u32 TAABox<float>::kCubeIndices[36] = {
		// Near
		NEAR_LEFT_BOTTOM, NEAR_LEFT_TOP, NEAR_RIGHT_TOP,
		NEAR_LEFT_BOTTOM, NEAR_RIGHT_TOP, NEAR_RIGHT_BOTTOM,

		// Far
		FAR_RIGHT_BOTTOM, FAR_RIGHT_TOP, FAR_LEFT_TOP,
		FAR_RIGHT_BOTTOM, FAR_LEFT_TOP, FAR_LEFT_BOTTOM,

		// Left
		FAR_LEFT_BOTTOM, FAR_LEFT_TOP, NEAR_LEFT_TOP,
		FAR_LEFT_BOTTOM, NEAR_LEFT_TOP, NEAR_LEFT_BOTTOM,

		// Right
		NEAR_RIGHT_BOTTOM, NEAR_RIGHT_TOP, FAR_RIGHT_TOP,
		NEAR_RIGHT_BOTTOM, FAR_RIGHT_TOP, FAR_RIGHT_BOTTOM,

		// Top
		FAR_LEFT_TOP, FAR_RIGHT_TOP, NEAR_RIGHT_TOP,
		FAR_LEFT_TOP, NEAR_RIGHT_TOP, NEAR_LEFT_TOP,

		// Bottom
		NEAR_LEFT_BOTTOM, NEAR_RIGHT_BOTTOM, FAR_RIGHT_BOTTOM,
		NEAR_LEFT_BOTTOM, FAR_RIGHT_BOTTOM, FAR_LEFT_BOTTOM
	};

	template<> const u32 TAABox<double>::kCubeIndices[36] = {
		// Near
		NEAR_LEFT_BOTTOM, NEAR_LEFT_TOP, NEAR_RIGHT_TOP,
		NEAR_LEFT_BOTTOM, NEAR_RIGHT_TOP, NEAR_RIGHT_BOTTOM,

		// Far
		FAR_RIGHT_BOTTOM, FAR_RIGHT_TOP, FAR_LEFT_TOP,
		FAR_RIGHT_BOTTOM, FAR_LEFT_TOP, FAR_LEFT_BOTTOM,

		// Left
		FAR_LEFT_BOTTOM, FAR_LEFT_TOP, NEAR_LEFT_TOP,
		FAR_LEFT_BOTTOM, NEAR_LEFT_TOP, NEAR_LEFT_BOTTOM,

		// Right
		NEAR_RIGHT_BOTTOM, NEAR_RIGHT_TOP, FAR_RIGHT_TOP,
		NEAR_RIGHT_BOTTOM, FAR_RIGHT_TOP, FAR_RIGHT_BOTTOM,

		// Top
		FAR_LEFT_TOP, FAR_RIGHT_TOP, NEAR_RIGHT_TOP,
		FAR_LEFT_TOP, NEAR_RIGHT_TOP, NEAR_LEFT_TOP,

		// Bottom
		NEAR_LEFT_BOTTOM, NEAR_RIGHT_BOTTOM, FAR_RIGHT_BOTTOM,
		NEAR_LEFT_BOTTOM, FAR_RIGHT_BOTTOM, FAR_LEFT_BOTTOM
	};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(AABox)) TAABox<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true), ExportName(AABoxD)) TAABox<double>;

	/** @} */
} // namespace b3d
