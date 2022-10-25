//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "Math/BsMath.h"

using namespace bs;

Vector3::Vector3(const Vector4& vec)
	: X(vec.X), Y(vec.Y), Z(vec.Z)
{
}

Radian Vector3::AngleBetween(const Vector3& dest) const
{
	float lenProduct = Length() * dest.Length();

	// Divide by zero check
	if(lenProduct < 1e-6f)
		lenProduct = 1e-6f;

	float f = Dot(dest) / lenProduct;

	f = Math::Clamp(f, -1.0f, 1.0f);
	return Math::Acos(f);
}

bool Vector3::IsNaN() const
{
	return Math::IsNaN(X) || Math::IsNaN(Y) || Math::IsNaN(Z);
}

const Vector3 Vector3::ZERO{ BS_ZERO() };
const Vector3 Vector3::ONE(1, 1, 1);
const Vector3 Vector3::INF =
	Vector3(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());

const Vector3 Vector3::UNIT_X(1, 0, 0);
const Vector3 Vector3::UNIT_Y(0, 1, 0);
const Vector3 Vector3::UNIT_Z(0, 0, 1);
