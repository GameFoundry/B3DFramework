//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BarrierHelper.h"
#include "B3DD3D12ResourceTracker.h"
#include "B3DD3D12BufferPool.h"
#include "B3DD3D12BarrierUtility.h"
#include "B3DD3D12GpuBuffer.h"
#include "B3DD3D12GpuCommandBuffer.h"
#include "B3DD3D12Texture.h"
#include "GpuBackend/B3DGpuBackendUtility.h"

using namespace b3d;
using namespace b3d::render;

#include "GpuBackend/B3DGpuBarrierHelper.inl"

template class b3d::render::TGpuBarrierHelper<b3d::render::D3D12BarrierHelper>;

D3D12BarrierHelper::D3D12BarrierHelper(D3D12ResourceTracker* resourceTracker, GpuQueueType queueType)
	: TGpuBarrierHelper<D3D12BarrierHelper>(resourceTracker), mQueueType(queueType)
{ }

void D3D12BarrierHelper::RecordSubresourceBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuBarrierScope& barrier, GpuImageLayout& oldLayout, GpuImageLayout newLayout)
{
	D3D12Image* const d3d12Image = static_cast<D3D12Image*>(image);
	ID3D12Resource* const resource = d3d12Image->GetD3D12Resource();
	const D3D12TextureLayout nativeOldLayout = d3d12Image->GetTextureLayout(oldLayout, mQueueType, range.AspectMask);
	const D3D12TextureLayout nativeNewLayout = d3d12Image->GetTextureLayout(newLayout, mQueueType, range.AspectMask);

	if(!B3D_ENSURE_LOG(mQueueType != GQT_TRANSFER || (nativeOldLayout == D3D12TextureLayout::Common() && nativeNewLayout == D3D12TextureLayout::Common()), "D3D12 copy queues cannot record texture layout transitions."))
		return;

	GpuBarrierScope nativeBarrier = barrier;
	if(oldLayout != GpuImageLayout::Undefined && !nativeBarrier.SourceAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write))
	{
		// A submission prologue can be prepended after this native command list has already been closed. Keep a leading
		// barrier without a tracked source dependency conservative so it chains with any prologue SyncAfter.
		// TODO - Use frame-graph-provided cross-command-buffer tracking to narrow this boundary scope.
		nativeBarrier.SourceStages = GpuStageFlag::All;
		// D3D12 requires a layout-compatible AccessBefore
		nativeBarrier.SourceAccess = GpuAccessFlag::Read | GpuAccessFlag::Write;
	}

	oldLayout = mBarriers.AddTextureBarrier(resource, range, nativeBarrier, oldLayout, newLayout, nativeOldLayout, nativeNewLayout, GetPrecedingBarrierDestinationStages(image, range));
}

void D3D12BarrierHelper::RecordBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier)
{
	D3D12BufferResource* const d3d12Buffer = static_cast<D3D12BufferResource*>(buffer);
	D3D12BufferPage* const page = d3d12Buffer->GetPage();
	const GpuStageFlags precedingBarrierDestinationStages = page != nullptr ? GetPrecedingBarrierDestinationStages(*page) : GetPrecedingBarrierDestinationStages(buffer);
	mBarriers.AddBufferBarrier(d3d12Buffer->GetD3D12Resource(), barrier, precedingBarrierDestinationStages);

	if(page != nullptr)
	{
		mPendingBufferPageBarriers.Add(PendingBufferPageBarrier(page, barrier));

		if(page != buffer)
		{
			BarrierTrackingInfo trackingInfo;
			trackingInfo.Buffer = page;
			trackingInfo.Barrier = barrier;

			mBarrierTracking.Add(trackingInfo);
		}
	}
}

GpuStageFlags D3D12BarrierHelper::GetPrecedingBarrierDestinationStages(IGpuBufferResource* buffer) const
{
	const GpuBufferTrackingState* const trackingState = mResourceTracker->FindBufferTrackingState(buffer);
	return trackingState != nullptr && trackingState->HazardState != nullptr ? trackingState->HazardState->LastBarrier.DestinationStages : GpuStageFlag::None;
}

GpuStageFlags D3D12BarrierHelper::GetPrecedingBarrierDestinationStages(IGpuImageResource* image, const GpuTextureSubresourceRange& range) const
{
	if(mResourceTracker->FindImageTrackingState(image) == nullptr)
		return GpuStageFlag::None;

	GpuStageFlags destinationStages = GpuStageFlag::None;
	for(const GpuImageSubresourceTrackingState& trackingState : mResourceTracker->GetSubresourceTrackingStatesForImage(image))
	{
		if(!GpuBackendUtility::RangeOverlaps(trackingState.Range, range) || trackingState.HazardState == nullptr)
			continue;

		destinationStages |= trackingState.HazardState->LastBarrier.DestinationStages;
	}

	return destinationStages;
}

GpuStageFlags D3D12BarrierHelper::GetPrecedingBarrierDestinationStages(D3D12BufferPage& page) const
{
	for(auto entry = mPendingBufferPageBarriers.rbegin(); entry != mPendingBufferPageBarriers.rend(); ++entry)
	{
		if(entry->Page == &page)
			return entry->Barrier.DestinationStages;
	}

	return GetPrecedingBarrierDestinationStages(&page);
}

void D3D12BarrierHelper::Execute(D3D12GpuCommandBuffer& commandBuffer)
{
	if(HasBarriers() || !mBarrierTracking.Empty() || !mImageLayoutTracking.Empty())
	{
		mBarriers.Record(*commandBuffer.GetD3D12Handle());
		ApplyPostBarrierTracking();
	}

	mResourceTracker->CommitPendingHazardRegistrations();
	Clear();
}

void D3D12BarrierHelper::Clear()
{
	mBarriers.Clear();
	mPendingBufferPageBarriers.Clear();
	TGpuBarrierHelper<D3D12BarrierHelper>::Clear();
}
