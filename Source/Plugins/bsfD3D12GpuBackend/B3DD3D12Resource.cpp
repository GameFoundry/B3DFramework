//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DD3D12Resource.h"
#include "B3DD3D12ResourceManager.h"

using namespace b3d;
using namespace b3d::render;

template<class TBase>
void TD3D12Resource<TBase>::OnNotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags)
{
	// Called under IGpuResource::mMutex from inside NotifyUsed, after the aggregate use counter has been incremented.
	B3D_ASSERT(queueId.Id < kMaximumUniqueQueueCount);

	if(useFlags.IsSet(GpuAccessFlag::Read))
	{
		B3D_ASSERT(mReadUses[queueId.Id] < 255 && "Resource used in too many command buffers at once.");
		mReadUses[queueId.Id]++;
	}

	if(useFlags.IsSet(GpuAccessFlag::Write))
	{
		B3D_ASSERT(mWriteUses[queueId.Id] < 255 && "Resource used in too many command buffers at once.");
		mWriteUses[queueId.Id]++;
	}
}

template<class TBase>
void TD3D12Resource<TBase>::OnNotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags)
{
	// Called under IGpuResource::mMutex from inside NotifyDone, after the aggregate counters have been decremented.
	if(useFlags.IsSet(GpuAccessFlag::Read))
	{
		B3D_ASSERT(mReadUses[queueId.Id] > 0);
		mReadUses[queueId.Id]--;
	}

	if(useFlags.IsSet(GpuAccessFlag::Write))
	{
		B3D_ASSERT(mWriteUses[queueId.Id] > 0);
		mWriteUses[queueId.Id]--;
	}
}

template<class TBase>
GpuQueueMask TD3D12Resource<TBase>::GetUseInfo(GpuAccessFlags useFlags) const
{
	GpuQueueMask mask = 0;

	Lock lock(this->mMutex);

	if(useFlags.IsSet(GpuAccessFlag::Read))
	{
		for(u32 i = 0; i < kMaximumUniqueQueueCount; i++)
		{
			if(mReadUses[i] > 0)
				mask |= GpuQueueId(i);
		}
	}

	if(useFlags.IsSet(GpuAccessFlag::Write))
	{
		for(u32 i = 0; i < kMaximumUniqueQueueCount; i++)
		{
			if(mWriteUses[i] > 0)
				mask |= GpuQueueId(i);
		}
	}

	return mask;
}

template<class TBase>
D3D12GpuDevice& TD3D12Resource<TBase>::GetDevice() const
{
	return mOwner->GetDevice();
}

namespace b3d::render
{
	template class TD3D12Resource<IGpuResource>;
	template class TD3D12Resource<IGpuBufferResource>;
	template class TD3D12Resource<IGpuImageResource>;
} // namespace b3d::render
