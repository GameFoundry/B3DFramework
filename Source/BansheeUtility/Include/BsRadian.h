//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPlatformDefines.h"
#include "BsRTTIPrerequisites.h"

namespace bs
{
	class Degree;

	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Wrapper class which indicates a given angle value is in radians.
	 * 			
	 * @note 
	 * Radian values are interchangeable with Degree values, and conversions will be done automatically between them.
	 */
	class BS_UTILITY_EXPORT Radian
	{
	public:
		explicit Radian(float r = 0.0f) : mRad(r) {}
		Radian(const Degree& d);
		Radian& operator= (const float& f) { mRad = f; return *this; }
		Radian& operator= (const Radian& r) { mRad = r.mRad; return *this; }
		Radian& operator= (const Degree& d);

		/** Returns the value of the angle in degrees. */
		float valueDegrees() const;

		/** Returns the value of the angle in radians. */
		float valueRadians() const { return mRad; }

		/** Wraps the angle in [0, 2 *  PI) range. */
		Radian wrap();

        const Radian& operator+ () const { return *this; }
		Radian operator+ (const Radian& r) const { return Radian (mRad + r.mRad); }
		Radian operator+ (const Degree& d) const;
		Radian& operator+= (const Radian& r) { mRad += r.mRad; return *this; }
		Radian& operator+= (const Degree& d);
		Radian operator- () const { return Radian(-mRad); }
		Radian operator- (const Radian& r) const { return Radian (mRad - r.mRad); }
		Radian operator- (const Degree& d) const;
		Radian& operator-= (const Radian& r) { mRad -= r.mRad; return *this; }
		Radian& operator-= (const Degree& d);
		Radian operator* (float f) const { return Radian (mRad * f); }
        Radian operator* (const Radian& f) const { return Radian (mRad * f.mRad); }
		Radian& operator*= (float f) { mRad *= f; return *this; }
		Radian operator/ (float f) const { return Radian (mRad / f); }
		Radian& operator/= (float f) { mRad /= f; return *this; }

		friend Radian operator* (float lhs, const Radian& rhs) { return Radian(lhs * rhs.mRad); }
		friend Radian operator/ (float lhs, const Radian& rhs) { return Radian(lhs / rhs.mRad); }
		friend Radian operator+ (Radian& lhs, float rhs) { return Radian(lhs.mRad + rhs); }
		friend Radian operator+ (float lhs, const Radian& rhs) { return Radian(lhs + rhs.mRad); }
		friend Radian operator- (const Radian& lhs, float rhs) { return Radian(lhs.mRad - rhs); }
		friend Radian operator- (const float lhs, const Radian& rhs) { return Radian(lhs - rhs.mRad); }

		bool operator<  (const Radian& r) const { return mRad <  r.mRad; }
		bool operator<= (const Radian& r) const { return mRad <= r.mRad; }
		bool operator== (const Radian& r) const { return mRad == r.mRad; }
		bool operator!= (const Radian& r) const { return mRad != r.mRad; }
		bool operator>= (const Radian& r) const { return mRad >= r.mRad; }
		bool operator>  (const Radian& r) const { return mRad >  r.mRad; }

	private:
		float mRad;
	};

	/** @} */

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(Radian);
	/** @endcond */
}
