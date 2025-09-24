//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "CoreObject/B3DRenderProxy.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

namespace b3d { namespace render
{
Signal RenderProxy::mRenderProxyInitializedCondition;
Mutex RenderProxy::mRenderProxyInitializedMutex;

RenderProxy::RenderProxy()
	: mFlags(RenderProxyFlag::None)
{}

RenderProxy::~RenderProxy()
{
	EnsureRenderThread();

	B3D_ENSURE(IsDestroyed());
	B3D_ENSURE(mThis.expired());
}

void RenderProxy::Initialize()
{
	{
		Lock lock(mRenderProxyInitializedMutex);
		mFlags.Set(RenderProxyFlag::Initialized);
	}

	mFlags.Unset(RenderProxyFlag::ScheduledForInitialization);
	mRenderProxyInitializedCondition.NotifyAll();
}

void RenderProxy::Destroy()
{
	if(!B3D_ENSURE(!IsDestroyed()))
		return;

	mFlags.Set(RenderProxyFlag::Destroyed);
}

void RenderProxy::BlockUntilInitialized()
{
	if(!IsInitialized())
	{
#if B3D_DEBUG
		if(B3D_CURRENT_THREAD_ID == RenderThread::Instance().GetThreadId())
			B3D_EXCEPT(InternalErrorException, "You cannot call this method on the render thread. It will cause a deadlock.");
#endif

		GetRenderThread().PostCommand([] {}, "RenderProxy::BlockUntilInitialized", true);

		Lock lock(mRenderProxyInitializedMutex);
		if(!IsInitialized() && !mFlags.IsSet(RenderProxyFlag::ScheduledForInitialization))
			B3D_EXCEPT(InternalErrorException, "Attempting to wait until initialization finishes but object is not scheduled to be initialized.");

		mRenderProxyInitializedCondition.Wait(lock, [this] { return IsInitialized(); });
	}
}

void RenderProxy::SetShared(SPtr<RenderProxy> sharedToThis)
{
	mThis = sharedToThis;
}
}}
