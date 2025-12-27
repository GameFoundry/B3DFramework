//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreObject/B3DCoreObject.h"
#include "CoreObject/B3DRenderProxy.h"
#include "CoreObject/B3DRenderThread.h"
#include "CoreObject/B3DCoreObjectManager.h"

using namespace b3d;

CoreObject::CoreObject(bool createRenderProxy)
	: mFlags(createRenderProxy ? CoreObjectFlag::RequiresRenderProxy : CoreObjectFlag::None)
	, mRenderProxyDirtyFlags(0)
	, mInternalID(CoreObjectManager::GenerateId())
{
}

CoreObject::~CoreObject()
{
	if(!IsDestroyed())
	{
		// Object must be released with Destroy() otherwise engine can still try to use it, even if it was destructed
		// (e.g. if an object has one of its methods queued in a command queue, and is destructed, you will be accessing invalid memory)
		B3D_EXCEPT(InternalErrorException, "Destructor called but object is not destroyed. This will result in nasty issues.");
	}

#if B3D_DEBUG
	if(!mThis.expired())
	{
		B3D_EXCEPT(InternalErrorException, "Shared pointer to this object still has active references but "
										  "the object is being deleted? You shouldn't delete CoreObjects manually.");
	}
#endif
}

void CoreObject::Destroy()
{
	CoreObjectManager::Instance().UnregisterObject(this);
	mFlags.Set(CoreObjectFlag::Destroyed);

	mRenderProxy = nullptr;
}

void CoreObject::Initialize()
{
	CoreObjectManager::Instance().RegisterObject(this);
	mRenderProxy = CreateRenderProxy();

	if(mRenderProxy != nullptr && !mRenderProxy->IsInitialized())
	{
		if(mFlags.IsSet(CoreObjectFlag::RequiresRenderProxy))
		{
			mRenderProxy->mFlags.Set(render::RenderProxyFlag::ScheduledForInitialization);

			B3D_ENSURE_LOG(B3D_CURRENT_THREAD_ID != GetRenderThread().GetThreadId(), "Cannot initialize CoreObject object from the render thread.");

			GetRenderThread().PostCommand([object = mRenderProxy]
										{ object->Initialize(); }, "RenderProxy::Initialize");
		}
		else
		{
			mRenderProxy->Initialize();
		}
	}

	mFlags.Set(CoreObjectFlag::Initialized);
	MarkDependenciesDirty();
}

void CoreObject::BlockUntilRenderProxyInitialized() const
{
	if(mRenderProxy != nullptr)
		mRenderProxy->BlockUntilInitialized();
}

void CoreObject::SyncToRenderProxy()
{
	CoreObjectManager::Instance().SyncToRenderThread(this);
}

void CoreObject::MarkRenderProxyDataDirty(u32 flags)
{
	bool wasDirty = IsRenderProxyDataOutOfDate();

	mRenderProxyDirtyFlags |= flags;

	if(!wasDirty && IsRenderProxyDataOutOfDate())
		CoreObjectManager::Instance().NotifyRenderProxyDirty(this);
}

void CoreObject::MarkDependenciesDirty()
{
	CoreObjectManager::Instance().NotifyDependenciesDirty(this);
}

void CoreObject::SetShared(SPtr<CoreObject> ptrThis)
{
	mThis = ptrThis;
}
