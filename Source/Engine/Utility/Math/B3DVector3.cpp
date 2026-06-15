//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DVector3.h"
#include "Math/B3DVector4.h"
#include "Math/B3DMath.h"

using namespace b3d;

template<class T>
TVector3<T>::TVector3(const Vector4& other)
	: X(other.X), Y(other.Y), Z(other.Z)
{
}

namespace b3d
{
	template struct B3D_EXPORT TVector3<float>;
	template struct B3D_EXPORT TVector3<double>;
} // namespace b3d
