//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptResource.h"
#include "BsScriptResourceManager.h"
#include "Resources/BsResource.h"
#include "BsMonoUtil.h"
#include <assert.h>

namespace bs
{
	ScriptResourceBase::ScriptResourceBase(MonoObject* instance)
		:PersistentScriptObjectBase(instance), mGCHandle(0)
	{ }

	ScriptResourceBase::~ScriptResourceBase()
	{
		BS_ASSERT(mGCHandle == 0 && "Object being destroyed without its managed instance being freed first.");
	}

	MonoObject* ScriptResourceBase::getManagedInstance() const
	{
		return MonoUtil::getObjectFromGCHandle(mGCHandle);
	}

	void ScriptResourceBase::setManagedInstance(::MonoObject* instance)
	{
		BS_ASSERT(mGCHandle == 0 && "Attempting to set a new managed instance without freeing the old one.");

		mGCHandle = MonoUtil::newGCHandle(instance, false);
	}

	void ScriptResourceBase::freeManagedInstance()
	{
		if (mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	void ScriptResourceBase::destroy()
	{
		ScriptResourceManager::instance().destroyScriptResource(this);
	}

	void ScriptResource::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetName", (void*)&ScriptResource::internal_getName);
		metaData.scriptClass->addInternalCall("Internal_GetUUID", (void*)&ScriptResource::internal_getUUID);
		metaData.scriptClass->addInternalCall("Internal_Release", (void*)&ScriptResource::internal_release);
	}

	MonoString* ScriptResource::internal_getName(ScriptResourceBase* nativeInstance)
	{
		return MonoUtil::wstringToMono(nativeInstance->getGenericHandle()->getName());
	}

	void ScriptResource::internal_getUUID(ScriptResourceBase* nativeInstance, UUID* uuid)
	{
		*uuid = nativeInstance->getGenericHandle().getUUID();
	}

	void ScriptResource::internal_release(ScriptResourceBase* nativeInstance)
	{
		nativeInstance->getGenericHandle().release();
	}

	ScriptUUID::ScriptUUID(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptUUID::initRuntimeData()
	{ }

	MonoObject* ScriptUUID::box(const UUID& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	UUID ScriptUUID::unbox(MonoObject* obj)
	{
		return *(UUID*)MonoUtil::unbox(obj);
	}
}
