//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsVector3.h"
#include "Math/BsVector4.h"
#include "Math/BsMath.h"

using namespace bs;

template<class T>
TVector3<T>::TVector3(const Vector4& other)
	: X(other.X), Y(other.Y), Z(other.Z)
{
}

template struct B3D_UTILITY_EXPORT TVector3<float>;
template struct B3D_UTILITY_EXPORT TVector3<double>;
