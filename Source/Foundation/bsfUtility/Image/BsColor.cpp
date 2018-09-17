//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/BsColor.h"
#include "Math/BsMath.h"

namespace bs 
{
	const Color Color::ZERO = Color(0.0f, 0.0f, 0.0f, 0.0f);
	const Color Color::Black = Color(0.0f, 0.0f, 0.0f);
	const Color Color::White = Color(1.0f, 1.0f, 1.0f);
	const Color Color::Red = Color(1.0f, 0.0f, 0.0f);
	const Color Color::Green = Color(0.0f, 1.0f, 0.0f);
	const Color Color::Blue = Color(0.0f, 0.0f, 1.0f);
	const Color Color::LightGray = Color(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f);
	const Color Color::BansheeOrange = Color(1.0f, (168.0f/255.0f), 0.0f);

	Color Color::fromRGBA(RGBA val)
	{
		Color output;
		const UINT32 val32 = val;

		output.a = ((val32 >> 24) & 0xFF) / 255.0f;
		output.b = ((val32 >> 16) & 0xFF) / 255.0f;
		output.g = ((val32 >> 8) & 0xFF) / 255.0f;
		output.r = (val32 & 0xFF) / 255.0f;

		return output;
	}

	Color Color::fromABGR(UINT32 val)
	{
		Color output;
		const UINT32 val32 = val;

		output.r = ((val32 >> 24) & 0xFF) / 255.0f;
		output.g = ((val32 >> 16) & 0xFF) / 255.0f;
		output.b = ((val32 >> 8) & 0xFF) / 255.0f;
		output.a = (val32 & 0xFF) / 255.0f;

		return output;
	}

	Color Color::fromARGB(ARGB val)
	{
		Color output;
		const UINT32 val32 = val;

		output.b = ((val32 >> 24) & 0xFF) / 255.0f;
		output.g = ((val32 >> 16) & 0xFF) / 255.0f;
		output.r = ((val32 >> 8) & 0xFF) / 255.0f;
		output.a = (val32 & 0xFF) / 255.0f;

		return output;
	}

	Color Color::fromBGRA(BGRA val)
	{
		Color output;
		const UINT32 val32 = val;

		output.a = ((val32 >> 24) & 0xFF) / 255.0f;
		output.r = ((val32 >> 16) & 0xFF) / 255.0f;
		output.g = ((val32 >> 8) & 0xFF) / 255.0f;
		output.b = (val32 & 0xFF) / 255.0f;

		return output;
	}

	Color Color::fromHSB(float hue, float saturation, float brightness)
	{
		Color output;

		// wrap hue
		if (hue > 1.0f)
			hue -= (int)hue;
		else if (hue < 0.0f)
			hue += (int)hue + 1;

		// clamp saturation / brightness
		saturation = std::min(saturation, (float)1.0);
		saturation = std::max(saturation, (float)0.0);
		brightness = std::min(brightness, (float)1.0);
		brightness = std::max(brightness, (float)0.0);

		if (brightness == 0.0f)
		{   
			// early exit, this has to be black
			output.r = output.g = output.b = 0.0f;
			return output;
		}

		if (saturation == 0.0f)
		{   
			// early exit, this has to be grey

			output.r = output.g = output.b = brightness;
			return output;
		}

		float hueDomain  = hue * 6.0f;
		if (hueDomain >= 6.0f)
		{
			// wrap around, and allow mathematical errors
			hueDomain = 0.0f;
		}

		const auto domain = (unsigned short)hueDomain;
		const float f1 = brightness * (1 - saturation);
		const float f2 = brightness * (1 - saturation * (hueDomain - domain));
		const float f3 = brightness * (1 - saturation * (1 - (hueDomain - domain)));

		switch (domain)
		{
		case 0:
			// red domain; green ascends
			output.r = brightness;
			output.g = f3;
			output.b = f1;
			break;
		case 1:
			// yellow domain; red descends
			output.r = f2;
			output.g = brightness;
			output.b = f1;
			break;
		case 2:
			// green domain; blue ascends
			output.r = f1;
			output.g = brightness;
			output.b = f3;
			break;
		case 3:
			// cyan domain; green descends
			output.r = f1;
			output.g = f2;
			output.b = brightness;
			break;
		case 4:
			// blue domain; red ascends
			output.r = f3;
			output.g = f1;
			output.b = brightness;
			break;
		case 5:
			// magenta domain; blue descends
			output.r = brightness;
			output.g = f1;
			output.b = f2;
			break;
		}

		return output;
	}

	ABGR Color::getAsABGR() const
	{
		UINT8 val8;
		UINT32 val32 = 0;

		// Convert to 32bit pattern
		// (RGBA = 8888)

		// Red
		val8 = static_cast<UINT8>(r * 255);
		val32 = val8 << 24;

		// Green
		val8 = static_cast<UINT8>(g * 255);
		val32 += val8 << 16;

		// Blue
		val8 = static_cast<UINT8>(b * 255);
		val32 += val8 << 8;

		// Alpha
		val8 = static_cast<UINT8>(a * 255);
		val32 += val8;

		return val32;
	}

	BGRA Color::getAsBGRA() const
	{
		UINT8 val8;
		UINT32 val32 = 0;

		// Convert to 32bit pattern
		// (ARGB = 8888)

		// Alpha
		val8 = static_cast<UINT8>(a * 255);
		val32 = val8 << 24;

		// Red
		val8 = static_cast<UINT8>(r * 255);
		val32 += val8 << 16;

		// Green
		val8 = static_cast<UINT8>(g * 255);
		val32 += val8 << 8;

		// Blue
		val8 = static_cast<UINT8>(b * 255);
		val32 += val8;

		return val32;
	}

	ARGB Color::getAsARGB() const
	{
		UINT8 val8;
		UINT32 val32 = 0;

		// Convert to 32bit pattern
		// (ARGB = 8888)

		// Blue
		val8 = static_cast<UINT8>(b * 255);
		val32 = val8 << 24;

		// Green
		val8 = static_cast<UINT8>(g * 255);
		val32 += val8 << 16;

		// Red
		val8 = static_cast<UINT8>(r * 255);
		val32 += val8 << 8;

		// Alpha
		val8 = static_cast<UINT8>(a * 255);
		val32 += val8;


		return val32;
	}

	RGBA Color::getAsRGBA() const
	{
		UINT8 val8;
		UINT32 val32 = 0;

		// Convert to 32bit pattern
		// (ABRG = 8888)

		// Alpha
		val8 = static_cast<UINT8>(a * 255);
		val32 = val8 << 24;

		// Blue
		val8 = static_cast<UINT8>(b * 255);
		val32 += val8 << 16;

		// Green
		val8 = static_cast<UINT8>(g * 255);
		val32 += val8 << 8;

		// Red
		val8 = static_cast<UINT8>(r * 255);
		val32 += val8;


		return val32;
	}

	bool Color::operator==(const Color& rhs) const
	{
		return (r == rhs.r &&
			g == rhs.g &&
			b == rhs.b &&
			a == rhs.a);
	}

	bool Color::operator!=(const Color& rhs) const
	{
		return !(*this == rhs);
	}

	void Color::getHSB(float* hue, float* saturation, float* brightness) const
	{
		float vMin = std::min(r, std::min(g, b));
		float vMax = std::max(r, std::max(g, b));
		float delta = vMax - vMin;

		*brightness = vMax;

		if (Math::approxEquals(delta, 0.0f, 1e-6f))
		{
			// grey
			*hue = 0;
			*saturation = 0;
		}
		else                                    
		{
			// a colour
			*saturation = delta / vMax;

			float deltaR = (((vMax - r) / 6.0f) + (delta / 2.0f)) / delta;
			float deltaG = (((vMax - g) / 6.0f) + (delta / 2.0f)) / delta;
			float deltaB = (((vMax - b) / 6.0f) + (delta / 2.0f)) / delta;

			if (Math::approxEquals(r, vMax))
				*hue = deltaB - deltaG;
			else if (Math::approxEquals(g, vMax))
				*hue = 0.3333333f + deltaR - deltaB;
			else if (Math::approxEquals(b, vMax)) 
				*hue = 0.6666667f + deltaG - deltaR;

			if (*hue < 0.0f) 
				*hue += 1.0f;
			if (*hue > 1.0f)
				*hue -= 1.0f;
		}
	}

	Color Color::lerp(float t, const Color& a, const Color& b)
	{
		t = Math::clamp01(t);
		return Color(a.r + (b.r - a.r) * t,
					 a.g + (b.g - a.g) * t,
					 a.b + (b.b - a.b) * t,
					 a.a + (b.a - a.a) * t);
	}
}

