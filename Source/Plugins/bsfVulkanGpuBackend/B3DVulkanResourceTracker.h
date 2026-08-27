//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"


namespace b3d::render
{
	class VulkanBarrierHelper;
	/** @addtogroup Vulkan
	 *  @{
	 */

	extern template class TGpuResourceTracker<VulkanBarrierHelper>;

	/** Vulkan-specific resource tracker. Inherits the backend-agnostic tracking machinery from TGpuResourceTracker. */
	class VulkanResourceTracker : public TGpuResourceTracker<VulkanBarrierHelper>
	{
	public:
		/** Clears shader-related usage flags for all image subresources that were used during the current render pass. Usually called after render pass ends. */
		void ClearShaderFlagsForAllRenderPassImageSubresources();
	};

	/** @} */
} // namespace b3d::render
