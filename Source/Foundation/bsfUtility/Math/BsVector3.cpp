//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "Math/BsMath.h"

namespace bs
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

	bool Vector3::isNaN() const
	{
		return Math::isNaN(x) || Math::isNaN(y) || Math::isNaN(z);
	}

	const Vector3 Vector3::ZERO{BS_ZERO()};
	const Vector3 Vector3::ONE(1, 1, 1);
	const Vector3 Vector3::INF =
		Vector3(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());

	const Vector3 Vector3::UNIT_X(1, 0, 0);
	const Vector3 Vector3::UNIT_Y(0, 1, 0);
	const Vector3 Vector3::UNIT_Z(0, 0, 1);
}
