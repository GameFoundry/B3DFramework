//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup Image
	 *  @{
	 */

	typedef u32 RGBA;
	typedef u32 ARGB;
	typedef u32 ABGR;
	typedef u32 BGRA;

	/**
	 * Color represented as 4 components, each being a floating point value ranging from 0 to 1. Color components are
	 * red, green, blue and alpha.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true)) Color
	{
	public:
		constexpr Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
			: R(red), G(green), B(blue), A(alpha)
		{}

		static const Color kZero;
		static const Color kBlack;
		static const Color kWhite;
		static const Color kRed;
		static const Color kGreen;
		static const Color kBlue;
		static const Color kLightGray;
		static const Color kBansheeOrange;
		static const Color kPink;

		bool operator==(const Color& rhs) const;
		bool operator!=(const Color& rhs) const;

		/** Returns the color as a 32-bit value in RGBA order. */
		RGBA GetAsRgba() const;

		/** Returns the color as a 32-bit value in ARGB order. */
		ARGB GetAsArgb() const;

		/** Returns the color as a 32-bit value in BGRA order. */
		BGRA GetAsBgra() const;

		/** Returns the color as a 32-bit value in ABGR order. */
		ABGR GetAsAbgr() const;

		/**
		 * Convert the current color to hue, saturation and brightness values.
		 *
		 * @param	outHue			Output hue value, scaled to the [0,1] range.
		 * @param	outSaturation	Output saturation level, [0,1].
		 * @param	outBrightness	Output brightness level, [0,1].
		 */
		void GetHsb(float* outHue, float* outSaturation, float* outBrightness) const;

		/** Converts the current color from gamma to linear space and returns the result. */
		Color GetLinear() const;

		/** Converts the current color from linear to gamma space and returns the result. */
		Color GetGamma() const;

		/** Clamps color value to the range [0, 1]. */
		void Saturate()
		{
			if(R < 0)
				R = 0;
			else if(R > 1)
				R = 1;

			if(G < 0)
				G = 0;
			else if(G > 1)
				G = 1;

			if(B < 0)
				B = 0;
			else if(B > 1)
				B = 1;

			if(A < 0)
				A = 0;
			else if(A > 1)
				A = 1;
		}

		/** Clamps colour value to the range [0, 1]. Returned saturated color as a copy. */
		Color SaturateCopy() const
		{
			Color ret = *this;
			ret.Saturate();
			return ret;
		}

		float operator[](const u32 i) const
		{
			B3D_ASSERT(i < 4);

			return *(&R + i);
		}

		float& operator[](const u32 i)
		{
			B3D_ASSERT(i < 4);

			return *(&R + i);
		}

		/** Pointer accessor for direct copying. */
		float* Ptr()
		{
			return &R;
		}

		/** Pointer accessor for direct copying. */
		const float* Ptr() const
		{
			return &R;
		}

		Color operator+(const Color& rhs) const
		{
			return Color(R + rhs.R, G + rhs.G, B + rhs.B, A + rhs.A);
		}

		Color operator-(const Color& rhs) const
		{
			return Color(R - rhs.R, G - rhs.G, B - rhs.B, A - rhs.A);
		}

		Color operator*(float rhs) const
		{
			return Color(rhs * R, rhs * G, rhs * B, rhs * A);
		}

		Color operator*(const Color& rhs) const
		{
			return Color(rhs.R * R, rhs.G * G, rhs.B * B, rhs.A * A);
		}

		Color operator/(const Color& rhs) const
		{
			return Color(R / rhs.R, G / rhs.G, B / rhs.B, A / rhs.A);
		}

		Color operator/(float rhs) const
		{
			B3D_ASSERT(rhs != 0.0f);
			float invRhs = 1.0f / rhs;

			return Color(R * invRhs, G * invRhs, B * invRhs, A * invRhs);
		}

		friend Color operator*(float lhs, const Color& rhs)
		{
			return Color(lhs * rhs.R, lhs * rhs.G, lhs * rhs.B, lhs * rhs.A);
		}

		Color& operator+=(const Color& rhs)
		{
			R += rhs.R;
			G += rhs.G;
			B += rhs.B;
			A += rhs.A;

			return *this;
		}

		Color& operator-=(const Color& rhs)
		{
			R -= rhs.R;
			G -= rhs.G;
			B -= rhs.B;
			A -= rhs.A;

			return *this;
		}

		Color& operator*=(const Color& rhs)
		{
			R *= rhs.R;
			G *= rhs.G;
			B *= rhs.B;
			A *= rhs.A;

			return *this;
		}

		Color& operator*=(float rhs)
		{
			R *= rhs;
			G *= rhs;
			B *= rhs;
			A *= rhs;

			return *this;
		}

		Color& operator/=(float rhs)
		{
			B3D_ASSERT(rhs != 0.0f);

			float invRhs = 1.0f / rhs;

			R *= invRhs;
			G *= invRhs;
			B *= invRhs;
			A *= invRhs;

			return *this;
		}

		/** Creates a color value from a 32-bit value that encodes a RGBA color. */
		static Color FromRgba(RGBA val);

		/** Creates a color value from a 32-bit value that encodes a ARGB color. */
		static Color FromArgb(ARGB val);

		/** Creates a color value from a 32-bit value that encodes a BGRA color. */
		static Color FromBgra(BGRA val);

		/** Creates a color value from a 32-bit value that encodes a ABGR color. */
		static Color FromAbgr(ABGR val);

		/**
		 * Creates a color value from hue, saturation and brightness.
		 *
		 * @param hue			Hue value, scaled to the [0,1] range.
		 * @param saturation	Saturation level, [0,1].
		 * @param brightness	Brightness level, [0,1].
		 * @param alpha			Transparency level, [0,1].
		 */
		static Color FromHSB(float hue, float saturation, float brightness, float alpha = 1.0f);

		/**
		 * Creates an RGBA color from a HSL(A) color.
		 *
		 * @param hue			Hue value, scaled to the [0,1] range.
		 * @param saturation	Saturation level, [0,1].
		 * @param lightness		Lightness level, [0,1].
		 * @param alpha			Transparency level, [0,1].
		 */
		static Color FromHSL(float hue, float saturation, float lightness, float alpha = 1.0f);

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
		static constexpr RGBA Lerp(u8 t, RGBA from, RGBA to)
		{
			constexpr u32 RB_MASK = 0x00FF00FF;
			constexpr u32 GA_MASK = 0xFF00FF00;

			// Lerp two channels at a time (this leaves 8 extra bits for each channel for results)
			//// Red-blue first
			const u32 rbFrom = from & RB_MASK;
			const u32 rbTo = to & RB_MASK;
			const u32 rb = (rbFrom + (((rbTo - rbFrom) * t) >> 8)) & RB_MASK;

			//// Then green-alpha
			const u32 gaFrom = from & GA_MASK;
			const u32 gaTo = to & GA_MASK;
			const u32 ga = (((gaFrom >> 8) + ((((gaTo >> 8) - (gaFrom >> 8)) * t) >> 8)) << 8) & GA_MASK;

			return rb | ga;
		}

		float R, G, B, A;
	};

	/** @} */
} // namespace b3d

/** @cond SPECIALIZATIONS */

namespace std
{
	/** Hash value generator for Color. */
	template <>
	struct hash<b3d::Color>
	{
		size_t operator()(const b3d::Color& color) const
		{
			size_t hash = 0;
			b3d::B3DCombineHash(hash, color.R);
			b3d::B3DCombineHash(hash, color.G);
			b3d::B3DCombineHash(hash, color.B);
			b3d::B3DCombineHash(hash, color.A);

			return hash;
		}
	};
} // namespace std

/** @endcond */
