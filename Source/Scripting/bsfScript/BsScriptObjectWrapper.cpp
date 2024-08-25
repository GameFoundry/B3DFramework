//********************************* bs::framework - Copyright 2024 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptObjectWrapper.h"

#include "BsMonoUtil.h"
#include "BsScriptObjectManager.h"
#include "Script/BsIScriptExportable.h"

using namespace bs;

ScriptObjectWrapper::ScriptObjectWrapper(IScriptExportable* nativeObject, MonoObject* scriptObject)
	:IScriptObjectWrapper(nativeObject)
{
	ScriptObjectManager::Instance().RegisterScriptObjectWrapper(this);

	CreateStrongScriptObjectHandle(scriptObject);
}

ScriptObjectWrapper::~ScriptObjectWrapper()
{
	ScriptObjectManager::Instance().UnregisterScriptObjectWrapper(this);
}

MonoObject* ScriptObjectWrapper::GetScriptObject() const
{
	if(mStrongScriptObjectHandle == ~0u)
		return nullptr;

	return MonoUtil::GetObjectFromGcHandle(mStrongScriptObjectHandle);
}

void ScriptObjectWrapper::NotifyScriptObjectDestroyed(bool isDestroyedDueToScriptReload)
{
	IScriptObjectWrapper::NotifyScriptObjectDestroyed();
	B3DDelete(this);
}

void ScriptObjectWrapper::NotifyNativeObjectDestroyed()
{
	ReleaseStrongScriptObjectHandle();

	IScriptObjectWrapper::NotifyNativeObjectDestroyed();
}

void ScriptObjectWrapper::CreateStrongScriptObjectHandle(MonoObject* scriptObject)
{
	if(B3D_ENSURE(scriptObject != nullptr))
	{
		B3D_ENSURE(mStrongScriptObjectHandle == ~0u);
		mStrongScriptObjectHandle = MonoUtil::NewGcHandle(scriptObject, false);
	}
}

void ScriptObjectWrapper::ReleaseStrongScriptObjectHandle()
{
	if(mStrongScriptObjectHandle != ~0u)
	{
		MonoUtil::FreeGcHandle(mStrongScriptObjectHandle);
		mStrongScriptObjectHandle = ~0u;
	}
}


ScriptScriptObject::ScriptScriptObject(MonoObject* scriptObject)
	: TNonInstantiableScriptObjectWrapper(scriptObject)
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
