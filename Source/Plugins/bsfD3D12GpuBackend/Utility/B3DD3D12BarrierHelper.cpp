//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BarrierHelper.h"
#include "B3DD3D12ResourceTracker.h"
#include "B3DD3D12BarrierUtility.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12Texture.h"

using namespace b3d;
using namespace b3d::render;

// Generic barrier-helper method definitions, followed by the explicit instantiation for the D3D12 barrier helper.
// Included here (after the complete D3D12BarrierHelper and D3D12ResourceTracker are available) so the single
// instantiation lives in this translation unit. The header carries a matching `extern template` to suppress implicit
// instantiation elsewhere.
#include "GpuBackend/B3DGpuBarrierHelper.inl"

template class b3d::render::TGpuBarrierHelper<b3d::render::D3D12BarrierHelper>;

D3D12BarrierHelper::D3D12BarrierHelper(D3D12ResourceTracker* resourceTracker)
	: TGpuBarrierHelper<D3D12BarrierHelper>(resourceTracker)
{ }

void D3D12BarrierHelper::AppendTransition(ID3D12Resource* resource, u32 nativeSubresource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	// PRESENT and COMMON share value 0, so this also collapses redundant COMMON<->PRESENT transitions.
	if(resource == nullptr || before == after)
		return;

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = resource;
	barrier.Transition.Subresource = nativeSubresource;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;

	mNativeBarriers.Add(barrier);
}

void D3D12BarrierHelper::AppendUavBarrier(ID3D12Resource* resource)
{
	if(resource == nullptr)
		return;

	for(ID3D12Resource* existing : mUavBarrierResources)
	{
		if(existing == resource)
			return;
	}

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = resource;

	mNativeBarriers.Add(barrier);
	mUavBarrierResources.Add(resource);
}

void D3D12BarrierHelper::RequireBufferTransition(D3D12Buffer* buffer, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	if(buffer == nullptr)
		return;

	AppendTransition(buffer->GetD3D12Resource(), D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, before, after);
}

void D3D12BarrierHelper::RequireSubresourceState(D3D12Image* image, u32 face, u32 mipLevel, D3D12_RESOURCE_STATES state)
{
	if(image == nullptr || image->GetD3D12Resource() == nullptr)
		return;

	D3D12ImageSubresource* const subresource = image->GetD3D12Subresource(face, mipLevel);
	AppendTransition(image->GetD3D12Resource(), image->GetNativeSubresourceIndex(face, mipLevel), subresource->GetState(), state);
	subresource->SetState(state);
}

void D3D12BarrierHelper::RecordBufferBarrier(IGpuBufferResource* buffer, const GpuHazardStageAndAccess& barrier)
{
	// State-changing buffer transitions are emitted by RequireBufferTransition (a D3D12 transition barrier also
	// synchronizes both sides, resolving the hazard this hook was called for). The only hazard a transition cannot
	// express is a same-state write hazard, which for shader-visible buffers is UAV -> UAV.
	//
	// TODO(d3d12-port): A same-state non-UAV write hazard (COPY_DEST -> COPY_DEST WAW on one buffer) has no direct
	// D3D12 barrier; it would need a dummy transition round-trip. No engine path currently records two overlapping
	// copies into one buffer on a single command buffer.
	D3D12Buffer* const d3d12Buffer = static_cast<D3D12Buffer*>(buffer);

	// This hook fires from the shared tracker before the shadow updates the per-command-buffer state, so the
	// tracked state still reflects the buffer's PREVIOUS use on this command buffer.
	const D3D12_RESOURCE_STATES trackedState =
		static_cast<D3D12ResourceTracker*>(mResourceTracker)->GetTrackedBufferState(d3d12Buffer);

	const bool isWriteHazard = barrier.SourceAccess.IsSet(GpuAccessFlag::Write) ||
		barrier.DestinationAccess.IsSet(GpuAccessFlag::Write);
	if(isWriteHazard && (trackedState & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0)
		AppendUavBarrier(d3d12Buffer->GetD3D12Resource());
}

void D3D12BarrierHelper::RecordSubresourceBarrier(IGpuImageResource* image,
	const GpuTextureSubresourceRange& subresourceRange, const GpuHazardStageAndAccess& barrier,
	GpuImageLayout& oldLayout, GpuImageLayout newLayout)
{
	(void)oldLayout; // The native before-state comes from the per-subresource stored state, not the core layout.

	D3D12Image* const d3d12Image = static_cast<D3D12Image*>(image);
	ID3D12Resource* const nativeResource = d3d12Image->GetD3D12Resource();
	if(nativeResource == nullptr)
		return;

	// Derive the target state from the destination layout when one is provided; otherwise fall back to the
	// destination stage/access flags (buffer-style derivation covers the shader/transfer cases).
	const D3D12_RESOURCE_STATES targetState = newLayout != GpuImageLayout::Undefined
		? D3D12BarrierUtility::GetResourceStateFromLayout(newLayout, barrier.DestinationAccess)
		: D3D12BarrierUtility::GetBufferStateFromStages(barrier.DestinationStages, barrier.DestinationAccess);

	const bool isWriteHazard = barrier.SourceAccess.IsSet(GpuAccessFlag::Write) ||
		barrier.DestinationAccess.IsSet(GpuAccessFlag::Write);

	for(u32 layerIndex = 0; layerIndex < subresourceRange.ArrayLayerCount; layerIndex++)
	{
		for(u32 levelIndex = 0; levelIndex < subresourceRange.MipLevelCount; levelIndex++)
		{
			const u32 face = subresourceRange.BaseArrayLayer + layerIndex;
			const u32 mipLevel = subresourceRange.BaseMipLevel + levelIndex;

			D3D12ImageSubresource* const subresource = d3d12Image->GetD3D12Subresource(face, mipLevel);
			const D3D12_RESOURCE_STATES before = subresource->GetState();

			if(before == targetState)
			{
				// Same-state hazard: only UAV accesses need (and have) a dedicated barrier.
				if(isWriteHazard && (before & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0)
					AppendUavBarrier(nativeResource);

				continue;
			}

			AppendTransition(nativeResource, d3d12Image->GetNativeSubresourceIndex(face, mipLevel), before, targetState);
			subresource->SetState(targetState);
		}
	}
}

void D3D12BarrierHelper::Execute(D3D12GpuCommandBuffer& commandBuffer)
{
	if(HasBarriers() || !mBarrierTracking.Empty() || !mImageLayoutTracking.Empty())
	{
		if(!mNativeBarriers.Empty())
			commandBuffer.GetD3D12Handle()->ResourceBarrier((UINT)mNativeBarriers.size(), mNativeBarriers.data());

		// Shared bookkeeping: advances the tracked layouts and marks the source->destination pairs safe to access.
		ApplyPostBarrierTracking();
	}

	// Apply the read/write hazard registrations that were deferred while tracking this dispatch/draw's resources
	mResourceTracker->CommitPendingHazardRegistrations();

	Clear();
}

void D3D12BarrierHelper::Clear()
{
	mNativeBarriers.Clear();
	mUavBarrierResources.Clear();

	TGpuBarrierHelper<D3D12BarrierHelper>::Clear();
}
