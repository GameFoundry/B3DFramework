//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptObject.h"
#include "BsScriptObjectManager.h"
#include "BsMonoManager.h"
#include "Error/BsCrashHandler.h"
#include "BsMonoField.h"

using namespace bs;
ScriptObjectBase::ScriptObjectBase(MonoObject* instance)
{
	ScriptObjectManager::Instance().RegisterScriptObject(this);
}

ScriptObjectBase::~ScriptObjectBase()
{
	ScriptObjectManager::Instance().UnregisterScriptObject(this);
}

ScriptObjectBackup ScriptObjectBase::BeginRefresh()
{
	return ScriptObjectBackup();
}

void ScriptObjectBase::EndRefresh(const ScriptObjectBackup& data)
{
}

void ScriptObjectBase::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
{
	B3DDelete(this);
}

PersistentScriptObjectBase::PersistentScriptObjectBase(MonoObject* instance)
	: ScriptObjectBase(instance)
{
}

ScriptObjectImpl::ScriptObjectImpl(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptObjectImpl::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_ManagedInstanceDeleted", (void*)&ScriptObjectImpl::InternalManagedInstanceDeleted);
}

void ScriptObjectImpl::InternalManagedInstanceDeleted(ScriptObjectBase* instance)
{
	// This method gets called on the finalizer thread, but so that we don't need to deal
	// with multi-threading issues we just delay it and execute it on the main thread.
	ScriptObjectManager::Instance().NotifyObjectFinalized(instance);
}
