//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsVector2I.h"

using namespace bs;

template<typename T> const TVector2I<T> TVector2I<T>::kZero = TVector2I<T>(BS_ZERO());

template struct B3D_UTILITY_EXPORT TVector2I<i32>;
template struct B3D_UTILITY_EXPORT TVector2I<u32>;

