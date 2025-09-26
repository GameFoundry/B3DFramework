//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DVector4.h"
#include "Math/B3DMath.h"

using namespace b3d;

template<class T>
bool TVector4<T>::IsNaN() const
{
	return Math::IsNaN(X) || Math::IsNaN(Y) || Math::IsNaN(Z) || Math::IsNaN(W);
}

template struct B3D_EXPORT TVector4<float>;
template struct B3D_EXPORT TVector4<double>;
