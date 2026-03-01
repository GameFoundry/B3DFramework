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
		if(!B3D_CHECK_LOG(B3D_CURRENT_THREAD_ID != RenderThread::Instance().GetThreadId(), "You cannot call this method on the render thread. It will cause a deadlock."))
			return;

		GetRenderThread().PostCommand([] {}, "RenderProxy::BlockUntilInitialized", true);

		Lock lock(mRenderProxyInitializedMutex);
		if(!B3D_CHECK_LOG(IsInitialized() || mFlags.IsSet(RenderProxyFlag::ScheduledForInitialization), "Attempting to wait until initialization finishes but object is not scheduled to be initialized."))
			return;

		mRenderProxyInitializedCondition.Wait(lock, [this] { return IsInitialized(); });
	}
}

void RenderProxy::SetShared(SPtr<RenderProxy> sharedToThis)
{
	mThis = sharedToThis;
}
}}
