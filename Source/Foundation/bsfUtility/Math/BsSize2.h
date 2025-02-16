//************************************ bs::framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Math/BsMath.h"

namespace bs
{
	/** @addtogroup Math
	 *  @{
	 */

	/** Encapsulates width/height in a single structure. */
	template<class T>
	struct TSize2
	{
		T Width, Height;
		
		constexpr TSize2() = default;

		B3D_SCRIPT_EXPORT(Exclude(true))
		constexpr TSize2(BS_ZERO)
			: Width((T)0), Height((T)0)
		{}

		constexpr TSize2(T width, T height)
			: Width(width), Height(height)
		{}

		TSize2 operator+(const TSize2& rhs) const
		{
			return TSize2(Width + rhs.Width, Height + rhs.Height);
		}

		TSize2 operator-(const TSize2& rhs) const
		{
			return TSize2(Width - rhs.Width, Height - rhs.Height);
		}

		TSize2 operator*(const T rhs) const
		{
			return TSize2(Width * rhs, Height * rhs);
		}

		TSize2 operator*(const TSize2& rhs) const
		{
			return TSize2(Width * rhs.Width, Height * rhs.Height);
		}

		TSize2 operator/(const T rhs) const
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverseRHS = (T)1.0 / rhs;
			return TSize2(Width * inverseRHS, Height * inverseRHS);
		}

		TSize2 operator/(const TSize2& rhs) const
		{
			return TSize2(Width / rhs.Width, Height / rhs.Height);
		}

		const TSize2& operator+() const
		{
			return *this;
		}

		template<typename U = T, typename = std::enable_if_t<std::is_signed_v<U>, i32>>
		TSize2 operator-() const
		{
			return TSize2(-Width, -Height);
		}

		friend TSize2 operator*(T lhs, const TSize2& rhs)
		{
			return TSize2(lhs * rhs.Width, lhs * rhs.Height);
		}

		friend TSize2 operator/(T lhs, const TSize2& rhs)
		{
			return TSize2(lhs / rhs.Width, lhs / rhs.Height);
		}

		friend TSize2 operator+(TSize2& lhs, T rhs)
		{
			return TSize2(lhs.Width + rhs, lhs.Height + rhs);
		}

		friend TSize2 operator+(T lhs, const TSize2& rhs)
		{
			return TSize2(lhs + rhs.Width, lhs + rhs.Height);
		}

		friend TSize2 operator-(const TSize2& lhs, T rhs)
		{
			return TSize2(lhs.Width - rhs, lhs.Height - rhs);
		}

		friend TSize2 operator-(const T lhs, const TSize2& rhs)
		{
			return TSize2(lhs - rhs.Width, lhs - rhs.Height);
		}

		TSize2& operator+=(const TSize2& rhs)
		{
			Width += rhs.Width;
			Height += rhs.Height;

			return *this;
		}

		TSize2& operator+=(T rhs)
		{
			Width += rhs;
			Height += rhs;

			return *this;
		}

		TSize2& operator-=(const TSize2& rhs)
		{
			Width -= rhs.Width;
			Height -= rhs.Height;

			return *this;
		}

		TSize2& operator-=(T rhs)
		{
			Width -= rhs;
			Height -= rhs;

			return *this;
		}

		TSize2& operator*=(T rhs)
		{
			Width *= rhs;
			Height *= rhs;

			return *this;
		}

		TSize2& operator*=(const TSize2& rhs)
		{
			Width *= rhs.Width;
			Height *= rhs.Height;

			return *this;
		}

		TSize2& operator/=(T rhs)
		{
			B3D_ASSERT(rhs != (T)0.0);

			const T inverseRHS = (T)1.0 / rhs;
			Width *= inverseRHS;
			Height *= inverseRHS;

			return *this;
		}

		TSize2& operator/=(const TSize2& rhs)
		{
			Width /= rhs.Width;
			Height /= rhs.Height;

			return *this;
		}

		bool operator==(const TSize2& rhs) const
		{
			return (Width == rhs.Width && Height == rhs.Height);
		}

		bool operator!=(const TSize2& rhs) const
		{
			return !operator==(rhs);
		}

		TSize2<float> ToFloat() const
		{
			return TSize2<float>((float)Width, (float)Height);
		}

		static TSize2<T> FromFloat(const TSize2<float>& other)
		{
			return TSize2<T>((T)other.Width, (T)other.Height);
		}

		static const TSize2 kZero;
	};

	template<> const TSize2<u32> TSize2<u32>::kZero{BsZero};
	template<> const TSize2<float> TSize2<float>::kZero{BsZero};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TSize2<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TSize2<u32>;

	using Size2UI = TSize2<u32>;
	using Size2 = TSize2<float>;

	/** @} */
} // namespace bs

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TSize2<T>. */
template<class T>
struct hash<bs::TSize2<T>>
{
	size_t operator()(const bs::TSize2<T>& value) const
	{
		size_t hash = 0;
		bs::B3DCombineHash(hash, value.Width);
		bs::B3DCombineHash(hash, value.Height);

		return hash;
	}
};
} // namespace std

/** @endcond */
