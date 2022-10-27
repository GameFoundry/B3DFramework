//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsVector4.h"
#include "Math/BsMath.h"

using namespace bs;

const Vector4 Vector4::kZero{ BS_ZERO() };

bool Vector4::IsNaN() const
{
	return Math::IsNaN(X) || Math::IsNaN(Y) || Math::IsNaN(Z) || Math::IsNaN(W);
}
