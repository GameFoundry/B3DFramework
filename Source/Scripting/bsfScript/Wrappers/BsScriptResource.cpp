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

	MonoObject* ScriptResourceBase::getManagedInstance() const
	{
		return MonoUtil::GetObjectFromGCHandle(mGCHandle);
	}

	MonoObject* ScriptResourceBase::getRRef(const HResource& resource, UINT32 rttiId)
	{
		::MonoClass* rrefClass = getRRefClass(rttiId);
		if(!rrefClass)
			return nullptr;

		ScriptRRefBase* rref = ScriptResourceManager::instance().GetScriptRRef(resource, rrefClass);
		if(!rref)
			return nullptr;

		return rref->GetManagedInstance();
	}

	void ScriptResourceBase::SetManagedInstance(::MonoObject* instance)
	{
		BS_ASSERT(mGCHandle == 0 && "Attempting to set a new managed instance without freeing the old one.");

		mGCHandle = MonoUtil::newGCHandle(instance, false);
	}

	void ScriptResourceBase::FreeManagedInstance()
	{
		if (mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	void ScriptResourceBase::Destroy()
	{
		ScriptResourceManager::instance().DestroyScriptResource(this);
	}

	::MonoClass* ScriptResourceBase::getManagedResourceClass(UINT32 rttiId)
	{
		if(rttiId == Resource::getRTTIStatic()->GetRTTIId())
			return ScriptResource::GetMetaData()->scriptClass->_getInternalClass();
		else if(rttiId == ManagedResource::getRTTIStatic()->GetRTTIId())
			return ScriptResource::GetMetaData()->scriptClass->_getInternalClass();
		else
		{
			BuiltinResourceInfo* info = ScriptAssemblyManager::instance().GetBuiltinResourceInfo(rttiId);

			if (info == nullptr)
				return nullptr;

			return info->monoClass->_getInternalClass();
		}
	}

	::MonoClass* ScriptResourceBase::getRRefClass(UINT32 rttiId)
	{
		::MonoClass* monoClass = getManagedResourceClass(rttiId);
		if (!monoClass)
			return nullptr;

		return ScriptRRefBase::BindGenericParam(monoClass);
	}

	void ScriptResource::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptResource::internal_getName);
		metaData.scriptClass->AddInternalCall("Internal_GetUUID", (void*)&ScriptResource::internal_getUUID);
		metaData.scriptClass->AddInternalCall("Internal_Release", (void*)&ScriptResource::internal_release);
	}

	MonoString* ScriptResource::internal_getName(ScriptResourceBase* nativeInstance)
	{
		return MonoUtil::StringToMono(nativeInstance->GetGenericHandle()->getName());
	}

	void ScriptResource::internal_getUUID(ScriptResourceBase* nativeInstance, UUID* uuid)
	{
		*uuid = nativeInstance->GetGenericHandle().GetUUID();
	}

	void ScriptResource::internal_release(ScriptResourceBase* nativeInstance)
	{
		nativeInstance->GetGenericHandle().Release();
	}

	ScriptUUID::ScriptUUID(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptUUID::InitRuntimeData()
	{ }

	MonoObject* ScriptUUID::box(const UUID& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	UUID ScriptUUID::Unbox(MonoObject* obj)
	{
		return *(UUID*)MonoUtil::unbox(obj);
	}
}
