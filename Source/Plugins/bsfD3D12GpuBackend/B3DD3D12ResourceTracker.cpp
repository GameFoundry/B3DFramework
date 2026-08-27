//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12ResourceTracker.h"

#include "B3DD3D12BufferPool.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12Texture.h"
#include "Utility/B3DD3D12BarrierHelper.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

#include "GpuBackend/B3DGpuResourceTracker.inl"

template class b3d::render::TGpuResourceTracker<b3d::render::D3D12BarrierHelper>;

using namespace b3d;
using namespace b3d::render;

void D3D12ResourceTracker::TrackBufferUsage(IGpuBufferResource* buffer, GpuResourceUseFlags useFlags,
	GpuAccessFlags accessFlags, D3D12BarrierHelper& barrierHelper, u32 dynamicOffset)
{
	if(buffer == nullptr)
		return;

	D3D12BufferResource* const d3d12Buffer = static_cast<D3D12BufferResource*>(buffer);
	D3D12BufferPage* const page = d3d12Buffer->GetPage();
	if(page != nullptr && page != buffer && accessFlags.IsSet(GpuAccessFlag::Write))
	{
		TGpuResourceTracker<D3D12BarrierHelper>::TrackBufferUsage(page, useFlags, GpuAccessFlag::Write, barrierHelper);
	}

	TGpuResourceTracker<D3D12BarrierHelper>::TrackBufferUsage(buffer, useFlags, accessFlags, barrierHelper, dynamicOffset);
}

void D3D12ResourceTracker::ClearShaderFlagsForAllRenderPassImageSubresources()
{
	for (u32 subresourceIndex : mRenderPassSubresources)
		mSubresourceTrackingState[subresourceIndex].ShaderUse = GpuAccessFlag::None;

	mRenderPassSubresources.clear();
}
