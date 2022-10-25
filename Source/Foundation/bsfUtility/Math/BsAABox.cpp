//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsAABox.h"
#include "Math/BsRay.h"
#include "Math/BsPlane.h"
#include "Math/BsSphere.h"
#include "Math/BsMath.h"

using namespace bs;

const AABox AABox::BOX_EMPTY = AABox(Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.0f));
const AABox AABox::UNIT_BOX = AABox(Vector3(-0.5f, -0.5f, -0.5f), Vector3(0.5f, 0.5f, 0.5f));
const AABox AABox::INF_BOX = AABox(
	Vector3(
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::infinity(),
		std::numeric_limits<float>::infinity()),
	Vector3(
		-std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity(),
		-std::numeric_limits<float>::infinity()));

const u32 AABox::CUBE_INDICES[36] = {
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

AABox::AABox()
{
	// Default to a unit box
	SetMin(Vector3(-0.5f, -0.5f, -0.5f));
	SetMax(Vector3(0.5f, 0.5f, 0.5f));
}

AABox::AABox(const Vector3& min, const Vector3& max)
{
	SetExtents(min, max);
}

void AABox::SetExtents(const Vector3& min, const Vector3& max)
{
	mMinimum = min;
	mMaximum = max;
}

void AABox::Scale(const Vector3& s)
{
	Vector3 center = GetCenter();
	Vector3 min = center + (mMinimum - center) * s;
	Vector3 max = center + (mMaximum - center) * s;

	SetExtents(min, max);
}

Vector3 AABox::GetCorner(Corner cornerToGet) const
{
	switch(cornerToGet)
	{
	case FAR_LEFT_BOTTOM:
		return mMinimum;
	case FAR_LEFT_TOP:
		return Vector3(mMinimum.X, mMaximum.Y, mMinimum.Z);
	case FAR_RIGHT_TOP:
		return Vector3(mMaximum.X, mMaximum.Y, mMinimum.Z);
	case FAR_RIGHT_BOTTOM:
		return Vector3(mMaximum.X, mMinimum.Y, mMinimum.Z);
	case NEAR_RIGHT_BOTTOM:
		return Vector3(mMaximum.X, mMinimum.Y, mMaximum.Z);
	case NEAR_LEFT_BOTTOM:
		return Vector3(mMinimum.X, mMinimum.Y, mMaximum.Z);
	case NEAR_LEFT_TOP:
		return Vector3(mMinimum.X, mMaximum.Y, mMaximum.Z);
	case NEAR_RIGHT_TOP:
		return mMaximum;
	default:
		return Vector3(BsZero);
	}
}

void AABox::Merge(const AABox& rhs)
{
	Vector3 min = mMinimum;
	Vector3 max = mMaximum;
	max.Max(rhs.mMaximum);
	min.Min(rhs.mMinimum);

	SetExtents(min, max);
}

void AABox::Merge(const Vector3& point)
{
	mMaximum.Max(point);
	mMinimum.Min(point);
}

void AABox::Transform(const Matrix4& matrix)
{
	// Getting the old values so that we can use the existing merge method.
	Vector3 oldMin = mMinimum;
	Vector3 oldMax = mMaximum;

	Vector3 currentCorner;
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

void AABox::TransformAffine(const Matrix4& m)
{
	Vector3 min = m.GetTranslation();
	Vector3 max = m.GetTranslation();
	for(u32 i = 0; i < 3; i++)
	{
		for(u32 j = 0; j < 3; j++)
		{
			float e = m[i][j] * mMinimum[j];
			float f = m[i][j] * mMaximum[j];

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

bool AABox::Intersects(const AABox& b2) const
{
	// Use up to 6 separating planes
	if(mMaximum.X < b2.mMinimum.X)
		return false;
	if(mMaximum.Y < b2.mMinimum.Y)
		return false;
	if(mMaximum.Z < b2.mMinimum.Z)
		return false;

	if(mMinimum.X > b2.mMaximum.X)
		return false;
	if(mMinimum.Y > b2.mMaximum.Y)
		return false;
	if(mMinimum.Z > b2.mMaximum.Z)
		return false;

	// Otherwise, must be intersecting
	return true;
}

bool AABox::Intersects(const Sphere& sphere) const
{
	// Use splitting planes
	const Vector3& center = sphere.GetCenter();
	float radius = sphere.GetRadius();
	const Vector3& min = GetMin();
	const Vector3& max = GetMax();

	// Arvo's algorithm
	float s, d = 0;
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

bool AABox::Intersects(const Plane& p) const
{
	return (p.GetSide(*this) == Plane::BOTH_SIDE);
}

std::pair<bool, float> AABox::Intersects(const Ray& ray) const
{
	float lowt = 0.0f;
	float t;
	bool hit = false;
	Vector3 hitpoint(BsZero);
	const Vector3& min = GetMin();
	const Vector3& max = GetMax();
	const Vector3& rayorig = ray.GetOrigin();
	const Vector3& raydir = ray.GetDirection();

	// Check origin inside first
	if((rayorig.X > min.X && rayorig.Y > min.Y && rayorig.Z > min.Z) && (rayorig.X < max.X && rayorig.Y < max.Y && rayorig.Z < max.Z))
	{
		return std::pair<bool, float>(true, 0.0f);
	}

	// Check each face in turn, only check closest 3
	// Min x
	if(rayorig.X <= min.X && raydir.X > 0)
	{
		t = (min.X - rayorig.X) / raydir.X;
		if(t >= 0)
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
	if(rayorig.X >= max.X && raydir.X < 0)
	{
		t = (max.X - rayorig.X) / raydir.X;
		if(t >= 0)
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
	if(rayorig.Y <= min.Y && raydir.Y > 0)
	{
		t = (min.Y - rayorig.Y) / raydir.Y;
		if(t >= 0)
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
	if(rayorig.Y >= max.Y && raydir.Y < 0)
	{
		t = (max.Y - rayorig.Y) / raydir.Y;
		if(t >= 0)
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
	if(rayorig.Z <= min.Z && raydir.Z > 0)
	{
		t = (min.Z - rayorig.Z) / raydir.Z;
		if(t >= 0)
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
	if(rayorig.Z >= max.Z && raydir.Z < 0)
	{
		t = (max.Z - rayorig.Z) / raydir.Z;
		if(t >= 0)
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

	return std::pair<bool, float>(hit, lowt);
}

bool AABox::Intersects(const Ray& ray, float& d1, float& d2) const
{
	const Vector3& min = GetMin();
	const Vector3& max = GetMax();
	const Vector3& rayorig = ray.GetOrigin();
	const Vector3& raydir = ray.GetDirection();

	Vector3 absDir;
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

	float start = 0, end = Math::POS_INFINITY;

#define _CALC_AXIS(i)                                      \
	do                                                     \
	{                                                      \
		float denom = 1 / raydir[i];                       \
		float newstart = (min[i] - rayorig[i]) * denom;    \
		float newend = (max[i] - rayorig[i]) * denom;      \
		if(newstart > newend) std::swap(newstart, newend); \
		if(newstart > end || newend < start) return false; \
		if(newstart > start) start = newstart;             \
		if(newend < end) end = newend;                     \
	}                                                      \
	while(0)

	// Check each axis in turn

	_CALC_AXIS(imax);

	if(absDir[imid] < std::numeric_limits<float>::epsilon())
	{
		// Parallel with middle and minimise axis, check bounds only
		if(rayorig[imid] < min[imid] || rayorig[imid] > max[imid] ||
		   rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
			return false;
	}
	else
	{
		_CALC_AXIS(imid);

		if(absDir[imin] < std::numeric_limits<float>::epsilon())
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

Vector3 AABox::GetCenter() const
{
	return Vector3(
		(mMaximum.X + mMinimum.X) * 0.5f,
		(mMaximum.Y + mMinimum.Y) * 0.5f,
		(mMaximum.Z + mMinimum.Z) * 0.5f);
}

Vector3 AABox::GetSize() const
{
	return mMaximum - mMinimum;
}

Vector3 AABox::GetHalfSize() const
{
	return (mMaximum - mMinimum) * 0.5;
}

float AABox::GetRadius() const
{
	return ((mMaximum - mMinimum) * 0.5).Length();
}

float AABox::GetVolume() const
{
	Vector3 diff = mMaximum - mMinimum;
	return diff.X * diff.Y * diff.Z;
}

bool AABox::Contains(const Vector3& v) const
{
	return mMinimum.X <= v.X && v.X <= mMaximum.X &&
		mMinimum.Y <= v.Y && v.Y <= mMaximum.Y &&
		mMinimum.Z <= v.Z && v.Z <= mMaximum.Z;
}

bool AABox::Contains(const Vector3& v, float extra) const
{
	return (mMinimum.X - extra) <= v.X && v.X <= (mMaximum.X + extra) &&
		(mMinimum.Y - extra) <= v.Y && v.Y <= (mMaximum.Y + extra) &&
		(mMinimum.Z - extra) <= v.Z && v.Z <= (mMaximum.Z + extra);
}

bool AABox::Contains(const AABox& other) const
{
	return this->mMinimum.X <= other.mMinimum.X &&
		this->mMinimum.Y <= other.mMinimum.Y &&
		this->mMinimum.Z <= other.mMinimum.Z &&
		other.mMaximum.X <= this->mMaximum.X &&
		other.mMaximum.Y <= this->mMaximum.Y &&
		other.mMaximum.Z <= this->mMaximum.Z;
}

bool AABox::operator==(const AABox& rhs) const
{
	return this->mMinimum == rhs.mMinimum &&
		this->mMaximum == rhs.mMaximum;
}

bool AABox::operator!=(const AABox& rhs) const
{
	return !(*this == rhs);
}
