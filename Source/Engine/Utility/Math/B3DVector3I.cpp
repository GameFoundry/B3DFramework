//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DVector3I.h"

using namespace b3d;

template<typename T> const TVector3I<T> TVector3I<T>::kZero = TVector3I<T>(ZeroTag());

namespace b3d
{
	template struct B3D_EXPORT TVector3I<i32>;
	template struct B3D_EXPORT TVector3I<u32>;
} // namespace b3d

