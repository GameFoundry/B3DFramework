//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	    explicit Color(float red = 1.0f, float green = 1.0f,
				    float blue = 1.0f, float alpha = 1.0f ) 
					:r(red), g(green), b(blue), a(alpha)
        { }

	    bool operator==(const Color& rhs) const;
	    bool operator!=(const Color& rhs) const;

	    RGBA getAsRGBA(void) const;
	    ARGB getAsARGB(void) const;
		BGRA getAsBGRA(void) const;
	    ABGR getAsABGR(void) const;

        void setAsRGBA(const RGBA val);
        void setAsARGB(const ARGB val);
		void setAsBGRA(const BGRA val);
        void setAsABGR(const ABGR val);

		/** Clamps colour value to the range [0, 1]. */
        void saturate()
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
        Color saturateCopy() const
        {
            Color ret = *this;
            ret.saturate();
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
            Color kSum;

            kSum.r = r + rhs.r;
            kSum.g = g + rhs.g;
            kSum.b = b + rhs.b;
            kSum.a = a + rhs.a;

            return kSum;
        }

        Color operator- ( const Color& rhs) const
        {
            Color kDiff;

            kDiff.r = r - rhs.r;
            kDiff.g = g - rhs.g;
            kDiff.b = b - rhs.b;
            kDiff.a = a - rhs.a;

            return kDiff;
        }

        Color operator* (const float rhs) const
        {
            Color kProd;

            kProd.r = rhs*r;
            kProd.g = rhs*g;
            kProd.b = rhs*b;
            kProd.a = rhs*a;

            return kProd;
        }

        Color operator* (const Color& rhs) const
        {
            Color kProd;

            kProd.r = rhs.r * r;
            kProd.g = rhs.g * g;
            kProd.b = rhs.b * b;
            kProd.a = rhs.a * a;

            return kProd;
        }

        Color operator/ (const Color& rhs) const
        {
            Color kProd;

            kProd.r = rhs.r / r;
            kProd.g = rhs.g / g;
            kProd.b = rhs.b / b;
            kProd.a = rhs.a / a;

            return kProd;
        }

        Color operator/ (const float rhs) const
        {
            assert(rhs != 0.0f);

            Color kDiv;

            float fInv = 1.0f / rhs;
            kDiv.r = r * fInv;
            kDiv.g = g * fInv;
            kDiv.b = b * fInv;
            kDiv.a = a * fInv;

            return kDiv;
        }

        friend Color operator* (const float lhs, const Color& rhs)
        {
            Color result;

            result.r = lhs * rhs.r;
            result.g = lhs * rhs.g;
            result.b = lhs * rhs.b;
            result.a = lhs * rhs.a;

            return result;
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

        Color& operator*= (const float rhs)
        {
            r *= rhs;
            g *= rhs;
            b *= rhs;
            a *= rhs;

            return *this;
        }

        Color& operator/= (const float rhs)
        {
            assert(rhs != 0.0f);

            float fInv = 1.0f / rhs;

            r *= fInv;
            g *= fInv;
            b *= fInv;
            a *= fInv;

            return *this;
        }

		/** 
		 * Set a colour value from Hue, Saturation and Brightness.
		 *
		 * @param[in] hue			Hue value, scaled to the [0,1] range.
		 * @param[in] saturation	Saturation level, [0,1].
		 * @param[in] brightness	Brightness level, [0,1].
		 */
		void setHSB(float hue, float saturation, float brightness);

		/** 
		 * Convert the current color to Hue, Saturation and Brightness values. 
		 * 
		 * @param[in] hue			Output hue value, scaled to the [0,1] range.
		 * @param[in] saturation	Output saturation level, [0,1].
		 * @param[in] brightness	Output brightness level, [0,1].
		 */
		void getHSB(float* hue, float* saturation, float* brightness) const;

        /**
        * Linearly interpolates between the two colors using @p t. t should be in [0, 1] range, where t = 0 corresponds
        * to the left color, while t = 1 corresponds to the right color.
        */
        static Color lerp(float t, const Color& a, const Color& b);

		float r, g, b, a;
    };

	/** @cond SPECIALIZATIONS */
	BS_ALLOW_MEMCPY_SERIALIZATION(Color);
	/** @endcond */

	/** @} */
}

/** @cond SPECIALIZATIONS */

namespace std
{
/** Hash value generator for Color. */
template<> 
struct hash<bs::Color>
{
	size_t operator()(const bs::Color& color) const
	{
		size_t hash = 0;
		bs::hash_combine(hash, color.r);
		bs::hash_combine(hash, color.g);
		bs::hash_combine(hash, color.b);
		bs::hash_combine(hash, color.a);

		return hash;
	}
};
}

/** @endcond */
