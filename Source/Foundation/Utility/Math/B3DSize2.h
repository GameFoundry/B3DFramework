//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Math/B3DMath.h"

namespace b3d
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
			B3D_ASSERT(rhs != (T)0);

			const T inverseRHS = (T)1 / rhs;
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
			B3D_ASSERT(rhs != (T)0);

			const T inverseRHS = (T)1 / rhs;
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

		/** Converts a size with one underlying type to another. */
		template<typename T2>
		TSize2<T2> To() const
		{
			return TSize2<T2>((T2)Width, (T2)Height);
		}

		static const TSize2 kZero;
	};

	template<> const TSize2<u32> TSize2<u32>::kZero{BsZero};
	template<> const TSize2<i32> TSize2<i32>::kZero{BsZero};
	template<> const TSize2<float> TSize2<float>::kZero{BsZero};
	template<> const TSize2<double> TSize2<double>::kZero{BsZero};

	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TSize2<float>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TSize2<double>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TSize2<u32>;
	extern template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Math), ExportAsStruct(true)) TSize2<i32>;

	using Size2UI = TSize2<u32>;
	using Size2I = TSize2<i32>;
	using Size2 = TSize2<float>;
	using Size2F = TSize2<float>;
	using Size2D = TSize2<double>;

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std
{
/** Hash value generator for TSize2<T>. */
template<class T>
struct hash<b3d::TSize2<T>>
{
	size_t operator()(const b3d::TSize2<T>& value) const
	{
		size_t hash = 0;
		b3d::B3DCombineHash(hash, value.Width);
		b3d::B3DCombineHash(hash, value.Height);

		return hash;
	}
};
} // namespace std

/** @endcond */
