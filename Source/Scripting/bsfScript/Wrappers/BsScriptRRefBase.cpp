//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRRefBase.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Resources/BsResources.h"
#include "Wrappers/BsScriptResource.h"
#include "BsScriptResourceManager.h"
#include "BsApplication.h"
#include "Serialization/BsScriptAssemblyManager.h"

namespace bs
{
	ScriptRRefBase::ScriptRRefBase(MonoObject* instance, ResourceHandle<Resource> resource)
		:ScriptObject(instance), mResource(std::move(resource)), mGCHandle(MonoUtil::NewGcHandle(instance))
	{ }

	ScriptRRefBase::~ScriptRRefBase()
	{
		BS_ASSERT(mGCHandle == 0 && "Object being destroyed without its managed instance being freed first.");
	}

	void ScriptRRefBase::initRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_IsLoaded", (void*)&ScriptRRefBase::InternalIsLoaded);
		metaData.scriptClass->AddInternalCall("Internal_GetResource", (void*)&ScriptRRefBase::InternalGetResource);
		metaData.scriptClass->AddInternalCall("Internal_GetUUID", (void*)&ScriptRRefBase::InternalGetUuid);
		metaData.scriptClass->AddInternalCall("Internal_CastAs", (void*)&ScriptRRefBase::internal_CastAs);
	}

	ScriptRRefBase* ScriptRRefBase::CreateInternal(const ResourceHandle<Resource>& handle, ::MonoClass* rawType)
	{
		MonoClass* type = nullptr;
		if(rawType == nullptr)
			type = metaData.scriptClass;
		else
		{
			type = MonoManager::Instance().findClass(rawType);
			if (type == nullptr)
				type = metaData.scriptClass;
			else
			{
				assert(type->isSubClassOf(metaData.scriptClass));
			}
		}

		MonoObject* obj = type->createInstance();
		ScriptRRefBase* output = new (bs_alloc<ScriptRRefBase>()) ScriptRRefBase(obj, handle);

		// Note: It's important this method never returns null, handles should always be created to avoid extensive null
		// checks
		return output;
	}

	MonoObject* ScriptRRefBase::GetManagedInstance() const
	{
		return MonoUtil::getObjectFromGCHandle(mGCHandle);
	}

	void ScriptRRefBase::ClearManagedInstanceInternal()
	{
		if (mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	void ScriptRRefBase::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
	{
		if (mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}

		ScriptObjectBase::OnManagedInstanceDeletedInternal(assemblyRefresh);
	}

	::MonoClass* ScriptRRefBase::BindGenericParam(::MonoClass* param)
	{
		MonoClass* rrefClass = ScriptAssemblyManager::Instance().getBuiltinClasses().genericRRefClass;

		::MonoClass* params[1] = { param };
		return MonoUtil::bindGenericParameters(rrefClass->GetInternalClassInternal(), params, 1);
	}

	bool ScriptRRefBase::InternalIsLoaded(ScriptRRefBase* thisPtr)
	{
		return thisPtr->mResource.IsLoaded(false);
	}

	MonoObject* ScriptRRefBase::InternalGetResource(ScriptRRefBase* thisPtr)
	{
		if(thisPtr->mScriptResource)
			return thisPtr->mScriptResource->GetManagedInstance();

		const HResource resource = thisPtr->GetHandle();
		if(resource == nullptr)
			return nullptr;

		if(resource.IsLoaded(false))
			thisPtr->mScriptResource = ScriptResourceManager::Instance().getScriptResource(resource, true);
		else
		{
			ResourceLoadFlags loadFlags = ResourceLoadFlag::LoadDependencies;

			if (gApplication().isEditor())
				loadFlags |= ResourceLoadFlag::KeepSourceData;

			const HResource loadedResource = gResources().loadFromUUID(thisPtr->GetHandle().GetUuid(), false, loadFlags);
			thisPtr->mScriptResource = ScriptResourceManager::Instance().getScriptResource(loadedResource, true);
		}

		if(thisPtr->mScriptResource)
			return thisPtr->mScriptResource->GetManagedInstance();

		return nullptr;
	}

	void ScriptRRefBase::InternalGetUuid(ScriptRRefBase* thisPtr, UUID* uuid)
	{
		*uuid = thisPtr->GetHandle().GetUuid();
	}

	MonoObject* ScriptRRefBase::InternalCastAs(ScriptRRefBase* thisPtr, MonoReflectionType* type)
	{
		::MonoClass* rawResType = MonoUtil::getClass(type);

		MonoClass* resType = MonoManager::Instance().findClass(rawResType);
		if (resType == nullptr)
			return nullptr; // Not a valid type

		::MonoClass* rrefType = nullptr;
		if(resType == ScriptResource::getMetaData()->scriptClass ||
			resType->isSubClassOf(ScriptResource::getMetaData()->scriptClass))
			rrefType = bindGenericParam(rawResType);

		ScriptRRefBase* castRRefBase = create(thisPtr->mResource, rrefType);
		if(castRRefBase)
			return castRRefBase->GetManagedInstance();

		return nullptr;
	}
}

