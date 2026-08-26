//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12BarrierBatch.h"

using namespace b3d;
using namespace b3d::render;

bool D3D12BarrierBatch::IsEmpty() const
{
	return mBarrierGroups.Empty();
}

void D3D12BarrierBatch::AddGlobalBarrier(const D3D12_GLOBAL_BARRIER& barrier)
{
	AddToGroup(D3D12_BARRIER_TYPE_GLOBAL, (u32)mGlobalBarriers.Size());
	mGlobalBarriers.Add(barrier);
}

void D3D12BarrierBatch::AddBufferBarrier(const D3D12_BUFFER_BARRIER& barrier)
{
	B3D_ASSERT(barrier.pResource != nullptr);

	AddToGroup(D3D12_BARRIER_TYPE_BUFFER, (u32)mBufferBarriers.Size());
	mBufferBarriers.Add(barrier);
}

u32 D3D12BarrierBatch::AddTextureBarrier(const D3D12_TEXTURE_BARRIER& barrier)
{
	B3D_ASSERT(barrier.pResource != nullptr);
	B3D_ASSERT(barrier.Subresources.NumPlanes == 1);

	const u32 barrierIndex = (u32)mTextureBarriers.Size();
	AddToGroup(D3D12_BARRIER_TYPE_TEXTURE, barrierIndex);
	mTextureBarriers.Add(barrier);

	return barrierIndex;
}

void D3D12BarrierBatch::ReplaceTextureBarrier(u32 barrierIndex, const D3D12_TEXTURE_BARRIER& barrier)
{
	B3D_ASSERT(barrierIndex < mTextureBarriers.Size());
	B3D_ASSERT(barrier.pResource != nullptr);
	B3D_ASSERT(barrier.Subresources.NumPlanes == 1);

	mTextureBarriers[barrierIndex] = barrier;
}

void D3D12BarrierBatch::AddToGroup(D3D12_BARRIER_TYPE type, u32 barrierIndex)
{
	if(!mBarrierGroups.Empty() && mBarrierGroups.Back().Type == type)
	{
		mBarrierGroups.Back().BarrierCount++;
		return;
	}

	BarrierGroup group;
	group.Type = type;
	group.FirstBarrier = barrierIndex;
	group.BarrierCount = 1;

	mBarrierGroups.Add(group);
}

void D3D12BarrierBatch::Record(ID3D12GraphicsCommandList7& commandList) const
{
	for(const BarrierGroup& entry : mBarrierGroups)
	{
		D3D12_BARRIER_GROUP group{};
		group.Type = entry.Type;
		group.NumBarriers = entry.BarrierCount;
		switch(entry.Type)
		{
		case D3D12_BARRIER_TYPE_GLOBAL:
			group.pGlobalBarriers = mGlobalBarriers.Data() + entry.FirstBarrier;
			break;
		case D3D12_BARRIER_TYPE_BUFFER:
			group.pBufferBarriers = mBufferBarriers.Data() + entry.FirstBarrier;
			break;
		case D3D12_BARRIER_TYPE_TEXTURE:
			group.pTextureBarriers = mTextureBarriers.Data() + entry.FirstBarrier;
			break;
		default:
			B3D_ASSERT(false);
			continue;
		}

		// Separate calls preserve ordering between adjacent native barrier types.
		commandList.Barrier(1, &group);
	}
}

void D3D12BarrierBatch::Clear()
{
	mBarrierGroups.Clear();
	mGlobalBarriers.Clear();
	mBufferBarriers.Clear();
	mTextureBarriers.Clear();
}
