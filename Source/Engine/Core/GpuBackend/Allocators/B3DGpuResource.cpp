//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuResourceManager.h"

namespace b3d
{
	IGpuResource::IGpuResource(GpuResourceManager* owner, const StringView& name)
		: mOwner(owner)
	{
		B3D_ASSERT(owner != nullptr && "Manager-owned IGpuResource must be constructed with a non-null GpuResourceManager.");
#if B3D_BUILD_TYPE_DEVELOPMENT
		mDebugName = name;
#else
		(void)name;
#endif
	}

	IGpuResource::~IGpuResource()
	{
		B3D_ASSERT((mOwner == nullptr || mDestroyRequested) && "Manager-owned IGpuResource destructed without Destroy() being called first.");
	}

	void IGpuResource::NotifyBound()
	{
		Lock lock(mMutex);
		B3D_ASSERT(!mDestroyRequested);
		mBountCount++;
	}

	void IGpuResource::NotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags)
	{
		Lock lock(mMutex);
		B3D_ASSERT(useFlags != GpuAccessFlag::None);
		B3D_ASSERT(queueId.Id < kMaximumUniqueQueueCount);
		mUsedCount++;

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

		OnNotifyUsed(queueId, useFlags);
	}

	void IGpuResource::NotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags)
	{
		bool destroyImmediately;
		{
			Lock lock(mMutex);
			B3D_ASSERT(mUsedCount > 0);
			B3D_ASSERT(mBountCount > 0);

			mUsedCount--;
			mBountCount--;

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

			OnNotifyDone(queueId, useFlags);

			destroyImmediately = mBountCount == 0 && mDestroyRequested;
		}

		// Safe to check outside of mutex — once queued for destruction, mDestroyRequested cannot be cleared.
		if(destroyImmediately)
			DestroyImmediately();
	}

	GpuQueueMask IGpuResource::GetUseInfo(GpuAccessFlags useFlags) const
	{
		GpuQueueMask mask = GpuQueueMask::kNone;
		Lock lock(mMutex);

		for(u32 queueIndex = 0; queueIndex < kMaximumUniqueQueueCount; queueIndex++)
		{
			if(useFlags.IsSet(GpuAccessFlag::Read) && mReadUses[queueIndex] > 0)
				mask |= GpuQueueId(queueIndex);

			if(useFlags.IsSet(GpuAccessFlag::Write) && mWriteUses[queueIndex] > 0)
				mask |= GpuQueueId(queueIndex);
		}

		return mask;
	}

	void IGpuResource::NotifyUnbound()
	{
		bool destroyImmediately;
		{
			Lock lock(mMutex);
			B3D_ASSERT(mBountCount > 0);

			mBountCount--;

			destroyImmediately = mBountCount == 0 && mDestroyRequested;
		}

		if(destroyImmediately)
			DestroyImmediately();
	}

	void IGpuResource::Destroy()
	{
		B3D_ASSERT(mOwner != nullptr && "Destroy() called on an IGpuResource that has no owning GpuResourceManager.");

		bool destroy;
		{
			Lock lock(mMutex);
			B3D_ASSERT(!mDestroyRequested && "IGpuResource::Destroy() called more than once.");

			mDestroyRequested = true;
			destroy = mBountCount == 0;
		}

		if(destroy)
			DestroyImmediately();
	}

	void IGpuResource::DestroyImmediately()
	{
		OnWillDestroy();
		mOwner->Destroy(this);
	}

	IGpuImageResource::IGpuImageResource(GpuResourceManager* owner, const StringView& name, u32 faceCount, u32 mipLevelCount, GpuTextureAspectFlags aspectMask)
		: IGpuResource(owner, name), mFaceCount(faceCount), mMipLevelCount(mipLevelCount), mFullRange(0, mipLevelCount, 0, faceCount, aspectMask)
	{
		B3D_ASSERT(aspectMask);

		const u32 subresourceCount = GetSubresourceCount();
		mSubresources = (IGpuResource**)B3DAllocate(sizeof(IGpuResource*) * subresourceCount);
		for(u32 subresourceIndex = 0; subresourceIndex < subresourceCount; subresourceIndex++)
			mSubresources[subresourceIndex] = nullptr;
	}

	IGpuImageResource::~IGpuImageResource()
	{
		if(mSubresources != nullptr)
		{
			B3DFree(mSubresources);
			mSubresources = nullptr;
		}
	}

	u32 IGpuImageResource::GetSubresourceIndex(u32 face, u32 mipLevel, GpuTextureAspectFlag aspect) const
	{
		B3D_ASSERT(face < mFaceCount && mipLevel < mMipLevelCount);
		B3D_ASSERT(mFullRange.AspectMask.IsSet(aspect));

		u32 aspectIndex = 0;
		for(GpuTextureAspectFlag availableAspect : kGpuTextureAspects)
		{
			if(availableAspect == aspect)
				break;

			if(mFullRange.AspectMask.IsSet(availableAspect))
				aspectIndex++;
		}

		return aspectIndex * mFaceCount * mMipLevelCount + mipLevel * mFaceCount + face;
	}

	IGpuResource* IGpuImageResource::GetSubresource(u32 face, u32 mipLevel, GpuTextureAspectFlag aspect) const
	{
		return mSubresources[GetSubresourceIndex(face, mipLevel, aspect)];
	}

	GpuQueueMask IGpuImageResource::GetSubresourceUseInfo(u32 face, u32 mipLevel, GpuAccessFlags useFlags) const
	{
		GpuQueueMask useMask = GpuQueueMask::kNone;
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(!mFullRange.AspectMask.IsSet(aspect))
				continue;

			useMask |= GetSubresource(face, mipLevel, aspect)->GetUseInfo(useFlags);
		}

		return useMask;
	}

	u32 IGpuImageResource::GetSubresourceBoundCount(u32 face, u32 mipLevel) const
	{
		u32 boundCount = 0;
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(mFullRange.AspectMask.IsSet(aspect))
				boundCount += GetSubresource(face, mipLevel, aspect)->GetBoundCount();
		}

		return boundCount;
	}

	u32 IGpuImageResource::GetSubresourceUseCount(u32 face, u32 mipLevel) const
	{
		u32 useCount = 0;
		for(GpuTextureAspectFlag aspect : kGpuTextureAspects)
		{
			if(mFullRange.AspectMask.IsSet(aspect))
				useCount += GetSubresource(face, mipLevel, aspect)->GetUseCount();
		}

		return useCount;
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	void IGpuBufferResource::InitializeSuballocationTracking(u32 suballocationCount, u32 suballocationSize)
	{
		Lock lock(mMutex);

		mSuballocationSize = suballocationSize;

		// Only track if there are multiple suballocations (single suballocation falls back to whole-buffer tracking)
		if(suballocationCount > 1)
		{
			mSuballocationStates.Clear();
			for(u32 subIndex = 0; subIndex < suballocationCount; ++subIndex)
				mSuballocationStates.Add(SuballocationTrackingState{});
		}
	}

	void IGpuBufferResource::NotifySuballocationBound(u32 suballocationIndex)
	{
		Lock lock(mMutex);
		if(mSuballocationStates.Empty())
			return; // Single suballocation, use existing whole-buffer tracking

		B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
		mSuballocationStates[suballocationIndex].BoundCount++;
	}

	void IGpuBufferResource::NotifySuballocationUsed(u32 suballocationIndex)
	{
		Lock lock(mMutex);
		if(mSuballocationStates.Empty())
			return;

		B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
		mSuballocationStates[suballocationIndex].UseCount++;
	}

	void IGpuBufferResource::NotifySuballocationDone(u32 suballocationIndex)
	{
		Lock lock(mMutex);
		if(mSuballocationStates.Empty())
			return;

		B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
		B3D_ASSERT(mSuballocationStates[suballocationIndex].UseCount > 0);
		B3D_ASSERT(mSuballocationStates[suballocationIndex].BoundCount > 0);
		mSuballocationStates[suballocationIndex].UseCount--;
		mSuballocationStates[suballocationIndex].BoundCount--;
	}

	void IGpuBufferResource::NotifySuballocationUnbound(u32 suballocationIndex)
	{
		Lock lock(mMutex);
		if(mSuballocationStates.Empty())
			return;

		B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
		B3D_ASSERT(mSuballocationStates[suballocationIndex].BoundCount > 0);
		mSuballocationStates[suballocationIndex].BoundCount--;
	}

	bool IGpuBufferResource::IsSuballocationBound(u32 suballocationIndex) const
	{
		Lock lock(mMutex);
		if(mSuballocationStates.Empty())
			return mBountCount > 0; // Fall back to whole-buffer tracking

		B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
		return mSuballocationStates[suballocationIndex].BoundCount > 0;
	}

	bool IGpuBufferResource::IsSuballocationInUse(u32 suballocationIndex) const
	{
		Lock lock(mMutex);
		if(mSuballocationStates.Empty())
			return mUsedCount > 0; // Fall back to whole-buffer tracking

		B3D_ASSERT(suballocationIndex < mSuballocationStates.Size());
		return mSuballocationStates[suballocationIndex].UseCount > 0;
	}

	u32 IGpuBufferResource::GetSuballocationIndexForOffset(u32 offset) const
	{
		if(mSuballocationSize == 0)
			return 0;

		return offset / mSuballocationSize;
	}

	bool IGpuBufferResource::IsRangeBound(u32 offset, u32 size) const
	{
		Lock lock(mMutex);
		if(mSuballocationStates.Empty())
			return mBountCount > 0; // Fall back to whole-buffer tracking

		const u32 firstSuballoc = GetSuballocationIndexForOffset(offset);
		const u32 lastSuballoc = GetSuballocationIndexForOffset(offset + size - 1);

		for(u32 subIndex = firstSuballoc; subIndex <= lastSuballoc && subIndex < mSuballocationStates.Size(); ++subIndex)
		{
			if(mSuballocationStates[subIndex].BoundCount > 0)
				return true;
		}
		return false;
	}

	bool IGpuBufferResource::IsRangeInUse(u32 offset, u32 size) const
	{
		Lock lock(mMutex);
		if(mSuballocationStates.Empty())
			return mUsedCount > 0; // Fall back to whole-buffer tracking

		const u32 firstSuballoc = GetSuballocationIndexForOffset(offset);
		const u32 lastSuballoc = GetSuballocationIndexForOffset(offset + size - 1);

		for(u32 subIndex = firstSuballoc; subIndex <= lastSuballoc && subIndex < mSuballocationStates.Size(); ++subIndex)
		{
			if(mSuballocationStates[subIndex].UseCount > 0)
				return true;
		}
		return false;
	}
#endif // B3D_BUILD_TYPE_DEVELOPMENT
} // namespace b3d
