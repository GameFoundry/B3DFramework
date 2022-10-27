//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsDegree.h"
#include "Math/BsMath.h"

using namespace bs;

Degree::Degree(const Radian& r)
	: mDeg(r.ValueDegrees())
{}

Degree Degree::Wrap()
{
	mDeg = fmod(mDeg, 360.0f);

	if(mDeg < 0)
		mDeg += 360.0f;

	return *this;
}

Degree& Degree::operator=(const Radian& r)
{
	mDeg = r.ValueDegrees();
	return *this;
}

Degree Degree::operator+(const Radian& r) const
{
	return Degree(mDeg + r.ValueDegrees());
}

Degree& Degree::operator+=(const Radian& r)
{
	mDeg += r.ValueDegrees();
	return *this;
}

Degree Degree::operator-(const Radian& r) const
{
	return Degree(mDeg - r.ValueDegrees());
}

Degree& Degree::operator-=(const Radian& r)
{
	mDeg -= r.ValueDegrees();
	return *this;
}

float Degree::ValueRadians() const
{
	return mDeg * Math::kDeG2Rad;
}
