//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalResourceTracker.h"

#include "B3DMetalBarrierHelper.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "Allocators/B3DFrameAllocator.h"
#include "Utility/B3DBitwise.h"

// Generic tracker method definitions, followed by the explicit instantiation for the Metal barrier
// helper. Included here (after the complete MetalBarrierHelper, GpuBackendUtility and frame
// allocator are available) so the single instantiation lives in this translation unit. The header
// carries a matching `extern template` to suppress implicit instantiation elsewhere.
#include "GpuBackend/B3DGpuResourceTracker.inl"

template class b3d::render::TGpuResourceTracker<b3d::render::MetalBarrierHelper>;

namespace b3d::render
{
	void MetalResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
	{
		for(u32 globalSubresourceIndex : mRenderPassSubresources)
			mSubresourceTrackingState[globalSubresourceIndex].ShaderUse = GpuAccessFlag::None;

		mRenderPassSubresources.clear();
	}
} // namespace b3d::render
