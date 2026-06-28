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
		mUsedCount++;
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

			OnNotifyDone(queueId, useFlags);

			destroyImmediately = mBountCount == 0 && mDestroyRequested;
		}

		// Safe to check outside of mutex — once queued for destruction, mDestroyRequested cannot be cleared.
		if(destroyImmediately)
			DestroyImmediately();
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
		const u32 subresourceCount = faceCount * mipLevelCount;
		mSubresources = (IGpuResource**)B3DAllocate(sizeof(IGpuResource*) * subresourceCount);
		for(u32 i = 0; i < subresourceCount; i++)
			mSubresources[i] = nullptr;
	}

	IGpuImageResource::~IGpuImageResource()
	{
		if(mSubresources != nullptr)
		{
			B3DFree(mSubresources);
			mSubresources = nullptr;
		}
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
