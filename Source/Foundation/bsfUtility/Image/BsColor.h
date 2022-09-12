//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"

namespace bs
{
	/** @addtogroup Image
	 *  @{
	 */

	typedef UINT32 RGBA;
	typedef UINT32 ARGB;
	typedef UINT32 ABGR;
	typedef UINT32 BGRA;

	/**
	 * Color represented as 4 components, each being a floating point value ranging from 0 to 1. Color components are
	 * red, green, blue and alpha.
	 */
	class BS_UTILITY_EXPORT Color
	{
	public:
		static const Color ZERO;
		static const Color Black;
		static const Color White;
		static const Color Red;
		static const Color Green;
		static const Color Blue;
		static const Color LightGray;
		static const Color BansheeOrange;

		constexpr explicit Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f )
			:r(red), g(green), b(blue), a(alpha)
		{ }

		bool operator==(const Color& rhs) const;
		bool operator!=(const Color& rhs) const;

		/** Returns the color as a 32-bit value in RGBA order. */
		RGBA GetAsRGBA() const;

		/** Returns the color as a 32-bit value in ARGB order. */
		ARGB GetAsARGB() const;

		/** Returns the color as a 32-bit value in BGRA order. */
		BGRA GetAsBGRA() const;

		/** Returns the color as a 32-bit value in ABGR order. */
		ABGR GetAsABGR() const;

		/**
		 * Convert the current color to hue, saturation and brightness values.
		 *
		 * @param[in] hue			Output hue value, scaled to the [0,1] range.
		 * @param[in] saturation	Output saturation level, [0,1].
		 * @param[in] brightness	Output brightness level, [0,1].
		 */
		void GetHSB(float* hue, float* saturation, float* brightness) const;

		/** Converts the current color from gamma to linear space and returns the result. */
		Color GetLinear() const;

		/** Converts the current color from linear to gamma space and returns the result. */
		Color GetGamma() const;

		/** Clamps color value to the range [0, 1]. */
		void Saturate()
		{
			if (r < 0)
				r = 0;
			else if (r > 1)
				r = 1;

			if (g < 0)
				g = 0;
			else if (g > 1)
				g = 1;

			if (b < 0)
				b = 0;
			else if (b > 1)
				b = 1;

			if (a < 0)
				a = 0;
			else if (a > 1)
				a = 1;
		}

		/** Clamps colour value to the range [0, 1]. Returned saturated color as a copy. */
		Color SaturateCopy() const
		{
			Color ret = *this;
			ret.Saturate();
			return ret;
		}

		float operator[] (const UINT32 i) const
		{
			assert(i < 4);

			return *(&r+i);
		}

		float& operator[] (const UINT32 i)
		{
			assert(i < 4);

			return *(&r+i);
		}

		/** Pointer accessor for direct copying. */
		float* ptr()
		{
			return &r;
		}

		/** Pointer accessor for direct copying. */
		const float* ptr() const
		{
			return &r;
		}

		Color operator+ (const Color& rhs) const
		{
			return Color(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a);
		}

		Color operator- (const Color& rhs) const
		{
			return Color(r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a);
		}

		Color operator* (float rhs) const
		{
			return Color(rhs * r, rhs * g, rhs * b, rhs * a);
		}

		Color operator* (const Color& rhs) const
		{
			return Color(rhs.r * r, rhs.g * g, rhs.b * b, rhs.a * a);
		}

		Color operator/ (const Color& rhs) const
		{
			return Color(r / rhs.r, g / rhs.g, b / rhs.b, a / rhs.a);
		}

		Color operator/ (float rhs) const
		{
			assert(rhs != 0.0f);
			float invRhs = 1.0f / rhs;

			return Color(r * invRhs, g * invRhs, b * invRhs, a * invRhs);
		}

		friend Color operator* (float lhs, const Color& rhs)
		{
			return Color(lhs * rhs.r, lhs * rhs.g, lhs * rhs.b, lhs * rhs.a);
		}

		Color& operator+= (const Color& rhs)
		{
			r += rhs.r;
			g += rhs.g;
			b += rhs.b;
			a += rhs.a;

			return *this;
		}

		Color& operator-= (const Color& rhs)
		{
			r -= rhs.r;
			g -= rhs.g;
			b -= rhs.b;
			a -= rhs.a;

			return *this;
		}

		Color& operator*= (float rhs)
		{
			r *= rhs;
			g *= rhs;
			b *= rhs;
			a *= rhs;

			return *this;
		}

		Color& operator/= (float rhs)
		{
			assert(rhs != 0.0f);

			float invRhs = 1.0f / rhs;

			r *= invRhs;
			g *= invRhs;
			b *= invRhs;
			a *= invRhs;

			return *this;
		}

		/** Creates a color value from a 32-bit value that encodes a RGBA color. */
		static Color FromRGBA(RGBA val);

		/** Creates a color value from a 32-bit value that encodes a ARGB color. */
		static Color FromARGB(ARGB val);

		/** Creates a color value from a 32-bit value that encodes a BGRA color. */
		static Color FromBGRA(BGRA val);

		/** Creates a color value from a 32-bit value that encodes a ABGR color. */
		static Color FromABGR(ABGR val);

		/**
		 * Creates a color value from hue, saturation and brightness.
		 *
		 * @param[in] hue			Hue value, scaled to the [0,1] range.
		 * @param[in] saturation	Saturation level, [0,1].
		 * @param[in] brightness	Brightness level, [0,1].
		 */
		static Color FromHSB(float hue, float saturation, float brightness);

		/**
		 * Linearly interpolates between the two colors using @p t. t should be in [0, 1] range, where t = 0 corresponds
		 * to the left color, while t = 1 corresponds to the right color.
		 */
		static Color Lerp(float t, const Color& a, const Color& b);

		/**
		 * Linearly interpolates between the two colors using @p t. t should be in [0, 255] range, where t = 0 corresponds
		 * to the left color, while t = 1 corresponds to the right color. Operates directly on 8-bit per channel
		 * encoded color instead of on floating point values.
		 */
		static constexpr RGBA Lerp(UINT8 t, RGBA from, RGBA to)
		{
			constexpr UINT32 RB_MASK = 0x00FF00FF;
			constexpr UINT32 GA_MASK = 0xFF00FF00;

			// Lerp two channels at a time (this leaves 8 extra bits for each channel for results)
			//// Red-blue first
			const UINT32 rbFrom = from & RB_MASK;
			const UINT32 rbTo = to & RB_MASK;
			const UINT32 rb = (rbFrom + (((rbTo - rbFrom) * t) >> 8)) & RB_MASK;

			//// Then green-alpha
			const UINT32 gaFrom = from & GA_MASK;
			const UINT32 gaTo = to & GA_MASK;
			const UINT32 ga = (((gaFrom >> 8) + ((((gaTo >> 8) - (gaFrom >> 8)) * t) >> 8)) << 8) & GA_MASK;

			return rb | ga;
		}

		float r, g, b, a;
	};

	/** @} */
}

/** @cond SPECIALIZATIONS */

namespace std
{
/** Hash value generator for Color. */
template<>
struct hash<bs::Color>
{
	size_t Operator()(const bs::Color& color) const
	{
		size_t hash = 0;
		bs::bs_hash_combine(hash, color.r);
		bs::bs_hash_combine(hash, color.g);
		bs::bs_hash_combine(hash, color.b);
		bs::bs_hash_combine(hash, color.a);

		return hash;
	}
};
}

/** @endcond */
