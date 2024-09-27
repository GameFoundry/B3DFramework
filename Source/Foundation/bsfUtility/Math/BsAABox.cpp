//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsAABox.h"
#include "Math/BsRay.h"
#include "Math/BsPlane.h"
#include "Math/BsSphere.h"
#include "Math/BsMath.h"

using namespace bs;

template<typename T>
TAABox<T>::TAABox(const TVector3<T>& min, const TVector3<T>& max)
{
	SetExtents(min, max);
}

template<typename T>
void TAABox<T>::SetExtents(const TVector3<T>& min, const TVector3<T>& max)
{
	Minimum = min;
	Maximum = max;
}

template<typename T>
void TAABox<T>::Scale(const TVector3<T>& s)
{
	TVector3<T> center = GetCenter();
	TVector3<T> min = center + (Minimum - center) * s;
	TVector3<T> max = center + (Maximum - center) * s;

	SetExtents(min, max);
}

template<typename T>
TVector3<T> TAABox<T>::GetCorner(Corner cornerToGet) const
{
	switch(cornerToGet)
	{
	case FAR_LEFT_BOTTOM:
		return Minimum;
	case FAR_LEFT_TOP:
		return TVector3<T>(Minimum.X, Maximum.Y, Minimum.Z);
	case FAR_RIGHT_TOP:
		return TVector3<T>(Maximum.X, Maximum.Y, Minimum.Z);
	case FAR_RIGHT_BOTTOM:
		return TVector3<T>(Maximum.X, Minimum.Y, Minimum.Z);
	case NEAR_RIGHT_BOTTOM:
		return TVector3<T>(Maximum.X, Minimum.Y, Maximum.Z);
	case NEAR_LEFT_BOTTOM:
		return TVector3<T>(Minimum.X, Minimum.Y, Maximum.Z);
	case NEAR_LEFT_TOP:
		return TVector3<T>(Minimum.X, Maximum.Y, Maximum.Z);
	case NEAR_RIGHT_TOP:
		return Maximum;
	default:
		return TVector3<T>(BsZero);
	}
}

template<typename T>
void TAABox<T>::Merge(const TAABox<T>& rhs)
{
	TVector3<T> min = Minimum;
	TVector3<T> max = Maximum;
	max.Max(rhs.Maximum);
	min.Min(rhs.Minimum);

	SetExtents(min, max);
}

template<typename T>
void TAABox<T>::Merge(const TVector3<T>& point)
{
	Maximum.Max(point);
	Minimum.Min(point);
}

template<>
template<>
B3D_UTILITY_EXPORT void AABox::Transform(const Matrix4& matrix)
{
	using T = float;

	// Getting the old values so that we can use the existing merge method.
	TVector3<T> oldMin = Minimum;
	TVector3<T> oldMax = Maximum;

	TVector3<T> currentCorner;
	// We sequentially compute the corners in the following order :
	// 0, 6, 5, 1, 2, 4, 7, 3
	// This sequence allows us to only change one member at a time to get at all corners.

	// For each one, we transform it using the matrix
	// Which gives the resulting point and merge the resulting point.

	// First corner
	// min min min
	currentCorner = oldMin;
	Merge(matrix.MultiplyAffine(currentCorner));

	// min,min,max
	currentCorner.Z = oldMax.Z;
	Merge(matrix.MultiplyAffine(currentCorner));

	// min max max
	currentCorner.Y = oldMax.Y;
	Merge(matrix.MultiplyAffine(currentCorner));

	// min max min
	currentCorner.Z = oldMin.Z;
	Merge(matrix.MultiplyAffine(currentCorner));

	// max max min
	currentCorner.X = oldMax.X;
	Merge(matrix.MultiplyAffine(currentCorner));

	// max max max
	currentCorner.Z = oldMax.Z;
	Merge(matrix.MultiplyAffine(currentCorner));

	// max min max
	currentCorner.Y = oldMin.Y;
	Merge(matrix.MultiplyAffine(currentCorner));

	// max min min
	currentCorner.Z = oldMin.Z;
	Merge(matrix.MultiplyAffine(currentCorner));
}

template<>
template<>
B3D_UTILITY_EXPORT void TAABox<float>::TransformAffine<float, 0>(const Matrix4& m)
{
	using T = float;

	TVector3<T> min = m.GetTranslation();
	TVector3<T> max = m.GetTranslation();
	for(u32 i = 0; i < 3; i++)
	{
		for(u32 j = 0; j < 3; j++)
		{
			T e = m[i][j] * Minimum[j];
			T f = m[i][j] * Maximum[j];

			if(e < f)
			{
				min[i] += e;
				max[i] += f;
			}
			else
			{
				min[i] += f;
				max[i] += e;
			}
		}
	}

	SetExtents(min, max);
}

template<typename T>
bool TAABox<T>::Intersects(const TAABox<T>& b2) const
{
	// Use up to 6 separating planes
	if(Maximum.X < b2.Minimum.X)
		return false;
	if(Maximum.Y < b2.Minimum.Y)
		return false;
	if(Maximum.Z < b2.Minimum.Z)
		return false;

	if(Minimum.X > b2.Maximum.X)
		return false;
	if(Minimum.Y > b2.Maximum.Y)
		return false;
	if(Minimum.Z > b2.Maximum.Z)
		return false;

	// Otherwise, must be intersecting
	return true;
}

template<typename T>
bool TAABox<T>::Intersects(const TSphere<T>& sphere) const
{
	// Use splitting planes
	const TVector3<T>& center = sphere.Center;
	T radius = sphere.Radius;
	const TVector3<T>& min = Minimum;
	const TVector3<T>& max = Maximum;

	// Arvo's algorithm
	T s, d = (T)0.0;
	for(int i = 0; i < 3; ++i)
	{
		if(center[i] < min[i])
		{
			s = center[i] - min[i];
			d += s * s;
		}
		else if(center[i] > max[i])
		{
			s = center[i] - max[i];
			d += s * s;
		}
	}
	return d <= radius * radius;
}

template<typename T>
bool TAABox<T>::Intersects(const TPlane<T>& p) const
{
	return (p.GetSide(*this) == PlaneSide::Both);
}

template<typename T>
std::pair<bool, T> TAABox<T>::Intersects(const TRay<T>& ray) const
{
	T lowt = (T)0.0;
	T t;
	bool hit = false;
	TVector3<T> hitpoint(BsZero);
	const TVector3<T>& min = Minimum;
	const TVector3<T>& max = Maximum;
	const TVector3<T>& rayorig = ray.Origin;
	const TVector3<T>& raydir = ray.Direction;

	// Check origin inside first
	if((rayorig.X > min.X && rayorig.Y > min.Y && rayorig.Z > min.Z) && (rayorig.X < max.X && rayorig.Y < max.Y && rayorig.Z < max.Z))
	{
		return std::pair<bool, T>(true, (T)0.0);
	}

	// Check each face in turn, only check closest 3
	// Min x
	if(rayorig.X <= min.X && raydir.X > (T)0.0)
	{
		t = (min.X - rayorig.X) / raydir.X;
		if(t >= (T)0.0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if(hitpoint.Y >= min.Y && hitpoint.Y <= max.Y &&
			   hitpoint.Z >= min.Z && hitpoint.Z <= max.Z &&
			   (!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max x
	if(rayorig.X >= max.X && raydir.X < (T)0.0)
	{
		t = (max.X - rayorig.X) / raydir.X;
		if(t >= (T)0.0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if(hitpoint.Y >= min.Y && hitpoint.Y <= max.Y &&
			   hitpoint.Z >= min.Z && hitpoint.Z <= max.Z &&
			   (!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Min y
	if(rayorig.Y <= min.Y && raydir.Y > (T)0.0)
	{
		t = (min.Y - rayorig.Y) / raydir.Y;
		if(t >= (T)0.0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if(hitpoint.X >= min.X && hitpoint.X <= max.X &&
			   hitpoint.Z >= min.Z && hitpoint.Z <= max.Z &&
			   (!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max y
	if(rayorig.Y >= max.Y && raydir.Y < (T)0.0)
	{
		t = (max.Y - rayorig.Y) / raydir.Y;
		if(t >= (T)0.0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if(hitpoint.X >= min.X && hitpoint.X <= max.X &&
			   hitpoint.Z >= min.Z && hitpoint.Z <= max.Z &&
			   (!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Min z
	if(rayorig.Z <= min.Z && raydir.Z > (T)0.0)
	{
		t = (min.Z - rayorig.Z) / raydir.Z;
		if(t >= (T)0.0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if(hitpoint.X >= min.X && hitpoint.X <= max.X &&
			   hitpoint.Y >= min.Y && hitpoint.Y <= max.Y &&
			   (!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max z
	if(rayorig.Z >= max.Z && raydir.Z < (T)0.0)
	{
		t = (max.Z - rayorig.Z) / raydir.Z;
		if(t >= (T)0.0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if(hitpoint.X >= min.X && hitpoint.X <= max.X &&
			   hitpoint.Y >= min.Y && hitpoint.Y <= max.Y &&
			   (!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}

	return std::pair<bool, T>(hit, lowt);
}

template<typename T>
bool TAABox<T>::Intersects(const TRay<T>& ray, T& d1, T& d2) const
{
	const TVector3<T>& min = Minimum;
	const TVector3<T>& max = Maximum;
	const TVector3<T>& rayorig = ray.Origin;
	const TVector3<T>& raydir = ray.Direction;

	TVector3<T> absDir;
	absDir[0] = abs(raydir[0]);
	absDir[1] = abs(raydir[1]);
	absDir[2] = abs(raydir[2]);

	// Sort the axis, ensure check minimise floating error axis first
	int imax = 0, imid = 1, imin = 2;
	if(absDir[0] < absDir[2])
	{
		imax = 2;
		imin = 0;
	}
	if(absDir[1] < absDir[imin])
	{
		imid = imin;
		imin = 1;
	}
	else if(absDir[1] > absDir[imax])
	{
		imid = imax;
		imax = 1;
	}

	T start = (T)0.0, end = std::numeric_limits<T>::infinity();

#define _CALC_AXIS(i)                                      \
	do                                                     \
	{                                                      \
		T denom = (T)1.0 / raydir[i];                      \
		T newstart = (min[i] - rayorig[i]) * denom;        \
		T newend = (max[i] - rayorig[i]) * denom;          \
		if(newstart > newend) std::swap(newstart, newend); \
		if(newstart > end || newend < start) return false; \
		if(newstart > start) start = newstart;             \
		if(newend < end) end = newend;                     \
	}                                                      \
	while(0)

	// Check each axis in turn

	_CALC_AXIS(imax);

	if(absDir[imid] < std::numeric_limits<T>::epsilon())
	{
		// Parallel with middle and minimise axis, check bounds only
		if(rayorig[imid] < min[imid] || rayorig[imid] > max[imid] ||
		   rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
			return false;
	}
	else
	{
		_CALC_AXIS(imid);

		if(absDir[imin] < std::numeric_limits<T>::epsilon())
		{
			// Parallel with minimise axis, check bounds only
			if(rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
				return false;
		}
		else
		{
			_CALC_AXIS(imin);
		}
	}
#undef _CALC_AXIS

	d1 = start;
	d2 = end;

	return true;
}

template<typename T>
TVector3<T> TAABox<T>::GetCenter() const
{
	return TVector3<T>(
		(Maximum.X + Minimum.X) * (T)0.5,
		(Maximum.Y + Minimum.Y) * (T)0.5,
		(Maximum.Z + Minimum.Z) * (T)0.5);
}

template<typename T>
TVector3<T> TAABox<T>::GetSize() const
{
	return Maximum - Minimum;
}

template<typename T>
TVector3<T> TAABox<T>::GetHalfSize() const
{
	return (Maximum - Minimum) * (T)0.5;
}

template<typename T>
T TAABox<T>::GetRadius() const
{
	return ((Maximum - Minimum) * (T)0.5).Length();
}

template<typename T>
T TAABox<T>::GetVolume() const
{
	TVector3<T> diff = Maximum - Minimum;
	return diff.X * diff.Y * diff.Z;
}

template<typename T>
bool TAABox<T>::Contains(const TVector3<T>& v) const
{
	return Minimum.X <= v.X && v.X <= Maximum.X &&
		Minimum.Y <= v.Y && v.Y <= Maximum.Y &&
		Minimum.Z <= v.Z && v.Z <= Maximum.Z;
}

template<typename T>
bool TAABox<T>::Contains(const TVector3<T>& v, T extra) const
{
	return (Minimum.X - extra) <= v.X && v.X <= (Maximum.X + extra) &&
		(Minimum.Y - extra) <= v.Y && v.Y <= (Maximum.Y + extra) &&
		(Minimum.Z - extra) <= v.Z && v.Z <= (Maximum.Z + extra);
}

template<typename T>
bool TAABox<T>::Contains(const TAABox<T>& other) const
{
	return this->Minimum.X <= other.Minimum.X &&
		this->Minimum.Y <= other.Minimum.Y &&
		this->Minimum.Z <= other.Minimum.Z &&
		other.Maximum.X <= this->Maximum.X &&
		other.Maximum.Y <= this->Maximum.Y &&
		other.Maximum.Z <= this->Maximum.Z;
}

template<typename T>
bool TAABox<T>::operator==(const TAABox<T>& rhs) const
{
	return this->Minimum == rhs.Minimum && this->Maximum == rhs.Maximum;
}

template<typename T>
bool TAABox<T>::operator!=(const TAABox<T>& rhs) const
{
	return !(*this == rhs);
}

template struct B3D_UTILITY_EXPORT TAABox<float>;
template struct B3D_UTILITY_EXPORT TAABox<double>;
