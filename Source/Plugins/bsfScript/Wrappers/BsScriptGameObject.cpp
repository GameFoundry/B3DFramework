//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptGameObject.h"
#include "BsMonoUtil.h"
#include <assert.h>

namespace bs
{
	ScriptGameObjectBase::ScriptGameObjectBase(MonoObject* instance)
		:PersistentScriptObjectBase(instance), mGCHandle(0)
	{
		
	}

	ScriptGameObjectBase::~ScriptGameObjectBase()
	{
		BS_ASSERT(mGCHandle == 0 && "Object being destroyed without its managed instance being freed first.");
	}
	
	MonoObject* ScriptGameObjectBase::getManagedInstance() const
	{
		return MonoUtil::getObjectFromGCHandle(mGCHandle);
	}

	void ScriptGameObjectBase::setManagedInstance(::MonoObject* instance)
	{
		BS_ASSERT(mGCHandle == 0 && "Attempting to set a new managed instance without freeing the old one.");

		mGCHandle = MonoUtil::newGCHandle(instance, false);
	}

	void ScriptGameObjectBase::freeManagedInstance()
	{
		if (mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	ScriptGameObject::ScriptGameObject(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptGameObject::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetInstanceId", (void*)&ScriptGameObject::internal_getInstanceId);
	}

	UINT64 ScriptGameObject::internal_getInstanceId(ScriptGameObject* nativeInstance)
	{
		return nativeInstance->getNativeHandle().getInstanceId();
	}
}
