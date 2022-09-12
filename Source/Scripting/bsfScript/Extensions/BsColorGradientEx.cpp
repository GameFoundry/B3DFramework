//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsColorGradientEx.h"

namespace bs
{
	Color ColorGradientEx::Evaluate(const SPtr<ColorGradient>& thisPtr, float t)
	{
		return Color::FromRGBA(thisPtr->evaluate(t));
	}

	Color ColorGradientHDREx::Evaluate(const SPtr<ColorGradientHDR>& thisPtr, float t)
	{
		return thisPtr->evaluate(t);
	}
}
