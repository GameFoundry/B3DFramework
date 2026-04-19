//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	template<class T>
	class TDegree;

	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Wrapper class which indicates a given angle value is in radians.
	 *
	 * @note
	 * Radian values are interchangeable with Degree values, and conversions will be done automatically between them.
	 */
	template<class T>
	class TRadian
	{
	public:
		constexpr TRadian() = default;
		constexpr TRadian(const TRadian& value) = default;
		TRadian(const TDegree<T>& value);

		constexpr explicit TRadian(T radians)
			: mRadians(radians) {}

		TRadian& operator=(const TDegree<T>& rhs);
		constexpr TRadian& operator=(const TRadian& rhs) = default;
		constexpr TRadian& operator=(const T& rhs)
		{
			mRadians = rhs;
			return *this;
		}


		/** Returns the value of the angle in degrees. */
		T GetValueInDegrees() const;

		/** Returns the value of the angle in radians. */
		constexpr T GetValueInRadians() const { return mRadians; }

		/** Wraps the angle in [0, 2 *  PI) range. */
		TRadian Wrap();

		const TRadian& operator+() const { return *this; }

		TRadian operator+(const TRadian& rhs) const { return TRadian(mRadians + rhs.mRadians); }
		TRadian operator+(const TDegree<T>& rhs) const;

		TRadian& operator+=(const TDegree<T>& rhs);
		TRadian& operator+=(const TRadian& rhs)
		{
			mRadians += rhs.mRadians;
			return *this;
		}

		TRadian operator-() const { return TRadian(-mRadians); }

		TRadian operator-(const TDegree<T>& rhs) const;
		TRadian operator-(const TRadian& rhs) const { return TRadian(mRadians - rhs.mRadians); }

		TRadian& operator-=(const TDegree<T>& rhs);
		TRadian& operator-=(const TRadian& rhs)
		{
			mRadians -= rhs.mRadians;
			return *this;
		}

		TRadian operator*(T rhs) const { return TRadian(mRadians * rhs); }
		TRadian operator*(const TRadian& rhs) const { return TRadian(mRadians * rhs.mRadians); }

		TRadian& operator*=(T rhs)
		{
			mRadians *= rhs;
			return *this;
		}

		TRadian operator/(T rhs) const { return TRadian(mRadians / rhs); }

		TRadian& operator/=(T rhs)
		{
			mRadians /= rhs;
			return *this;
		}

		friend TRadian operator*(T lhs, const TRadian& rhs) { return TRadian(lhs * rhs.mRadians); }
		friend TRadian operator/(T lhs, const TRadian& rhs) { return TRadian(lhs / rhs.mRadians); }
		friend TRadian operator+(TRadian& lhs, T rhs) { return TRadian(lhs.mRadians + rhs); }
		friend TRadian operator+(T lhs, const TRadian& rhs) { return TRadian(lhs + rhs.mRadians); }
		friend TRadian operator-(const TRadian& lhs, T rhs) { return TRadian(lhs.mRadians - rhs); }
		friend TRadian operator-(const T lhs, const TRadian& rhs) { return TRadian(lhs - rhs.mRadians); }

		bool operator<(const TRadian& rhs) const { return mRadians < rhs.mRadians; }
		bool operator<=(const TRadian& rhs) const { return mRadians <= rhs.mRadians; }
		bool operator==(const TRadian& rhs) const { return mRadians == rhs.mRadians; }
		bool operator!=(const TRadian& rhs) const { return mRadians != rhs.mRadians; }
		bool operator>=(const TRadian& rhs) const { return mRadians >= rhs.mRadians; }
		bool operator>(const TRadian& rhs) const { return mRadians > rhs.mRadians; }

	private:
		T mRadians = (T)0.0;
	};

	extern template class TRadian<float>;
	extern template class TRadian<double>;

	/** @} */
} // namespace b3d
