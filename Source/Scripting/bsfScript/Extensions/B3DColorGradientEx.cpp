//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsColorGradientEx.h"

using namespace b3d;
Color ColorGradientEx::Evaluate(const SPtr<ColorGradient>& thisPtr, float t)
{
	return Color::FromRgba(thisPtr->Evaluate(t));
}

Color ColorGradientHDREx::Evaluate(const SPtr<ColorGradientHDR>& thisPtr, float t)
{
	return thisPtr->Evaluate(t);
}
