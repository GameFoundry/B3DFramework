//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DRay.h"
#include "Math/B3DPlane.h"
#include "Math/B3DSphere.h"
#include "Math/B3DAABox.h"
#include "Math/B3DMath.h"

using namespace b3d;

template<typename T>
void TRay<T>::Transform(const TMatrix4<T>& matrix)
{
	TVector3<T> end = GetPoint((T)1.0);

	Origin = matrix.Multiply(Origin);
	end = matrix.Multiply(end);

	Direction = TVector3<T>::Normalize(end - Origin);
}

template<typename T>
void TRay<T>::TransformAffine(const TMatrix4<T>& matrix)
{
	TVector3<T> end = GetPoint((T)1.0);

	Origin = matrix.MultiplyAffine(Origin);
	end = matrix.MultiplyAffine(end);

	Direction = TVector3<T>::Normalize(end - Origin);
}

template<typename T>
std::pair<bool, T> TRay<T>::Intersects(const TPlane<T>& p) const
{
	return p.Intersects(*this);
}

template<typename T>
std::pair<bool, T> TRay<T>::Intersects(const TSphere<T>& s) const
{
	return s.Intersects(*this);
}

template<typename T>
std::pair<bool, T> TRay<T>::Intersects(const TAABox<T>& box) const
{
	return box.Intersects(*this);
}

template<typename T>
std::pair<bool, T> TRay<T>::Intersects(const TVector3<T>& a, const TVector3<T>& b, const TVector3<T>& c, const TVector3<T>& normal, bool positiveSide, bool negativeSide) const
{
	// Calculate intersection with plane.
	T t;
	{
		T denom = normal.Dot(Direction);

		// Check intersect side
		if(denom > +std::numeric_limits<T>::epsilon())
		{
			if(!negativeSide)
				return std::pair<bool, T>(false, (T)0.0);
		}
		else if(denom < -std::numeric_limits<T>::epsilon())
		{
			if(!positiveSide)
				return std::pair<bool, T>(false, (T)0.0);
		}
		else
		{
			// Parallel or triangle area is close to zero when
			// the plane normal not normalized.
			return std::pair<bool, T>(false, (T)0.0);
		}

		t = normal.Dot(a - Origin) / denom;

		if(t < (T)0.0)
		{
			// Intersection is behind origin
			return std::pair<bool, T>(false, (T)0.0);
		}
	}

	// Calculate the largest area projection plane in X, Y or Z.
	u32 i0, i1;
	{
		T n0 = Math::Abs(normal[0]);
		T n1 = Math::Abs(normal[1]);
		T n2 = Math::Abs(normal[2]);

		i0 = 1;
		i1 = 2;
		if(n1 > n2)
		{
			if(n1 > n0) i0 = 0;
		}
		else
		{
			if(n2 > n0) i1 = 0;
		}
	}

	// Check the intersection point is inside the triangle.
	{
		T u1 = b[i0] - a[i0];
		T v1 = b[i1] - a[i1];
		T u2 = c[i0] - a[i0];
		T v2 = c[i1] - a[i1];
		T u0 = t * Direction[i0] + Origin[i0] - a[i0];
		T v0 = t * Direction[i1] + Origin[i1] - a[i1];

		T alpha = u0 * v2 - u2 * v0;
		T beta = u1 * v0 - u0 * v1;
		T area = u1 * v2 - u2 * v1;

		// Epsilon to avoid float precision errors.
		const T EPSILON = (T)1e-6;

		T tolerance = -EPSILON * area;

		if(area > 0)
		{
			if(alpha < tolerance || beta < tolerance || alpha + beta > area - tolerance)
				return std::pair<bool, T>(false, (T)0.0);
		}
		else
		{
			if(alpha > tolerance || beta > tolerance || alpha + beta < area - tolerance)
				return std::pair<bool, T>(false, (T)0.0);
		}
	}

	return std::pair<bool, T>(true, t);
}

template struct B3D_EXPORT TRay<float>;
template struct B3D_EXPORT TRay<double>;
