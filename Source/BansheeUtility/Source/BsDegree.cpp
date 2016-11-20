//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsDegree.h"
#include "BsMath.h"

namespace bs
{
	Degree::Degree(const Radian& r) 
		:mDeg(r.valueDegrees()) 
	{ }

	inline Degree Degree::wrap()
	{
		mDeg = fmod(mDeg, 360.0f);

		if (mDeg < 0)
			mDeg += 360.0f;

		return *this;
	}

	Degree& Degree::operator= (const Radian& r) 
	{ 
		mDeg = r.valueDegrees(); return *this; 
	}

	Degree Degree::operator+ (const Radian& r) const 
	{ 
		return Degree (mDeg + r.valueDegrees()); 
	}

	Degree& Degree::operator+= (const Radian& r) 
	{ 
		mDeg += r.valueDegrees(); return *this; 
	}

	Degree Degree::operator- (const Radian& r) const 
	{ 
		return Degree (mDeg - r.valueDegrees()); 
	}

	Degree& Degree::operator-= (const Radian& r) 
	{ 
		mDeg -= r.valueDegrees(); return *this; 
	}

	inline float Degree::valueRadians() const
	{
		return mDeg * Math::DEG2RAD;
	}
}