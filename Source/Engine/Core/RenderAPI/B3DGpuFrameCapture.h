//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** Allows capturing a range of GPU commands for analysis by external tools (such as RenderDoc, nSight). */
	class GpuFrameCapture
	{
	public:
		GpuFrameCapture() = default;
		virtual ~GpuFrameCapture() = default;

		/** Captures all GPU commands following this point. */
		virtual void Start() = 0;

		/** Stops capture started by Start() and makes the captured commands ready for analysis. */
		virtual void Stop() = 0;
	};
} // namespace b3d
