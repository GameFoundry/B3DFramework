//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsRadian.h"
#include "Math/BsMath.h"

using namespace bs;

Radian::Radian(const Degree& d)
	: mRad(d.ValueRadians())
{}

Radian Radian::Wrap()
{
	mRad = fmod(mRad, Math::kTwoPi);

	if(mRad < 0)
		mRad += Math::kTwoPi;

	return *this;
}

Radian& Radian::operator=(const Degree& d)
{
	mRad = d.ValueRadians();
	return *this;
}

Radian Radian::operator+(const Degree& d) const
{
	return Radian(mRad + d.ValueRadians());
}

Radian& Radian::operator+=(const Degree& d)
{
	mRad += d.ValueRadians();
	return *this;
}

Radian Radian::operator-(const Degree& d) const
{
	return Radian(mRad - d.ValueRadians());
}

Radian& Radian::operator-=(const Degree& d)
{
	mRad -= d.ValueRadians();
	return *this;
}

float Radian::ValueDegrees() const
{
	return mRad * Math::kRaD2Deg;
}
