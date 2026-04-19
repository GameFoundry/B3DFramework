//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DRadian.h"
#include "Math/B3DMath.h"

using namespace b3d;

template<class T>
TRadian<T>::TRadian(const TDegree<T>& value)
	: mRadians(value.GetValueInRadians())
{}

template<class T>
TRadian<T> TRadian<T>::Wrap()
{
	mRadians = fmod(mRadians, Math::kTwoPi);

	if(mRadians < 0)
		mRadians += Math::kTwoPi;

	return *this;
}

template<class T>
TRadian<T>& TRadian<T>::operator=(const TDegree<T>& rhs)
{
	mRadians = rhs.GetValueInRadians();
	return *this;
}

template<class T>
TRadian<T> TRadian<T>::operator+(const TDegree<T>& rhs) const
{
	return TRadian(mRadians + rhs.GetValueInRadians());
}

template<class T>
TRadian<T>& TRadian<T>::operator+=(const TDegree<T>& rhs)
{
	mRadians += rhs.GetValueInRadians();
	return *this;
}

template<class T>
TRadian<T> TRadian<T>::operator-(const TDegree<T>& rhs) const
{
	return TRadian(mRadians - rhs.GetValueInRadians());
}

template<class T>
TRadian<T>& TRadian<T>::operator-=(const TDegree<T>& rhs)
{
	mRadians -= rhs.GetValueInRadians();
	return *this;
}

template<class T>
T TRadian<T>::GetValueInDegrees() const
{
	return mRadians * Math::kRaD2Deg;
}

template class B3D_EXPORT TRadian<float>;
template class B3D_EXPORT TRadian<double>;
