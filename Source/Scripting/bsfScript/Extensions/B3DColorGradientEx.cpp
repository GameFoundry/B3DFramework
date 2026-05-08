//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DColorGradientEx.h"

using namespace b3d;
Color ColorGradientEx::Evaluate(const TShared<ColorGradient>& thisPtr, float t)
{
	return Color::FromRgba(thisPtr->Evaluate(t));
}

Color ColorGradientHDREx::Evaluate(const TShared<ColorGradientHDR>& thisPtr, float t)
{
	return thisPtr->Evaluate(t);
}
