//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DParticleDistributionEx.h"

using namespace b3d;
Color ColorDistributionEx::Evaluate(const SPtr<ColorDistribution>& thisPtr, float t, float factor)
{
	return Color::FromRgba(thisPtr->Evaluate(t, factor));
}

class Color ColorDistributionEx::Evaluate(const SPtr<ColorDistribution>& thisPtr, float t, Random& factor)
{
	return Color::FromRgba(thisPtr->Evaluate(t, factor));
}
