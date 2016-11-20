//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"
#include "BsVector3.h"
#include "BsMatrix4.h"

namespace bs 
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Axis aligned box represented by minimum and maximum point. */
	class BS_UTILITY_EXPORT AABox
	{
	public:
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
		enum CornerEnum 
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

		AABox();
		AABox(const AABox& copy);
		AABox(const Vector3& min, const Vector3& max);

		AABox& operator=(const AABox& rhs);

		~AABox() { }
		
		/** Gets the corner of the box with minimum values (opposite to maximum corner). */
		const Vector3& getMin() const { return mMinimum; }

		/** Gets the corner of the box with maximum values (opposite to minimum corner). */
		const Vector3& getMax() const { return mMaximum; }

		/** Sets the corner of the box with minimum values (opposite to maximum corner). */
		void setMin(const Vector3& vec) { mMinimum = vec; }

		/** Sets the corner of the box with maximum values (opposite to minimum corner). */
		void setMax(const Vector3& vec) { mMaximum = vec; }

		/** Sets the minimum and maximum corners. */
		void setExtents(const Vector3& min, const Vector3& max);

		/** Scales the box around the center by multiplying its extents with the provided scale. */
		void scale(const Vector3& s);

		/** Returns the coordinates of a specific corner. */
		Vector3 getCorner(CornerEnum cornerToGet) const;

		/** Merges the two boxes, creating a new bounding box that encapsulates them both. */
		void merge(const AABox& rhs);

		/** Expands the bounding box so it includes the provided point. */
		void merge(const Vector3& point);

		/**
		 * Transforms the bounding box by the given matrix.
		 *
		 * @note	
		 * As the resulting box will no longer be axis aligned, an axis align box 
		 * is instead created by encompassing the transformed oriented bounding box.
		 * Retrieving the value as an actual OBB would provide a tighter fit.
		 */
		void transform(const Matrix4& matrix);

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
		void transformAffine(const Matrix4& matrix);

		/** Returns true if this and the provided box intersect. */
		bool intersects(const AABox& b2) const;

		/** Returns true if the sphere intersects the bounding box. */
		bool intersects(const Sphere& s) const;

		/** Returns true if the plane intersects the bounding box. */
		bool intersects(const Plane& p) const;

        /** Ray / box intersection, returns a boolean result and nearest distance to intersection. */
        std::pair<bool, float> intersects(const Ray& ray) const;

        /** Ray / box intersection, returns boolean result and near and far intersection distance. */
        bool intersects(const Ray& ray, float& d1, float& d2) const;

		/** Center of the box. */
		Vector3 getCenter() const;

		/** Size of the box (difference between minimum and maximum corners) */
		Vector3 getSize() const;

		/** Extents of the box (distance from center to one of the corners) */
		Vector3 getHalfSize() const;

		/** Radius of a sphere that fully encompasses the box. */
		float getRadius() const;

		/** Size of the volume in the box. */
		float getVolume() const;

        /** Returns true if the provided point is inside the bounding box. */
        bool contains(const Vector3& v) const;

        /** Returns true if the provided bounding box is completely inside the bounding box. */
        bool contains(const AABox& other) const;

        bool operator== (const AABox& rhs) const;
        bool operator!= (const AABox& rhs) const;

		static const AABox BOX_EMPTY;

	protected:
		Vector3 mMinimum;
		Vector3 mMaximum;
	};

	/** @} */

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(AABox);
	/** @endcond */
}
