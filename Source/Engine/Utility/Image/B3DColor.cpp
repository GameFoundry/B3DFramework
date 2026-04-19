//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DColor.h"
#include "Math/B3DMath.h"

using namespace b3d;

const Color Color::kZero = Color(0.0f, 0.0f, 0.0f, 0.0f);
const Color Color::kBlack = Color(0.0f, 0.0f, 0.0f);
const Color Color::kWhite = Color(1.0f, 1.0f, 1.0f);
const Color Color::kRed = Color(1.0f, 0.0f, 0.0f);
const Color Color::kGreen = Color(0.0f, 1.0f, 0.0f);
const Color Color::kBlue = Color(0.0f, 0.0f, 1.0f);
const Color Color::kLightGray = Color(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f);
const Color Color::kBansheeOrange = Color(1.0f, (168.0f / 255.0f), 0.0f);
const Color Color::kPink = Color(1.0f, (105.0f / 255.0f), (180.0f / 255.0f));

Color Color::FromRgba(RGBA val)
{
	Color output;
	const u32 val32 = val;

	output.A = ((val32 >> 24) & 0xFF) / 255.0f;
	output.B = ((val32 >> 16) & 0xFF) / 255.0f;
	output.G = ((val32 >> 8) & 0xFF) / 255.0f;
	output.R = (val32 & 0xFF) / 255.0f;

	return output;
}

Color Color::FromAbgr(u32 val)
{
	Color output;
	const u32 val32 = val;

	output.R = ((val32 >> 24) & 0xFF) / 255.0f;
	output.G = ((val32 >> 16) & 0xFF) / 255.0f;
	output.B = ((val32 >> 8) & 0xFF) / 255.0f;
	output.A = (val32 & 0xFF) / 255.0f;

	return output;
}

Color Color::FromArgb(ARGB val)
{
	Color output;
	const u32 val32 = val;

	output.B = ((val32 >> 24) & 0xFF) / 255.0f;
	output.G = ((val32 >> 16) & 0xFF) / 255.0f;
	output.R = ((val32 >> 8) & 0xFF) / 255.0f;
	output.A = (val32 & 0xFF) / 255.0f;

	return output;
}

Color Color::FromBgra(BGRA val)
{
	Color output;
	const u32 val32 = val;

	output.A = ((val32 >> 24) & 0xFF) / 255.0f;
	output.R = ((val32 >> 16) & 0xFF) / 255.0f;
	output.G = ((val32 >> 8) & 0xFF) / 255.0f;
	output.B = (val32 & 0xFF) / 255.0f;

	return output;
}

Color Color::FromHSB(float hue, float saturation, float brightness, float alpha)
{
	Color output;

	// wrap hue
	if(hue > 1.0f)
		hue -= (int)hue;
	else if(hue < 0.0f)
		hue += (int)hue + 1;

	// clamp saturation / brightness
	saturation = std::min(saturation, (float)1.0);
	saturation = std::max(saturation, (float)0.0);
	brightness = std::min(brightness, (float)1.0);
	brightness = std::max(brightness, (float)0.0);

	if(brightness == 0.0f)
	{
		// early exit, this has to be black
		output.R = output.G = output.B = 0.0f;
		return output;
	}

	if(saturation == 0.0f)
	{
		// early exit, this has to be grey

		output.R = output.G = output.B = brightness;
		return output;
	}

	float hueDomain = hue * 6.0f;
	if(hueDomain >= 6.0f)
	{
		// wrap around, and allow mathematical errors
		hueDomain = 0.0f;
	}

	const auto domain = (unsigned short)hueDomain;
	const float f1 = brightness * (1 - saturation);
	const float f2 = brightness * (1 - saturation * (hueDomain - domain));
	const float f3 = brightness * (1 - saturation * (1 - (hueDomain - domain)));

	switch(domain)
	{
	case 0:
		// red domain; green ascends
		output.R = brightness;
		output.G = f3;
		output.B = f1;
		break;
	case 1:
		// yellow domain; red descends
		output.R = f2;
		output.G = brightness;
		output.B = f1;
		break;
	case 2:
		// green domain; blue ascends
		output.R = f1;
		output.G = brightness;
		output.B = f3;
		break;
	case 3:
		// cyan domain; green descends
		output.R = f1;
		output.G = f2;
		output.B = brightness;
		break;
	case 4:
		// blue domain; red ascends
		output.R = f3;
		output.G = f1;
		output.B = brightness;
		break;
	case 5:
		// magenta domain; blue descends
		output.R = brightness;
		output.G = f1;
		output.B = f2;
		break;
	}

	output.A = alpha;
	return output;
}

Color Color::FromHSL(float hue, float saturation, float lightness, float alpha)
{
	auto fnHue = [](float hue, float m1, float m2)
	{
		if(hue < 0.0f) hue += 1.0f;
		if(hue > 1.0f) hue -= 1.0f;

		if(hue < 1.0f / 6.0f)
			return m1 + (m2 - m1) * hue * 6.0f;
		else if(hue < 3.0f / 6.0f)
			return m2;
		else if(hue < 4.0f / 6.0f)
			return m1 + (m2 - m1) * (2.0f / 3.0f - hue) * 6.0f;

		return m1;
	};

	hue = fmodf(hue, 1.0f);
	if(hue < 0.0f)
		hue += 1.0f;

	saturation = Math::Clamp01(saturation);
	lightness = Math::Clamp01(lightness);

	const float m2 = lightness <= 0.5f ? (lightness * (1 + saturation)) : (lightness + saturation - lightness * saturation);
	const float m1 = 2 * lightness - m2;

	Color output;
	output.R = Math::Clamp01(fnHue(hue + 1.0f / 3.0f, m1, m2));
	output.G = Math::Clamp01(fnHue(hue, m1, m2));
	output.B = Math::Clamp01(fnHue(hue - 1.0f / 3.0f, m1, m2));
	output.A = alpha;

	return output;
}

ABGR Color::GetAsAbgr() const
{
	u8 val8;
	u32 val32 = 0;

	// Convert to 32bit pattern
	// (RGBA = 8888)

	// Red
	val8 = static_cast<u8>(R * 255);
	val32 = val8 << 24;

	// Green
	val8 = static_cast<u8>(G * 255);
	val32 += val8 << 16;

	// Blue
	val8 = static_cast<u8>(B * 255);
	val32 += val8 << 8;

	// Alpha
	val8 = static_cast<u8>(A * 255);
	val32 += val8;

	return val32;
}

BGRA Color::GetAsBgra() const
{
	u8 val8;
	u32 val32 = 0;

	// Convert to 32bit pattern
	// (ARGB = 8888)

	// Alpha
	val8 = static_cast<u8>(A * 255);
	val32 = val8 << 24;

	// Red
	val8 = static_cast<u8>(R * 255);
	val32 += val8 << 16;

	// Green
	val8 = static_cast<u8>(G * 255);
	val32 += val8 << 8;

	// Blue
	val8 = static_cast<u8>(B * 255);
	val32 += val8;

	return val32;
}

ARGB Color::GetAsArgb() const
{
	u8 val8;
	u32 val32 = 0;

	// Convert to 32bit pattern
	// (ARGB = 8888)

	// Blue
	val8 = static_cast<u8>(B * 255);
	val32 = val8 << 24;

	// Green
	val8 = static_cast<u8>(G * 255);
	val32 += val8 << 16;

	// Red
	val8 = static_cast<u8>(R * 255);
	val32 += val8 << 8;

	// Alpha
	val8 = static_cast<u8>(A * 255);
	val32 += val8;

	return val32;
}

RGBA Color::GetAsRgba() const
{
	u8 val8;
	u32 val32 = 0;

	// Convert to 32bit pattern
	// (ABRG = 8888)

	// Alpha
	val8 = static_cast<u8>(A * 255);
	val32 = val8 << 24;

	// Blue
	val8 = static_cast<u8>(B * 255);
	val32 += val8 << 16;

	// Green
	val8 = static_cast<u8>(G * 255);
	val32 += val8 << 8;

	// Red
	val8 = static_cast<u8>(R * 255);
	val32 += val8;

	return val32;
}

float LinearToSrgb(float x)
{
	if(x <= 0.0f)
		return 0.0f;
	else if(x >= 1.0f)
		return 1.0f;
	else if(x < 0.0031308f)
		return x * 12.92f;
	else
		return std::pow(x, 1.0f / 2.4f) * 1.055f - 0.055f;
}

float SRGBToLinear(float x)
{
	if(x <= 0.0f)
		return 0.0f;
	else if(x >= 1.0f)
		return 1.0f;
	else if(x < 0.04045f)
		return x / 12.92f;
	else
		return std::pow((x + 0.055f) / 1.055f, 2.4f);
}

Color Color::GetGamma() const
{
	return Color(
		LinearToSrgb(R),
		LinearToSrgb(G),
		LinearToSrgb(B),
		A);
}

Color Color::GetLinear() const
{
	return Color(
		SRGBToLinear(R),
		SRGBToLinear(G),
		SRGBToLinear(B),
		A);
}

bool Color::operator==(const Color& rhs) const
{
	return (R == rhs.R && G == rhs.G && B == rhs.B && A == rhs.A);
}

bool Color::operator!=(const Color& rhs) const
{
	return !(*this == rhs);
}

void Color::GetHsb(float* outHue, float* outSaturation, float* outBrightness) const
{
	float vMin = std::min(R, std::min(G, B));
	float vMax = std::max(R, std::max(G, B));
	float delta = vMax - vMin;

	*outBrightness = vMax;

	if(Math::ApproxEquals(delta, 0.0f, 1e-6f))
	{
		// grey
		*outHue = 0;
		*outSaturation = 0;
	}
	else
	{
		// a colour
		*outSaturation = delta / vMax;

		float deltaR = (((vMax - R) / 6.0f) + (delta / 2.0f)) / delta;
		float deltaG = (((vMax - G) / 6.0f) + (delta / 2.0f)) / delta;
		float deltaB = (((vMax - B) / 6.0f) + (delta / 2.0f)) / delta;

		if(Math::ApproxEquals(R, vMax))
			*outHue = deltaB - deltaG;
		else if(Math::ApproxEquals(G, vMax))
			*outHue = 0.3333333f + deltaR - deltaB;
		else if(Math::ApproxEquals(B, vMax))
			*outHue = 0.6666667f + deltaG - deltaR;

		if(*outHue < 0.0f)
			*outHue += 1.0f;
		if(*outHue > 1.0f)
			*outHue -= 1.0f;
	}
}

Color Color::Lerp(float t, const Color& a, const Color& b)
{
	t = Math::Clamp01(t);
	return Color(a.R + (b.R - a.R) * t, a.G + (b.G - a.G) * t, a.B + (b.B - a.B) * t, a.A + (b.A - a.A) * t);
}
