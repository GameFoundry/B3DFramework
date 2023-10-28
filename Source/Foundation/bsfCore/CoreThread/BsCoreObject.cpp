//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreObject.h"
#include "CoreThread/BsCoreObjectCore.h"
#include "CoreThread/BsCoreThread.h"
#include "CoreThread/BsCoreObjectManager.h"

using namespace std::placeholders;

using namespace bs;

CoreObject::CoreObject(bool initializeOnCoreThread)
	: mFlags(initializeOnCoreThread ? CoreObjectFlag::RequiresRenderProxy : CoreObjectFlag::None)
	, mCoreDirtyFlags(0)
	, mInternalID(CoreObjectManager::Instance().GenerateId())
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

	mCoreSpecific = nullptr;
}

void CoreObject::Initialize()
{
	CoreObjectManager::Instance().RegisterObject(this);
	mCoreSpecific = CreateCore();

	if(mCoreSpecific != nullptr && !mCoreSpecific->IsInitialized())
	{
		if(mFlags.IsSet(CoreObjectFlag::RequiresRenderProxy))
		{
			mCoreSpecific->mFlags.Set(ct::RenderProxyFlag::ScheduledForInitialization);

			B3D_ASSERT(B3D_CURRENT_THREAD_ID != CoreThread::Instance().GetCoreThreadId() && "Cannot initialize sim thread object from core thread.");

			CoreThread::Instance().PostCommand([object = mCoreSpecific] { object->Initialize(); });
		}
		else
		{
			mCoreSpecific->Initialize();
		}
	}

	mFlags.Set(CoreObjectFlag::Initialized);
	MarkDependenciesDirty();
}

void CoreObject::BlockUntilCoreInitialized() const
{
	if(mCoreSpecific != nullptr)
		mCoreSpecific->Synchronize();
}

void CoreObject::SyncToCore()
{
	CoreObjectManager::Instance().SyncToCore(this);
}

void CoreObject::MarkCoreDirty(u32 flags)
{
	bool wasDirty = IsCoreDirty();

	mCoreDirtyFlags |= flags;

	if(!wasDirty && IsCoreDirty())
		CoreObjectManager::Instance().NotifyCoreDirty(this);
}

void CoreObject::MarkDependenciesDirty()
{
	CoreObjectManager::Instance().NotifyDependenciesDirty(this);
}

void CoreObject::SetShared(SPtr<CoreObject> ptrThis)
{
	mThis = ptrThis;
}
