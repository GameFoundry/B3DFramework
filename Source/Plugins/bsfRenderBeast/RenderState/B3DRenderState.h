//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"
#include "Utility/B3DUniformBufferPools.h"
#include "RenderAPI/B3DGpuBuffer.h"

namespace b3d::render
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Contains renderer-specific state for main thread objects such as renderables, decals, particle systems, etc. */
	struct RenderState
	{
		/** Handle for releasing the per-object uniform buffer allocation. */
		UniformBufferPools::AllocationHandle PerObjectBufferAllocationHandle;

		/** Shared parameter set for per-object data binding. */
		SPtr<GpuParameterSet> PerObjectParameterSet;

		/** Suballocation for per-object uniform buffer data. */
		GpuBufferSuballocation PerObjectSuballocation;

		/** Determines if the previous frame properties require updating. */
		PrevFrameDirtyState PrevFrameDirtyState = PrevFrameDirtyState::Clean;

		/** Current world transform matrix. */
		Matrix4 WorldTransform = Matrix4::kIdentity;

		/** World transform matrix without scale. */
		Matrix4 WorldNoScale = Matrix4::kIdentity;

		/** Previous frame's world transform matrix. */
		Matrix4 PrevWorldTransform = Matrix4::kIdentity;

		/** Render layer index. */
		u32 Layer = 0;
	};

	/** @} */
}
