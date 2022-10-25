//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptGameObject.h"
#include "BsMonoUtil.h"
#include <assert.h>

namespace bs
{
	bool checkIfDestroyed(ScriptGameObject* nativeInstance)
	{
		HGameObject go = nativeInstance->GetNativeHandle();
		if(go.IsDestroyed())
		{
			BS_LOG(Warning, Scene, "Trying to access a destroyed GameObject with instance ID: {0}", go.GetInstanceId());
			return true;
		}

		return false;
	}

	ScriptGameObjectBase::ScriptGameObjectBase(MonoObject* instance)
		: PersistentScriptObjectBase(instance)
	{
	}

	ScriptGameObjectBase::~ScriptGameObjectBase()
	{
		BS_ASSERT(mGCHandle == 0 && "Object being destroyed without its managed instance being freed first.");
	}

	MonoObject* ScriptGameObjectBase::GetManagedInstance() const
	{
		return MonoUtil::GetObjectFromGcHandle(mGCHandle);
	}

	void ScriptGameObjectBase::SetManagedInstance(::MonoObject* instance)
	{
		BS_ASSERT(mGCHandle == 0 && "Attempting to set a new managed instance without freeing the old one.");

		mGCHandle = MonoUtil::NewGcHandle(instance, false);
	}

	void ScriptGameObjectBase::FreeManagedInstance()
	{
		if(mGCHandle != 0)
		{
			MonoUtil::FreeGcHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	ScriptGameObject::ScriptGameObject(MonoObject* instance)
		: ScriptObject(instance)
	{}

	void ScriptGameObject::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetInstanceId", (void*)&ScriptGameObject::InternalGetInstanceId);
		metaData.ScriptClass->AddInternalCall("Internal_GetUUID", (void*)&ScriptGameObject::InternalGetUuid);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsDestroyed", (void*)&ScriptGameObject::InternalGetIsDestroyed);
	}

	u64 ScriptGameObject::InternalGetInstanceId(ScriptGameObject* nativeInstance)
	{
		return nativeInstance->GetNativeHandle().GetInstanceId();
	}

	void ScriptGameObject::InternalGetUuid(ScriptGameObject* nativeInstance, UUID* uuid)
	{
		if(checkIfDestroyed(nativeInstance))
		{
			*uuid = UUID::EMPTY;
			return;
		}

		*uuid = nativeInstance->GetNativeHandle()->GetUuid();
	}

	bool ScriptGameObject::InternalGetIsDestroyed(ScriptGameObject* nativeInstance)
	{
		return nativeInstance->GetNativeHandle().IsDestroyed(true);
	}
} // namespace bs
