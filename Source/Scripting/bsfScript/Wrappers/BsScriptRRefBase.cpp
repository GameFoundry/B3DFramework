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

using namespace bs;
ScriptRRefBase::ScriptRRefBase(MonoObject* instance, ResourceHandle<Resource> resource)
	: ScriptObject(instance), mResource(std::move(resource)), mGCHandle(MonoUtil::NewGcHandle(instance))
{}

ScriptRRefBase::~ScriptRRefBase()
{
	BS_ASSERT(mGCHandle == 0 && "Object being destroyed without its managed instance being freed first.");
}

void ScriptRRefBase::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_IsLoaded", (void*)&ScriptRRefBase::InternalIsLoaded);
	metaData.ScriptClass->AddInternalCall("Internal_GetResource", (void*)&ScriptRRefBase::InternalGetResource);
	metaData.ScriptClass->AddInternalCall("Internal_GetUUID", (void*)&ScriptRRefBase::InternalGetUuid);
	metaData.ScriptClass->AddInternalCall("Internal_CastAs", (void*)&ScriptRRefBase::InternalCastAs);
}

ScriptRRefBase* ScriptRRefBase::CreateInternal(const ResourceHandle<Resource>& handle, ::MonoClass* rawType)
{
	MonoClass* type = nullptr;
	if(rawType == nullptr)
		type = metaData.ScriptClass;
	else
	{
		type = MonoManager::Instance().FindClass(rawType);
		if(type == nullptr)
			type = metaData.ScriptClass;
		else
		{
			assert(type->IsSubClassOf(metaData.ScriptClass));
		}
	}

	MonoObject* obj = type->CreateInstance();
	ScriptRRefBase* output = new(bs_alloc<ScriptRRefBase>()) ScriptRRefBase(obj, handle);

	// Note: It's important this method never returns null, handles should always be created to avoid extensive null
	// checks
	return output;
}

MonoObject* ScriptRRefBase::GetManagedInstance() const
{
	return MonoUtil::GetObjectFromGcHandle(mGCHandle);
}

void ScriptRRefBase::ClearManagedInstanceInternal()
{
	if(mGCHandle != 0)
	{
		MonoUtil::FreeGcHandle(mGCHandle);
		mGCHandle = 0;
	}
}

void ScriptRRefBase::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
{
	if(mGCHandle != 0)
	{
		MonoUtil::FreeGcHandle(mGCHandle);
		mGCHandle = 0;
	}

	ScriptObjectBase::OnManagedInstanceDeletedInternal(assemblyRefresh);
}

::MonoClass* ScriptRRefBase::BindGenericParam(::MonoClass* param)
{
	MonoClass* rrefClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().GenericRRefClass;

	::MonoClass* params[1] = { param };
	return MonoUtil::BindGenericParameters(rrefClass->GetInternalClassInternal(), params, 1);
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
		thisPtr->mScriptResource = ScriptResourceManager::Instance().GetScriptResource(resource, true);
	else
	{
		ResourceLoadFlags loadFlags = ResourceLoadFlag::LoadDependencies;

		if(GetApplication().IsEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		const HResource loadedResource = GetResources().LoadFromUuid(thisPtr->GetHandle().GetUuid(), false, loadFlags);
		thisPtr->mScriptResource = ScriptResourceManager::Instance().GetScriptResource(loadedResource, true);
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
	::MonoClass* rawResType = MonoUtil::GetClass(type);

	MonoClass* resType = MonoManager::Instance().FindClass(rawResType);
	if(resType == nullptr)
		return nullptr; // Not a valid type

	::MonoClass* rrefType = nullptr;
	if(resType == ScriptResource::GetMetaData()->ScriptClass ||
	   resType->IsSubClassOf(ScriptResource::GetMetaData()->ScriptClass))
		rrefType = BindGenericParam(rawResType);

	ScriptRRefBase* castRRefBase = Create(thisPtr->mResource, rrefType);
	if(castRRefBase)
		return castRRefBase->GetManagedInstance();

	return nullptr;
}
