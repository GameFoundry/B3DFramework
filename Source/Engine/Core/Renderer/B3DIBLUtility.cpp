//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/B3DIBLUtility.h"

using namespace b3d;

namespace b3d { namespace render
{
const u32 IBLUtility::kReflectionCubemapSize = 256;
const u32 IBLUtility::kIrradianceCubemapSize = 32;

/** Returns the size of the texture required to store the provided number of SH coefficients. */
Vector2I IBLUtility::GetShCoeffTextureSize(u32 numCoeffSets, u32 shOrder)
{
	u32 coeffsPerSet = shOrder * shOrder;

	// Assuming the texture maximum size is 4096
	u32 maxSetsPerRow = 4096 / coeffsPerSet;

	Vector2I output;
	output.X = (numCoeffSets > maxSetsPerRow ? maxSetsPerRow : numCoeffSets) * coeffsPerSet;
	output.Y = 1 + numCoeffSets / (maxSetsPerRow + 1);

	return output;
}

/** Determines the position of a set of coefficients in the coefficient texture, depending on the coefficient index. */
Vector2I IBLUtility::GetShCoeffXyFromIdx(u32 idx, u32 shOrder)
{
	u32 coeffsPerSet = shOrder * shOrder;

	// Assuming the texture maximum size is 4096
	u32 maxSetsPerRow = 4096 / coeffsPerSet;

	Vector2I output;
	output.X = (idx % maxSetsPerRow) * coeffsPerSet;
	output.Y = idx / maxSetsPerRow;

	return output;
}

const IBLUtility& GetIBLUtility()
{
	return IBLUtility::Instance();
}
}}
