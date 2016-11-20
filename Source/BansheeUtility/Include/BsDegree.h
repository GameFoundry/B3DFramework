//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPlatformDefines.h"
#include "BsRTTIPrerequisites.h"

namespace bs
{
	class Radian;
	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Wrapper class which indicates a given angle value is in degrees.
	 * 			
	 * @note	
	 * Degree values are interchangeable with Radian values, and conversions will be done automatically between them.
	 */
	class BS_UTILITY_EXPORT Degree
	{
	public:
		explicit Degree(float d = 0.0f) : mDeg(d) {}
		Degree(const Radian& r);
		Degree& operator= (const float& f) { mDeg = f; return *this; }
		Degree& operator= (const Degree& d) { mDeg = d.mDeg; return *this; }
		Degree& operator= (const Radian& r);

		/** Returns the value of the angle in degrees. */
		float valueDegrees() const { return mDeg; }

		/** Returns the value of the angle in radians. */
		float valueRadians() const;

		/** Wraps the angle in [0, 360) range */
		Degree wrap();

		const Degree& operator+ () const { return *this; }
		Degree operator+ (const Degree& d) const { return Degree (mDeg + d.mDeg); }
		Degree operator+ (const Radian& r) const;
		Degree& operator+= (const Degree& d) { mDeg += d.mDeg; return *this; }
		Degree& operator+= (const Radian& r);
		Degree operator- () const { return Degree(-mDeg); }
		Degree operator- (const Degree& d) const { return Degree (mDeg - d.mDeg); }
		Degree operator- (const Radian& r) const;
		Degree& operator-= (const Degree& d) { mDeg -= d.mDeg; return *this; }
		Degree& operator-= (const Radian& r);
		Degree operator* (float f) const { return Degree (mDeg * f); }
        Degree operator* (const Degree& f) const { return Degree (mDeg * f.mDeg); }
		Degree& operator*= (float f) { mDeg *= f; return *this; }
		Degree operator/ (float f) const { return Degree (mDeg / f); }
		Degree& operator/= (float f) { mDeg /= f; return *this; }

		friend Degree operator* (float lhs, const Degree& rhs) { return Degree(lhs * rhs.mDeg); }
		friend Degree operator/ (float lhs, const Degree& rhs) { return Degree(lhs / rhs.mDeg); }
		friend Degree operator+ (Degree& lhs, float rhs) { return Degree(lhs.mDeg + rhs); }
		friend Degree operator+ (float lhs, const Degree& rhs) { return Degree(lhs + rhs.mDeg); }
		friend Degree operator- (const Degree& lhs, float rhs) { return Degree(lhs.mDeg - rhs); }
		friend Degree operator- (const float lhs, const Degree& rhs) { return Degree(lhs - rhs.mDeg); }

		bool operator<  (const Degree& d) const { return mDeg <  d.mDeg; }
		bool operator<= (const Degree& d) const { return mDeg <= d.mDeg; }
		bool operator== (const Degree& d) const { return mDeg == d.mDeg; }
		bool operator!= (const Degree& d) const { return mDeg != d.mDeg; }
		bool operator>= (const Degree& d) const { return mDeg >= d.mDeg; }
		bool operator>  (const Degree& d) const { return mDeg >  d.mDeg; }

	private:
		float mDeg;
	};

	/** @} */

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(Degree);
	/** @endcond */
}
