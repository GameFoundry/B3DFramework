//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptObjectWrapper.h"

#include "B3DMonoUtil.h"
#include "B3DScriptObjectManager.h"
#include "Script/B3DIScriptExportable.h"

using namespace b3d;

ScriptObjectWrapper::ScriptObjectWrapper(IScriptExportable* nativeObject)
	:IScriptObjectWrapper(nativeObject)
{
	ScriptObjectManager::Instance().RegisterScriptObjectWrapper(this);
}

ScriptObjectWrapper::~ScriptObjectWrapper()
{
	ScriptObjectManager::Instance().UnregisterScriptObjectWrapper(this);
}

MonoObject* ScriptObjectWrapper::GetScriptObject() const
{
	if(mScriptObjectHandle == ~0u)
		return nullptr;

	return MonoUtil::GetObjectFromGcHandle(mScriptObjectHandle);
}

void ScriptObjectWrapper::NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload)
{
	 // Don't kill the wrapper if we're persisting script reload, as we'll just bind a new script object on the wrapper after reload finishes
	if(ShouldPersistScriptReload() && isDestroyedDueToScriptReload)
	{
		// Handle should have been cleared already by the caller
		B3D_ENSURE(mScriptObjectHandle == ~0u);
		return;
	}

	IScriptObjectWrapper::NotifyScriptObjectDestroyed(isDestroyedDueToScriptReload);
	B3DDelete(this);
}

void ScriptObjectWrapper::NotifyNativeObjectDestroyed()
{
	ReleaseScriptObjectHandle();

	IScriptObjectWrapper::NotifyNativeObjectDestroyed();
}

bool ScriptObjectWrapper::ShouldPersistScriptReload() const
{
	if(mNativeObject != nullptr)
		return mNativeObject->ShouldPersistScriptReload();

	return false;
}

void ScriptObjectWrapper::NotifyScriptWillReload()
{
	if(mNativeObject != nullptr)
		mNativeObject->NotifyScriptWillReload();
}

TOptional<ScriptObjectReloadPersistentData> ScriptObjectWrapper::BackupDataBeforeScriptReload()
{
	if(mNativeObject != nullptr)
		return mNativeObject->BackupDataBeforeScriptReload();

	return {};
}

void ScriptObjectWrapper::RecreateScriptObjectAfterScriptReload()
{
	if(mNativeObject != nullptr)
		mNativeObject->RecreateScriptObjectAfterScriptReload();
}

void ScriptObjectWrapper::RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data)
{
	if(mNativeObject != nullptr)
		mNativeObject->RestoreDataAfterScriptReload(data);
}

void ScriptObjectWrapper::NotifyScriptReloadFinished()
{
	if(mNativeObject != nullptr)
		mNativeObject->NotifyScriptReloadFinished();
}

void ScriptObjectWrapper::CreateScriptObjectHandle(MonoObject* scriptObject)
{
	if(B3D_ENSURE(scriptObject != nullptr))
	{
		B3D_ENSURE(mScriptObjectHandle == ~0u);

		if(GetLifetimeTrackingMode() == ScriptObjectLifetimeTrackingMode::WeakHandle)
		{
			mScriptObjectHandle = MonoUtil::NewWeakGcHandle(scriptObject);
			mHoldsStrongScriptObjectHandle = false; // Not used in WeakHandle mode, but set it anyway
		}
		else
		{
			mScriptObjectHandle = MonoUtil::NewGcHandle(scriptObject, false);
			mHoldsStrongScriptObjectHandle = true; // Always start of as a strong handle, GC can transition to weak if needed
		}
	}
}

void ScriptObjectWrapper::TransitionToWeakScriptObjectHandle()
{
	if(!mHoldsStrongScriptObjectHandle)
		return;

	if(mScriptObjectHandle == ~0u)
		return;

	if(!B3D_ENSURE(GetLifetimeTrackingMode() == ScriptObjectLifetimeTrackingMode::StrongHandleWithGarbageCollection))
		return;

	MonoObject* const scriptObject = GetScriptObject();
	if(!B3D_ENSURE(scriptObject != nullptr))
		return;

	const u32 weakHandle = MonoUtil::NewWeakGcHandle(scriptObject);
	MonoUtil::FreeGcHandle(mScriptObjectHandle);

	mScriptObjectHandle = weakHandle;
	mHoldsStrongScriptObjectHandle = false;
}

void ScriptObjectWrapper::TransitionToStrongScriptObjectHandle()
{
	if(mHoldsStrongScriptObjectHandle)
		return;

	if(mScriptObjectHandle == ~0u)
		return;

	if(!B3D_ENSURE(GetLifetimeTrackingMode() == ScriptObjectLifetimeTrackingMode::StrongHandleWithGarbageCollection))
		return;

	MonoObject* const scriptObject = GetScriptObject();
	if(scriptObject == nullptr)
		return;

	const u32 strongHandle = MonoUtil::NewGcHandle(scriptObject, false);
	MonoUtil::FreeGcHandle(mScriptObjectHandle);

	mScriptObjectHandle = strongHandle;
	mHoldsStrongScriptObjectHandle = true;
}

void ScriptObjectWrapper::ReleaseScriptObjectHandle()
{
	if(mScriptObjectHandle != ~0u)
	{
		MonoUtil::FreeGcHandle(mScriptObjectHandle);
		mScriptObjectHandle = ~0u;
		mHoldsStrongScriptObjectHandle = false;
	}
}

ScriptScriptObject::ScriptScriptObject()
{}

void ScriptScriptObject::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_ScriptObjectFinalizerCalled", (void*)&ScriptScriptObject::Internal_ScriptObjectFinalizerCalled);
}

void ScriptScriptObject::Internal_ScriptObjectFinalizerCalled(ScriptObjectWrapper* scriptObjectWrapper)
{
	// This method gets called on the finalizer thread, but so that we don't need to deal
	// with multi-threading issues we just delay it and execute it on the main thread.
	ScriptObjectManager::Instance().NotifyObjectFinalized(scriptObjectWrapper);
}
