//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
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

	/** Base class for all renderable objects. */
	struct RendererObject
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
