//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BarrierBatch.h"
#include "B3DD3D12BarrierUtility.h"
#include "Allocators/B3DFrameAllocator.h"

#include <algorithm>

using namespace b3d;
using namespace b3d::render;

namespace
{
	bool SubresourceRangesEqual(const D3D12_BARRIER_SUBRESOURCE_RANGE& lhs, const D3D12_BARRIER_SUBRESOURCE_RANGE& rhs)
	{
		return lhs.IndexOrFirstMipLevel == rhs.IndexOrFirstMipLevel
			&& lhs.NumMipLevels == rhs.NumMipLevels
			&& lhs.FirstArraySlice == rhs.FirstArraySlice
			&& lhs.NumArraySlices == rhs.NumArraySlices
			&& lhs.FirstPlane == rhs.FirstPlane
			&& lhs.NumPlanes == rhs.NumPlanes;
	}
}

bool D3D12BarrierBatch::IsEmpty() const
{
	if(!mBufferBarriers.Empty())
		return false;

	for(const TextureBarrierEntry& entry : mTextureBarriers)
	{
		if(entry.EmitsNativeBarrier)
			return false;
	}

	return true;
}

void D3D12BarrierBatch::AddBufferBarrier(ID3D12Resource* resource, const GpuBarrierScope& scope)
{
	if(resource == nullptr || !scope.IsValid())
		return;

	const D3D12_RESOURCE_FLAGS resourceFlags = resource->GetDesc().Flags;
	const D3D12BarrierScope beforeScope = D3D12BarrierUtility::GetBufferScope(scope.SourceStages, scope.SourceAccess, resourceFlags);
	const D3D12BarrierScope afterScope = D3D12BarrierUtility::GetBufferScope(scope.DestinationStages, scope.DestinationAccess, resourceFlags);

	D3D12_BUFFER_BARRIER barrier = {};
	// TODO - Track the previous native SyncAfter per resource and narrow these scopes after profiling. SYNC_ALL keeps
	// pairwise enhanced-barrier chaining valid without adding D3D12 state to the shared tracker.
	barrier.SyncBefore = D3D12_BARRIER_SYNC_ALL;
	barrier.SyncAfter = D3D12_BARRIER_SYNC_ALL;
	barrier.AccessBefore = beforeScope.Access;
	barrier.AccessAfter = afterScope.Access;
	barrier.pResource = resource;
	barrier.Offset = 0;
	barrier.Size = UINT64_MAX;

	mBufferBarriers.Add(barrier);
}

GpuImageLayout D3D12BarrierBatch::AddTextureBarrier(ID3D12Resource* resource, const GpuTextureSubresourceRange& range, const GpuBarrierScope& scope, GpuImageLayout logicalBeforeLayout, GpuImageLayout logicalAfterLayout, const D3D12TextureLayout& nativeBeforeLayout, const D3D12TextureLayout& nativeAfterLayout)
{
	if(resource == nullptr)
		return logicalBeforeLayout;

	GpuImageLayout effectiveBeforeLayout = logicalBeforeLayout;

	auto fnAddAspectBarrier = [&](const GpuTextureSubresourceRange& subresourceRange)
	{
		const D3D12_BARRIER_LAYOUT layoutBefore = nativeBeforeLayout.GetLayout(subresourceRange.AspectMask);
		const D3D12_BARRIER_LAYOUT layoutAfter = nativeAfterLayout.GetLayout(subresourceRange.AspectMask);

		const bool hasDepth = subresourceRange.AspectMask.IsSet(GpuTextureAspectFlag::Depth);
		const bool hasStencil = subresourceRange.AspectMask.IsSet(GpuTextureAspectFlag::Stencil);

		D3D12_BARRIER_SUBRESOURCE_RANGE nativeRange{};
		nativeRange.IndexOrFirstMipLevel = subresourceRange.BaseMipLevel;
		nativeRange.NumMipLevels = subresourceRange.MipLevelCount;
		nativeRange.FirstArraySlice = subresourceRange.BaseArrayLayer;
		nativeRange.NumArraySlices = subresourceRange.ArrayLayerCount;
		nativeRange.NumPlanes = 1;

		if(hasStencil && !hasDepth)
			nativeRange.FirstPlane = 1;
		else if(hasStencil)
			nativeRange.NumPlanes = 2;

		auto found = std::find_if(mTextureBarriers.begin(), mTextureBarriers.end(),
			[resource, &nativeRange](const TextureBarrierEntry& entry)
		{
			return entry.Barrier.pResource == resource && SubresourceRangesEqual(entry.Barrier.Subresources, nativeRange);
		});

		if(found != mTextureBarriers.end())
		{
			found->Scope.SourceStages |= scope.SourceStages;
			found->Scope.SourceAccess |= scope.SourceAccess;
			if(found->LogicalAfterLayout == logicalAfterLayout)
			{
				found->Scope.DestinationStages |= scope.DestinationStages;
				found->Scope.DestinationAccess |= scope.DestinationAccess;
			}
			else
			{
				// No resource operation occurs between queued barriers. When the requested layout changes again, only
				// the final destination scope consumes the consolidated transition.
				found->Scope.DestinationStages = scope.DestinationStages;
				found->Scope.DestinationAccess = scope.DestinationAccess;
			}
			found->LogicalAfterLayout = logicalAfterLayout;
			found->Barrier.LayoutAfter = layoutAfter;

			RebuildTextureBarrier(*found);
			effectiveBeforeLayout = found->LogicalBeforeLayout;
			return;
		}

		TextureBarrierEntry entry;
		entry.Barrier.LayoutBefore = layoutBefore;
		entry.Barrier.LayoutAfter = layoutAfter;
		entry.Barrier.pResource = resource;
		entry.Barrier.Subresources = nativeRange;
		entry.Scope = scope;
		entry.LogicalBeforeLayout = logicalBeforeLayout;
		entry.LogicalAfterLayout = logicalAfterLayout;
		entry.Aspects = subresourceRange.AspectMask;

		RebuildTextureBarrier(entry);

		if(entry.EmitsNativeBarrier || logicalBeforeLayout != logicalAfterLayout)
			mTextureBarriers.Add(entry);
	};

	const bool hasDepth = range.AspectMask.IsSet(GpuTextureAspectFlag::Depth);
	const bool hasStencil = range.AspectMask.IsSet(GpuTextureAspectFlag::Stencil);
	const bool splitAspects = hasDepth && hasStencil && (nativeBeforeLayout.HasSeparateDepthStencilLayouts() || nativeAfterLayout.HasSeparateDepthStencilLayouts());
	if(splitAspects)
	{
		GpuTextureSubresourceRange aspectRange = range;
		aspectRange.AspectMask = GpuTextureAspectFlag::Depth;
		fnAddAspectBarrier(aspectRange);

		aspectRange.AspectMask = GpuTextureAspectFlag::Stencil;
		fnAddAspectBarrier(aspectRange);
	}
	else
		fnAddAspectBarrier(range);

	return effectiveBeforeLayout;
}

void D3D12BarrierBatch::RebuildTextureBarrier(TextureBarrierEntry& entry)
{
	D3D12_TEXTURE_BARRIER& barrier = entry.Barrier;
	const bool discard = barrier.LayoutBefore == D3D12_BARRIER_LAYOUT_UNDEFINED;
	const bool hasSourceAccess = entry.Scope.SourceAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write);
	const bool hasDestinationAccess = entry.Scope.DestinationAccess.IsSetAny(GpuAccessFlag::Read | GpuAccessFlag::Write);
	const bool layoutOnly = !hasSourceAccess && !hasDestinationAccess;

	const D3D12BarrierScope beforeScope = D3D12BarrierUtility::GetTextureScope(entry.Scope.SourceStages, entry.Scope.SourceAccess, entry.LogicalBeforeLayout, entry.Aspects);
	barrier.AccessBefore = beforeScope.Access;
	barrier.SyncBefore = beforeScope.Sync;

	const D3D12BarrierScope afterScope = D3D12BarrierUtility::GetTextureScope(entry.Scope.DestinationStages, entry.Scope.DestinationAccess, entry.LogicalAfterLayout, entry.Aspects);
	barrier.AccessAfter = afterScope.Access;
	barrier.SyncAfter = afterScope.Sync;
	barrier.Flags = discard ? D3D12_TEXTURE_BARRIER_FLAG_DISCARD : D3D12_TEXTURE_BARRIER_FLAG_NONE;

	const bool redundantLayoutOnly = layoutOnly && !discard && barrier.LayoutBefore == barrier.LayoutAfter;
	const bool redundantTrackedBarrier = !discard && barrier.LayoutBefore == barrier.LayoutAfter && !entry.Scope.IsValid();
	entry.EmitsNativeBarrier = !redundantLayoutOnly && !redundantTrackedBarrier;
}

void D3D12BarrierBatch::Record(ID3D12GraphicsCommandList7& commandList) const
{
	FrameAllocatorScope frameScope;
	FrameVector<D3D12_TEXTURE_BARRIER> textureBarriers;
	textureBarriers.reserve(mTextureBarriers.Size());
	for(const TextureBarrierEntry& entry : mTextureBarriers)
	{
		if(entry.EmitsNativeBarrier)
			textureBarriers.push_back(entry.Barrier);
	}

	D3D12_BARRIER_GROUP groups[2] = {};
	u32 groupCount = 0;
	if(!mBufferBarriers.Empty())
	{
		D3D12_BARRIER_GROUP& group = groups[groupCount++];
		group.Type = D3D12_BARRIER_TYPE_BUFFER;
		group.NumBarriers = (u32)mBufferBarriers.Size();
		group.pBufferBarriers = mBufferBarriers.Data();
	}

	if(!textureBarriers.empty())
	{
		D3D12_BARRIER_GROUP& group = groups[groupCount++];
		group.Type = D3D12_BARRIER_TYPE_TEXTURE;
		group.NumBarriers = (u32)textureBarriers.size();
		group.pTextureBarriers = textureBarriers.data();
	}

	if(groupCount > 0)
		commandList.Barrier(groupCount, groups);
}

void D3D12BarrierBatch::Clear()
{
	mBufferBarriers.Clear();
	mTextureBarriers.Clear();
}
