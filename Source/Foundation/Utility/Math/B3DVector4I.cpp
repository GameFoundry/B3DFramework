//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DVector4I.h"

using namespace b3d;

template<typename T> const TVector4I<T> TVector4I<T>::kZero = TVector4I<T>(BS_ZERO());

template struct B3D_EXPORT TVector4I<i32>;
template struct B3D_EXPORT TVector4I<u32>;

