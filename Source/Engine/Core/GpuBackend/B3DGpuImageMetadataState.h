//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d::render
{
	/** Backend-owned meta-data values. */
	class GpuImageMetadataState
	{
	public:
		GpuImageMetadataState() = default;
		virtual ~GpuImageMetadataState() = default;

		/** Returns an independent copy for another tracking range. */
		virtual TShared<GpuImageMetadataState> Clone() const = 0;
	};
}
