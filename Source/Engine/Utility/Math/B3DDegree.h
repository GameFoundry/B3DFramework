//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	template<class T>
	class TRadian;

	/** @addtogroup Math
	 *  @{
	 */

	/**
	 * Wrapper class which indicates a given angle value is in degrees.
	 *
	 * @note
	 * Degree values are interchangeable with Radian values, and conversions will be done automatically between them.
	 */
	template<class T>
	class TDegree
	{
	public:
		constexpr TDegree() = default;
		constexpr TDegree(const TDegree& value) = default;
		constexpr explicit TDegree(T degrees)
			: mDegrees(degrees) {}

		TDegree(const TRadian<T>& value);

		TDegree& operator=(const TRadian<T>& rhs);
		constexpr TDegree& operator=(const TDegree& rhs) = default;
		constexpr TDegree& operator=(const T& rhs)
		{
			mDegrees = rhs;
			return *this;
		}


		/** Returns the value of the angle in degrees. */
		constexpr T GetValueInDegrees() const { return mDegrees; }

		/** Returns the value of the angle in radians. */
		T GetValueInRadians() const;

		/** Wraps the angle in [0, 360) range */
		TDegree Wrap();

		const TDegree& operator+() const { return *this; }

		TDegree operator+(const TRadian<T>& rhs) const;
		TDegree operator+(const TDegree& rhs) const { return TDegree(mDegrees + rhs.mDegrees); }

		TDegree& operator+=(const TRadian<T>& rhs);
		TDegree& operator+=(const TDegree& rhs)
		{
			mDegrees += rhs.mDegrees;
			return *this;
		}

		TDegree operator-() const { return TDegree(-mDegrees); }

		TDegree operator-(const TRadian<T>& rhs) const;
		TDegree operator-(const TDegree& rhs) const { return TDegree(mDegrees - rhs.mDegrees); }

		TDegree& operator-=(const TRadian<T>& rhs);
		TDegree& operator-=(const TDegree& rhs)
		{
			mDegrees -= rhs.mDegrees;
			return *this;
		}

		TDegree operator*(T rhs) const { return TDegree(mDegrees * rhs); }
		TDegree operator*(const TDegree& rhs) const { return TDegree(mDegrees * rhs.mDegrees); }

		TDegree& operator*=(T rhs)
		{
			mDegrees *= rhs;
			return *this;
		}

		TDegree operator/(T rhs) const { return TDegree(mDegrees / rhs); }
		TDegree operator/(const TDegree& rhs) const { return TDegree(mDegrees / rhs.mDegrees); }

		TDegree& operator/=(T rhs)
		{
			mDegrees /= rhs;
			return *this;
		}

		friend TDegree operator*(T lhs, const TDegree& rhs) { return TDegree(lhs * rhs.mDegrees); }
		friend TDegree operator/(T lhs, const TDegree& rhs) { return TDegree(lhs / rhs.mDegrees); }
		friend TDegree operator+(TDegree& lhs, T rhs) { return TDegree(lhs.mDegrees + rhs); }
		friend TDegree operator+(T lhs, const TDegree& rhs) { return TDegree(lhs + rhs.mDegrees); }
		friend TDegree operator-(const TDegree& lhs, T rhs) { return TDegree(lhs.mDegrees - rhs); }
		friend TDegree operator-(const T lhs, const TDegree& rhs) { return TDegree(lhs - rhs.mDegrees); }

		bool operator<(const TDegree& rhs) const { return mDegrees < rhs.mDegrees; }
		bool operator<=(const TDegree& rhs) const { return mDegrees <= rhs.mDegrees; }
		bool operator==(const TDegree& rhs) const { return mDegrees == rhs.mDegrees; }
		bool operator!=(const TDegree& rhs) const { return mDegrees != rhs.mDegrees; }
		bool operator>=(const TDegree& rhs) const { return mDegrees >= rhs.mDegrees; }
		bool operator>(const TDegree& rhs) const { return mDegrees > rhs.mDegrees; }

	private:
		T mDegrees = (T)0.0;
	};

	extern template class TDegree<float>;
	extern template class TDegree<double>;

	/** @} */
} // namespace b3d
