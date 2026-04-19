//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/B3DDegree.h"
#include "Math/B3DMath.h"

using namespace b3d;

template<class T>
TDegree<T>::TDegree(const TRadian<T>& value)
	: mDegrees(value.GetValueInDegrees())
{}

template<class T>
TDegree<T> TDegree<T>::Wrap()
{
	mDegrees = fmod(mDegrees, (T)360.0);

	if(mDegrees < 0)
		mDegrees += (T)360.0;

	return *this;
}

template<class T>
TDegree<T>& TDegree<T>::operator=(const TRadian<T>& rhs)
{
	mDegrees = rhs.GetValueInDegrees();
	return *this;
}

template<class T>
TDegree<T> TDegree<T>::operator+(const TRadian<T>& rhs) const
{
	return TDegree(mDegrees + rhs.GetValueInDegrees());
}

template<class T>
TDegree<T>& TDegree<T>::operator+=(const TRadian<T>& rhs)
{
	mDegrees += rhs.GetValueInDegrees();
	return *this;
}

template<class T>
TDegree<T> TDegree<T>::operator-(const TRadian<T>& rhs) const
{
	return TDegree(mDegrees - rhs.GetValueInDegrees());
}

template<class T>
TDegree<T>& TDegree<T>::operator-=(const TRadian<T>& rhs)
{
	mDegrees -= rhs.GetValueInDegrees();
	return *this;
}

template<class T>
T TDegree<T>::GetValueInRadians() const
{
	return mDegrees * Math::kDeG2Rad;
}

template class B3D_EXPORT TDegree<float>;
template class B3D_EXPORT TDegree<double>;
