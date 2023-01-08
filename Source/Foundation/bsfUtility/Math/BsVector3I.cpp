//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsVector3I.h"

using namespace bs;

template<typename T> const TVector3I<T> TVector3I<T>::kZero = TVector3I<T>(BS_ZERO());

template struct B3D_UTILITY_EXPORT TVector3I<i32>;
template struct B3D_UTILITY_EXPORT TVector3I<u32>;

