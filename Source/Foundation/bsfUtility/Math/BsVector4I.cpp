//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsVector4I.h"

using namespace bs;

template<typename T> const TVector4I<T> TVector4I<T>::kZero = TVector4I<T>(BS_ZERO());

template struct B3D_UTILITY_EXPORT TVector4I<i32>;
template struct B3D_UTILITY_EXPORT TVector4I<u32>;

