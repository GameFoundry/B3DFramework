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

#include <algorithm>

using namespace b3d;
using namespace b3d::render;

#include "GpuBackend/B3DGpuBarrierHelper.inl"

template class b3d::render::TGpuBarrierHelper<b3d::render::D3D12BarrierHelper, b3d::render::D3D12ResourceTracker>;

D3D12BarrierHelper::D3D12BarrierHelper(D3D12ResourceTracker* resourceTracker, GpuQueueType queueType)
	: TGpuBarrierHelper<D3D12BarrierHelper, D3D12ResourceTracker>(resourceTracker), mQueueType(queueType)
{ }

void D3D12BarrierHelper::RecordNativeImageBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuBarrierScope& barrier, GpuImageLayout oldLayout, GpuImageLayout newLayout, GpuBarrierFlags barrierFlags)
{
	D3D12Image* const d3d12Image = static_cast<D3D12Image*>(image);
	const bool discardContents = barrierFlags.IsSet(GpuBarrierFlag::DiscardContents);
	const GpuImageLayout barrierOldLayout = discardContents ? GpuImageLayout::Undefined : oldLayout;
	const D3D12TextureLayout nativeOldLayout = discardContents ? D3D12TextureLayout::Undefined() : d3d12Image->GetTextureLayout(oldLayout, mQueueType);
	const D3D12TextureLayout nativeNewLayout = d3d12Image->GetTextureLayout(newLayout, mQueueType);
	const D3D12_TEXTURE_BARRIER_FLAGS nativeBarrierFlags = discardContents ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE;

	if(!B3D_ENSURE_LOG(mQueueType != GQT_TRANSFER || (nativeOldLayout == D3D12TextureLayout::Common() && nativeNewLayout == D3D12TextureLayout::Common()), "D3D12 copy queues cannot record texture layout transitions."))
		return;

	// If it a texture has a layout, it must have an access scope
	B3D_ASSERT(barrierOldLayout == GpuImageLayout::Undefined || barrier.SourceAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write));

	// An alias acquire's source belongs to earlier resources on the memory. A discarding texture barrier has AccessBefore NO_ACCESS and flushes
	// nothing, so the source gets a global barrier. Its group is recorded in a separate Barrier() call before the discard: a discard and an
	// access of overlapping memory in one call break the enhanced-barrier rules.
	if(barrierFlags.IsSet(GpuBarrierFlag::AliasAcquire) && barrier.SourceStages != GpuStageFlag::None)
		AddAliasGlobalBarrier(barrier);

	auto found = std::find_if(mPendingImageBarriers.begin(), mPendingImageBarriers.end(), [image, &range](const PendingImageBarrier& pendingBarrier)
	{
		return pendingBarrier.Image == image && GpuBackendUtility::RangeEquals(pendingBarrier.SubresourceRange, range);
	});

	if(found != mPendingImageBarriers.end())
	{
		found->Barrier.SourceStages |= barrier.SourceStages;
		found->Barrier.SourceAccess |= barrier.SourceAccess;

		if(found->NewLayout == newLayout)
		{
			found->Barrier.DestinationStages |= barrier.DestinationStages;
			found->Barrier.DestinationAccess |= barrier.DestinationAccess;
		}
		else
		{
			found->Barrier.DestinationStages = barrier.DestinationStages;
			found->Barrier.DestinationAccess = barrier.DestinationAccess;
		}

		found->NewLayout = newLayout;
		found->BarrierFlags |= barrierFlags;
		found->PrecedingBarrierDestinationStages |= GetPrecedingBarrierDestinationStages(image, range);

		const bool mergedDiscardContents = found->BarrierFlags.IsSet(GpuBarrierFlag::DiscardContents);
		const GpuImageLayout resolvedOldLogicalLayout = mergedDiscardContents ? GpuImageLayout::Undefined : found->OldLayout;
		const D3D12TextureLayout resolvedOldLayout = mergedDiscardContents ? D3D12TextureLayout::Undefined() : d3d12Image->GetTextureLayout(found->OldLayout, mQueueType);
		const D3D12TextureLayout resolvedNewLayout = d3d12Image->GetTextureLayout(found->NewLayout, mQueueType);
		const D3D12_TEXTURE_BARRIER_FLAGS mergedNativeBarrierFlags = mergedDiscardContents ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE;
		const D3D12_TEXTURE_BARRIER nativeBarrier = D3D12BarrierUtility::GetTextureBarrier(d3d12Image->GetD3D12Resource(), found->SubresourceRange, found->Barrier, resolvedOldLogicalLayout, found->NewLayout, resolvedOldLayout, resolvedNewLayout, mergedNativeBarrierFlags, found->PrecedingBarrierDestinationStages);

		mBarriers.ReplaceTextureBarrier(found->NativeBarrierIndex, nativeBarrier);
		return;
	}

	PendingImageBarrier pendingBarrier;
	pendingBarrier.Image = image;
	pendingBarrier.SubresourceRange = range;
	pendingBarrier.Barrier = barrier;
	pendingBarrier.OldLayout = oldLayout;
	pendingBarrier.NewLayout = newLayout;
	pendingBarrier.BarrierFlags = barrierFlags;
	pendingBarrier.PrecedingBarrierDestinationStages = GetPrecedingBarrierDestinationStages(image, range);
	pendingBarrier.NativeBarrierIndex = mBarriers.AddTextureBarrier(D3D12BarrierUtility::GetTextureBarrier(d3d12Image->GetD3D12Resource(), range, barrier, barrierOldLayout, newLayout, nativeOldLayout, nativeNewLayout, nativeBarrierFlags, pendingBarrier.PrecedingBarrierDestinationStages));

	mPendingImageBarriers.Add(pendingBarrier);
}

void D3D12BarrierHelper::RecordNativeBufferBarrier(IGpuBufferResource* buffer, const GpuBarrierScope& barrier, GpuBarrierFlags barrierFlags)
{
	D3D12BufferResource* const d3d12Buffer = static_cast<D3D12BufferResource*>(buffer);
	D3D12BufferPage* const page = d3d12Buffer->GetPage();
	const GpuStageFlags precedingBarrierDestinationStages = page != nullptr ? GetPrecedingBarrierDestinationStages(*page) : GetPrecedingBarrierDestinationStages(buffer);

	// An alias acquire's source belongs to earlier resources on the memory, which a barrier on this buffer's resource does not cover
	if(barrierFlags.IsSet(GpuBarrierFlag::AliasAcquire))
		AddAliasGlobalBarrier(barrier);
	else if(d3d12Buffer->GetHeapType() == D3D12_HEAP_TYPE_READBACK)
	{
		// Agility SDK 1.619 reports BARRIER_INTEROP_INVALID_STATE for resource-scoped enhanced barriers on READBACK
		// buffers, even when created with CreatePlacedResource2 and UNDEFINED. The enhanced-barrier specification
		// explicitly permits these barriers for readback WAW hazards, so retain the dependency through a global barrier.
		// TODO - Restore the page-scoped buffer barrier once the D3D12 debug layer accepts enhanced READBACK barriers.
		mBarriers.AddGlobalBarrier(D3D12BarrierUtility::GetGlobalBufferBarrier(D3D12_RESOURCE_FLAG_NONE, barrier, precedingBarrierDestinationStages));
	}
	else
		mBarriers.AddBufferBarrier(D3D12BarrierUtility::GetBufferBarrier(d3d12Buffer->GetD3D12Resource(), barrier, precedingBarrierDestinationStages));

	if(page != nullptr)
	{
		mPendingBufferPageBarriers.Add(PendingBufferPageBarrier(page, barrier));

		// Before the page's first access on the command buffer, the barrier orders none of the page's accesses. Recording it would remove that
		// first access from the page's submission barrier, which orders it after the page's earlier submissions.
		const GpuBufferTrackingState* const pageTrackingState = mResourceTracker->FindBufferTrackingState(page);
		const bool pageHasAccess = pageTrackingState != nullptr && pageTrackingState->HazardState != nullptr && pageTrackingState->HazardState->HasAccess();
		if(page != buffer && pageHasAccess)
		{
			BarrierTrackingInfo trackingInfo;
			trackingInfo.Buffer = page;
			trackingInfo.Barrier = barrier;

			mBarrierTracking.Add(trackingInfo);
		}
	}
}

void D3D12BarrierHelper::AddAliasGlobalBarrier(const GpuBarrierScope& barrier)
{
	// The first acquire's global barrier precedes its discard, and with it the discards of every later acquire in the batch
	if(mAliasGlobalBarrierIndex == ~0u)
	{
		mAliasGlobalBarrierScope = barrier;
		mAliasGlobalBarrierIndex = mBarriers.AddGlobalBarrier(D3D12BarrierUtility::GetAliasGlobalBarrier(mAliasGlobalBarrierScope));
		return;
	}

	mAliasGlobalBarrierScope.SourceStages |= barrier.SourceStages;
	mAliasGlobalBarrierScope.SourceAccess |= barrier.SourceAccess;
	mAliasGlobalBarrierScope.DestinationStages |= barrier.DestinationStages;
	mAliasGlobalBarrierScope.DestinationAccess |= barrier.DestinationAccess;
	mBarriers.ReplaceGlobalBarrier(mAliasGlobalBarrierIndex, D3D12BarrierUtility::GetAliasGlobalBarrier(mAliasGlobalBarrierScope));
}

GpuStageFlags D3D12BarrierHelper::GetPrecedingBarrierDestinationStages(IGpuBufferResource* buffer) const
{
	const GpuBufferTrackingState* const trackingState = mResourceTracker->FindBufferTrackingState(buffer);
	if(trackingState == nullptr || trackingState->HazardState == nullptr)
		return GpuStageFlag::None;

	const GpuResourceHazardState& hazardState = *trackingState->HazardState;
	if(hazardState.LastBarrier.DestinationStages != GpuStageFlag::None)
		return hazardState.LastBarrier.DestinationStages;

	return hazardState.GetSubmissionBarrierAccessScope().GetStages();
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

		const GpuResourceHazardState& hazardState = *trackingState.HazardState;
		if(hazardState.LastBarrier.DestinationStages != GpuStageFlag::None)
			destinationStages |= hazardState.LastBarrier.DestinationStages;
		else
			destinationStages |= hazardState.GetSubmissionBarrierAccessScope().GetStages();
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

	mResourceTracker->CommitPendingAccesses();
	Clear();
}

void D3D12BarrierHelper::Clear()
{
	mBarriers.Clear();
	mPendingBufferPageBarriers.Clear();
	mPendingImageBarriers.Clear();
	mAliasGlobalBarrierScope = GpuBarrierScope();
	mAliasGlobalBarrierIndex = ~0u;
	TGpuBarrierHelper<D3D12BarrierHelper, D3D12ResourceTracker>::Clear();
}
