//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVector3.h"
#include "BsVector4.h"
#include "BsMath.h"

namespace BansheeEngine
{
	Vector3::Vector3(const Vector4& vec)
		:x(vec.x), y(vec.y), z(vec.z)
	{

	}

	Radian Vector3::angleBetween(const Vector3& dest) const
	{
		float lenProduct = length() * dest.length();

		// Divide by zero check
		if (lenProduct < 1e-6f)
			lenProduct = 1e-6f;

		float f = dot(dest) / lenProduct;

		f = Math::clamp(f, -1.0f, 1.0f);
		return Math::acos(f);
	}

	Vector3 Vector3::normalize(const Vector3& val)
	{
		float len = Math::sqrt(val.x * val.x + val.y * val.y + val.z * val.z);

		// Will also work for zero-sized vectors, but will change nothing
		if (len > 1e-08)
		{
			float invLen = 1.0f / len;

			Vector3 normalizedVec;
			normalizedVec.x = val.x * invLen;
			normalizedVec.y = val.y * invLen;
			normalizedVec.z = val.z * invLen;

			return normalizedVec;
		}
		else
			return val;
	}

	bool Vector3::isNaN() const
	{
		return Math::isNaN(x) || Math::isNaN(y) || Math::isNaN(z);
	}

    const Vector3 Vector3::ZERO(0, 0, 0);
	const Vector3 Vector3::ONE(1, 1, 1);
	const Vector3 Vector3::INF =
		Vector3(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());

    const Vector3 Vector3::UNIT_X(1, 0, 0);
    const Vector3 Vector3::UNIT_Y(0, 1, 0);
    const Vector3 Vector3::UNIT_Z(0, 0, 1);    
}
