//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Math/BsRadian.h"
#include "Math/BsMath.h"

namespace bs
{
	Radian::Radian (const Degree& d) : mRad(d.valueRadians())
	{ }

	Radian Radian::wrap()
	{
		mRad = fmod(mRad, Math::TWO_PI);

		if (mRad < 0)
			mRad += Math::TWO_PI;

		return *this;
	}

	Radian& Radian::operator= (const Degree& d)
	{
		mRad = d.valueRadians();
		return *this;
	}

	Radian Radian::operator+ (const Degree& d) const
	{
		return Radian (mRad + d.valueRadians());
	}

	Radian& Radian::operator+= (const Degree& d)
	{
		mRad += d.valueRadians();
		return *this;
	}

	Radian Radian::operator- (const Degree& d) const
	{
		return Radian (mRad - d.valueRadians());
	}

	Radian& Radian::operator-= (const Degree& d)
	{
		mRad -= d.valueRadians();
		return *this;
	}

	float Radian::valueDegrees() const
	{
		return mRad * Math::RAD2DEG;
	}
}
