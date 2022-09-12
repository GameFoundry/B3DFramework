//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsIBLUtility.h"
#include "Math/BsVector2I.h"

namespace bs { namespace ct
{
	const UINT32 IBLUtility::REFLECTION_CUBEMAP_SIZE = 256;
	const UINT32 IBLUtility::IRRADIANCE_CUBEMAP_SIZE = 32;

	/** Returns the size of the texture required to store the provided number of SH coefficients. */
	Vector2I IBLUtility::GetSHCoeffTextureSize(UINT32 numCoeffSets, UINT32 shOrder)
	{
		UINT32 coeffsPerSet = shOrder * shOrder;
		
		// Assuming the texture maximum size is 4096
		UINT32 maxSetsPerRow = 4096 / coeffsPerSet;
		
		Vector2I output;
		output.x = (numCoeffSets > maxSetsPerRow ? maxSetsPerRow : numCoeffSets) * coeffsPerSet;
		output.y = 1 + numCoeffSets / (maxSetsPerRow + 1);

		return output;
	}
	
	/** Determines the position of a set of coefficients in the coefficient texture, depending on the coefficient index. */
	Vector2I IBLUtility::GetSHCoeffXYFromIdx(UINT32 idx, UINT32 shOrder)
	{
		UINT32 coeffsPerSet = shOrder * shOrder;
		
		// Assuming the texture maximum size is 4096
		UINT32 maxSetsPerRow = 4096 / coeffsPerSet;

		Vector2I output;
		output.x = (idx % maxSetsPerRow) * coeffsPerSet;
		output.y = idx / maxSetsPerRow;
		
		return output;
	}

	const IBLUtility& GIBLUtility()
	{
		return IBLUtility::Instance();
	}
}}
