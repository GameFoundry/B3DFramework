//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsRay.h"
#include "BsPlane.h"
#include "BsSphere.h"
#include "BsAABox.h"
#include "BsMath.h"

namespace bs
{
	void Ray::transform(const Matrix4& matrix)
	{
		Vector3 end = getPoint(1.0f);

		mOrigin = matrix.multiply(mOrigin);
		end = matrix.multiply(end);

		mDirection = Vector3::normalize(end - mOrigin);
	}

	void Ray::transformAffine(const Matrix4& matrix)
	{
		Vector3 end = getPoint(1.0f);

		mOrigin = matrix.multiplyAffine(mOrigin);
		end = matrix.multiplyAffine(end);

		mDirection = Vector3::normalize(end - mOrigin);
	}

	std::pair<bool, float> Ray::intersects(const Plane& p) const
	{
		return p.intersects(*this);
	}

	std::pair<bool, float> Ray::intersects(const Sphere& s) const
	{
		return s.intersects(*this);
	}

	std::pair<bool, float> Ray::intersects(const AABox& box) const
	{
		return box.intersects(*this);
	}

	std::pair<bool, float> Ray::intersects(const Vector3& a,
		const Vector3& b, const Vector3& c, const Vector3& normal,
		bool positiveSide, bool negativeSide) const
	{
		// Calculate intersection with plane.
		float t;
		{
			float denom = normal.dot(getDirection());

			// Check intersect side
			if (denom > + std::numeric_limits<float>::epsilon())
			{
				if (!negativeSide)
					return std::pair<bool, float>(false, 0.0f);
			}
			else if (denom < - std::numeric_limits<float>::epsilon())
			{
				if (!positiveSide)
					return std::pair<bool, float>(false, 0.0f);
			}
			else
			{
				// Parallel or triangle area is close to zero when
				// the plane normal not normalized.
				return std::pair<bool, float>(false, 0.0f);
			}

			t = normal.dot(a - getOrigin()) / denom;

			if (t < 0)
			{
				// Intersection is behind origin
				return std::pair<bool, float>(false, 0.0f);
			}
		}

		// Calculate the largest area projection plane in X, Y or Z.
		UINT32 i0, i1;
		{
			float n0 = Math::abs(normal[0]);
			float n1 = Math::abs(normal[1]);
			float n2 = Math::abs(normal[2]);

			i0 = 1; i1 = 2;
			if (n1 > n2)
			{
				if (n1 > n0) i0 = 0;
			}
			else
			{
				if (n2 > n0) i1 = 0;
			}
		}

		// Check the intersection point is inside the triangle.
		{
			float u1 = b[i0] - a[i0];
			float v1 = b[i1] - a[i1];
			float u2 = c[i0] - a[i0];
			float v2 = c[i1] - a[i1];
			float u0 = t * getDirection()[i0] + getOrigin()[i0] - a[i0];
			float v0 = t * getDirection()[i1] + getOrigin()[i1] - a[i1];

			float alpha = u0 * v2 - u2 * v0;
			float beta  = u1 * v0 - u0 * v1;
			float area  = u1 * v2 - u2 * v1;

			// Epsilon to avoid float precision errors.
			const float EPSILON = 1e-6f;

			float tolerance = - EPSILON * area;

			if (area > 0)
			{
				if (alpha < tolerance || beta < tolerance || alpha+beta > area-tolerance)
					return std::pair<bool, float>(false, 0.0f);
			}
			else
			{
				if (alpha > tolerance || beta > tolerance || alpha+beta < area-tolerance)
					return std::pair<bool, float>(false, 0.0f);
			}
		}

		return std::pair<bool, float>(true, t);
	}
}