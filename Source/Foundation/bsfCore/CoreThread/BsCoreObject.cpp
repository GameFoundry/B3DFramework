//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreObject.h"
#include "CoreThread/BsCoreObjectCore.h"
#include "CoreThread/BsCoreThread.h"
#include "CoreThread/BsCoreObjectManager.h"

using namespace std::placeholders;

using namespace bs;

CoreObject::CoreObject(bool initializeOnCoreThread)
	: mFlags(initializeOnCoreThread ? CGO_INIT_ON_CORE_THREAD : 0)
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
	SetIsDestroyed(true);

	if(RequiresInitOnCoreThread())
	{
		B3D_ASSERT(B3D_CURRENT_THREAD_ID != CoreThread::Instance().GetCoreThreadId() && "Cannot destroy sim thead object from core thread.");

		// This will only destroy the ct::CoreObject if this was the last reference
		QueueDestroyGpuCommand(mCoreSpecific); // TODO - Need an explicit Destroy method here, otherwise clearing mCoreSpecific pointer below can still be the last reference if this code runs fast enough. Which means it gets destroyed on the wrong thread.
	}

	mCoreSpecific = nullptr;
}

void CoreObject::Initialize()
{
	CoreObjectManager::Instance().RegisterObject(this);
	mCoreSpecific = CreateCore();

	if(mCoreSpecific != nullptr && !mCoreSpecific->IsInitialized())
	{
		if(RequiresInitOnCoreThread())
		{
			mCoreSpecific->SetScheduledToBeInitialized(true);

			B3D_ASSERT(B3D_CURRENT_THREAD_ID != CoreThread::Instance().GetCoreThreadId() && "Cannot initialize sim thread object from core thread.");

			QueueInitializeGpuCommand(mCoreSpecific);
		}
		else
		{
			mCoreSpecific->Initialize();
		}
	}

	mFlags |= CGO_INITIALIZED;
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

void CoreObject::QueueGpuCommand(const SPtr<ct::CoreObject>& obj, std::function<void()> func)
{
	// We call another internal method and go through an additional layer of abstraction in order to keep an active
	// reference to the obj (saved in the bound function).
	// We could have called the function directly using "this" pointer but then we couldn't have used a shared_ptr for the object,
	// in which case there is a possibility that the object would be released and deleted while still being in the command queue.
	GetCoreThread().PostCommand(std::bind(&CoreObject::ExecuteGpuCommand, obj, func));
}

void CoreObject::QueueInitializeGpuCommand(const SPtr<ct::CoreObject>& obj)
{
	std::function<void()> func = std::bind(&ct::CoreObject::Initialize, obj.get());

	CoreThread::Instance().PostCommand(std::bind(&CoreObject::ExecuteGpuCommand, obj, func));
}

void CoreObject::QueueDestroyGpuCommand(const SPtr<ct::CoreObject>& obj)
{
	std::function<void()> func = [&]() {}; // Do nothing function. We just need the shared pointer to stay alive until it reaches the core thread

	GetCoreThread().PostCommand(std::bind(&CoreObject::ExecuteGpuCommand, obj, func));
}

void CoreObject::ExecuteGpuCommand(const SPtr<ct::CoreObject>& obj, std::function<void()> func)
{
	volatile SPtr<ct::CoreObject> objParam = obj; // Makes sure obj isn't optimized out?

	func();
}

void CoreObject::ExecuteReturnGpuCommand(const SPtr<ct::CoreObject>& obj, std::function<void(AsyncOp&)> func, AsyncOp& op)
{
	volatile SPtr<ct::CoreObject> objParam = obj; // Makes sure obj isn't optimized out?

	func(op);
}
