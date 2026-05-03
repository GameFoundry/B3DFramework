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
} // namespace b3d
