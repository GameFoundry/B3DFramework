//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptResource.h"
#include "BsScriptResourceManager.h"
#include "Resources/BsResource.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsManagedResource.h"
#include "Reflection/BsRTTIType.h"

namespace bs
{
	ScriptResourceBase::ScriptResourceBase(MonoObject* instance)
		:PersistentScriptObjectBase(instance)
	{ }

	ScriptResourceBase::~ScriptResourceBase()
	{
		BS_ASSERT(mGCHandle == 0 && "Object being destroyed without its managed instance being freed first.");
	}

	MonoObject* ScriptResourceBase::GetManagedInstance() const
	{
		return MonoUtil::GetObjectFromGcHandle(mGCHandle);
	}

	MonoObject* ScriptResourceBase::GetRRef(const HResource& resource, UINT32 rttiId)
	{
		::MonoClass* rrefClass = GetRRefClass(rttiId);
		if(!rrefClass)
			return nullptr;

		ScriptRRefBase* rref = ScriptResourceManager::Instance().GetScriptRRef(resource, rrefClass);
		if(!rref)
			return nullptr;

		return rref->GetManagedInstance();
	}

	void ScriptResourceBase::SetManagedInstance(::MonoObject* instance)
	{
		BS_ASSERT(mGCHandle == 0 && "Attempting to set a new managed instance without freeing the old one.");

		mGCHandle = MonoUtil::NewGcHandle(instance, false);
	}

	void ScriptResourceBase::freeManagedInstance()
	{
		if (mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	void ScriptResourceBase::Destroy()
	{
		ScriptResourceManager::Instance().destroyScriptResource(this);
	}

	::MonoClass* ScriptResourceBase::GetManagedResourceClass(UINT32 rttiId)
	{
		if(rttiId == Resource::GetRttiStatic()->GetRttiId())
			return ScriptResource::getMetaData()->scriptClass->GetInternalClassInternal();
		else if(rttiId == ManagedResource::GetRttiStatic()->GetRttiId())
			return ScriptResource::getMetaData()->scriptClass->GetInternalClassInternal();
		else
		{
			BuiltinResourceInfo* info = ScriptAssemblyManager::Instance().getBuiltinResourceInfo(rttiId);

			if (info == nullptr)
				return nullptr;

			return info->monoClass->GetInternalClassInternal();
		}
	}

	::MonoClass* ScriptResourceBase::GetRRefClass(UINT32 rttiId)
	{
		::MonoClass* monoClass = getManagedResourceClass(rttiId);
		if (!monoClass)
			return nullptr;

		return ScriptRRefBase::bindGenericParam(monoClass);
	}

	void ScriptResource::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetName", (void*)&ScriptResource::internal_getName);
		metaData.scriptClass->addInternalCall("Internal_GetUUID", (void*)&ScriptResource::internal_getUUID);
		metaData.scriptClass->addInternalCall("Internal_Release", (void*)&ScriptResource::internal_release);
	}

	MonoString* ScriptResource::InternalGetName(ScriptResourceBase* nativeInstance)
	{
		return MonoUtil::stringToMono(nativeInstance->GetGenericHandle()->GetName());
	}

	void ScriptResource::InternalGetUuid(ScriptResourceBase* nativeInstance, UUID* uuid)
	{
		*uuid = nativeInstance->GetGenericHandle().GetUuid();
	}

	void ScriptResource::InternalRelease(ScriptResourceBase* nativeInstance)
	{
		nativeInstance->GetGenericHandle().release();
	}

	ScriptUUID::ScriptUUID(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptUUID::initRuntimeData()
	{ }

	MonoObject* ScriptUUID::Box(const UUID& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->GetInternalClassInternal(), (void*)&value);
	}

	UUID ScriptUUID::Unbox(MonoObject* obj)
	{
		return *(UUID*)MonoUtil::unbox(obj);
	}
}
