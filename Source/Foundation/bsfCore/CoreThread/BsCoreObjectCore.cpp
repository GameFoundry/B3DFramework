//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreThread/BsCoreObjectCore.h"
#include "CoreThread/BsCoreThread.h"

using namespace bs;

namespace bs { namespace ct
{
Signal CoreObject::mCoreGpuObjectLoadedCondition;
Mutex CoreObject::mCoreGpuObjectLoadedMutex;

CoreObject::CoreObject()
	: mFlags(RenderProxyFlag::None)
{}

CoreObject::~CoreObject()
{
	EnsureRenderThread();

	B3D_ENSURE(IsDestroyed());
	B3D_ENSURE(mThis.expired());
}

void CoreObject::Initialize()
{
	{
		Lock lock(mCoreGpuObjectLoadedMutex);
		mFlags.Set(RenderProxyFlag::Initialized);
	}

	mFlags.Unset(RenderProxyFlag::ScheduledForInitialization);
	mCoreGpuObjectLoadedCondition.NotifyAll();
}

void CoreObject::Destroy()
{
	if(!B3D_ENSURE(!IsDestroyed()))
		return;

	mFlags.Set(RenderProxyFlag::Destroyed);
}

void CoreObject::Synchronize()
{
	if(!IsInitialized())
	{
#if B3D_DEBUG
		if(B3D_CURRENT_THREAD_ID == CoreThread::Instance().GetCoreThreadId())
			B3D_EXCEPT(InternalErrorException, "You cannot call this method on the core thread. It will cause a deadlock!");
#endif

		GetCoreThread().PostCommand([] {}, true);

		Lock lock(mCoreGpuObjectLoadedMutex);
		if(!IsInitialized() && !mFlags.IsSet(RenderProxyFlag::ScheduledForInitialization))
			B3D_EXCEPT(InternalErrorException, "Attempting to wait until initialization finishes but object is not scheduled to be initialized.");

		mCoreGpuObjectLoadedCondition.Wait(lock, [this] { return IsInitialized(); });
	}
}

void CoreObject::SetShared(SPtr<CoreObject> sharedToThis)
{
	mThis = sharedToThis;
}
}}
