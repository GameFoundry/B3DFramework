//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanResourceTracker.h"

#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Allocators/B3DFrameAllocator.h"
#include "Utility/B3DBitwise.h"
#include "Utility/B3DVulkanBarrierHelper.h"

// Generic tracker method definitions, followed by the explicit instantiation for the Vulkan barrier helper. Included
// here (after the complete VulkanBarrierHelper, GpuBackendUtility and frame allocator are available) so the single
// instantiation lives in this translation unit. The header carries a matching `extern template` to suppress implicit
// instantiation elsewhere.
#include "GpuBackend/B3DGpuResourceTracker.inl"

template class b3d::render::TGpuResourceTracker<b3d::render::VulkanBarrierHelper>;

using namespace b3d;
using namespace b3d::render;

void VulkanResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
{
	for(const auto& subresourceIndex : mRenderPassSubresources)
	{
		GpuImageSubresourceTrackingState& subresourceTrackingState = mSubresourceTrackingState[subresourceIndex];
		subresourceTrackingState.ShaderUse = GpuAccessFlag::None;
	}

	mRenderPassSubresources.clear();
}
