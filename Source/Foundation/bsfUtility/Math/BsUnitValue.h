//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Contains a number value and an associated unit. Used primarily to prevent implicit conversion between numbers of different units. */
	template<typename T, typename Unit>
	struct TUnitValue
	{
		T Value = (T)0;

		TUnitValue() = default;
		TUnitValue(T value)
			:Value(value)
		{ }

		explicit operator T() const { return Value; }

		TUnitValue& operator=(T value) { Value = value; return *this; }

		bool operator==(const TUnitValue& rhs) const { return Value == rhs.Value; }
		bool operator!=(const TUnitValue& rhs) const { return Value != rhs.Value; }

		bool operator<(const TUnitValue& rhs) const { return Value < rhs.Value; }
		bool operator>(const TUnitValue& rhs) const { return Value > rhs.Value; }
		bool operator<=(const TUnitValue& rhs) const { return Value <= rhs.Value; }
		bool operator>=(const TUnitValue& rhs) const { return Value >= rhs.Value; }

		TUnitValue operator+(const TUnitValue& rhs) const { return TUnitValue(Value + rhs.Value); }
		TUnitValue operator-(const TUnitValue& rhs) const { return TUnitValue(Value - rhs.Value); }

		TUnitValue operator*(const TUnitValue& rhs) const { return TUnitValue(Value * rhs.Value); }
		TUnitValue operator/(const TUnitValue& rhs) const { return TUnitValue(Value / rhs.Value); }

		const TUnitValue& operator+() const { return *this; }

		template<typename U = T, typename = std::enable_if_t<std::is_signed_v<U>, i32>>
		TUnitValue operator-() const { return TUnitValue(-Value); }

		TUnitValue& operator+=(const TUnitValue& rhs) { Value += rhs.Value; return *this; }
		TUnitValue& operator-=(const TUnitValue& rhs) { Value -= rhs.Value; return *this; }

		TUnitValue& operator*=(const TUnitValue& rhs) { Value *= rhs.Value; return *this; }
		TUnitValue& operator/=(const TUnitValue& rhs) { Value /= rhs.Value; return *this; }
	};

	/** @} */
} // namespace bs

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TUnitValue<T, Unit>. */
template<typename T, typename Unit>
struct hash<bs::TUnitValue<T, Unit>>
{
	size_t operator()(const bs::TUnitValue<T, Unit>& value) const
	{
		size_t hash = 0;
		bs::B3DCombineHash(hash, value.Value);

		return hash;
	}
};
} // namespace std

/** @endcond */
