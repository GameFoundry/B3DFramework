//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptResources.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Resources/BsGameResourceManager.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptResource.h"
#include "BsApplication.h"

using namespace bs;
#if BS_IS_BANSHEE3D
ScriptResources::ScriptResources(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptResources::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_Load", (void*)&ScriptResources::InternalLoad);
	metaData.ScriptClass->AddInternalCall("Internal_LoadFromUUID", (void*)&ScriptResources::InternalLoadFromUuid);
	metaData.ScriptClass->AddInternalCall("Internal_LoadAsync", (void*)&ScriptResources::InternalLoadAsync);
	metaData.ScriptClass->AddInternalCall("Internal_LoadAsyncFromUUID", (void*)&ScriptResources::InternalLoadAsyncFromUuid);
	metaData.ScriptClass->AddInternalCall("Internal_UnloadUnused", (void*)&ScriptResources::InternalUnloadUnused);
	metaData.ScriptClass->AddInternalCall("Internal_Release", (void*)&ScriptResources::InternalRelease);
	metaData.ScriptClass->AddInternalCall("Internal_ReleaseRef", (void*)&ScriptResources::InternalReleaseRef);
	metaData.ScriptClass->AddInternalCall("Internal_GetLoadProgress", (void*)&ScriptResources::InternalGetLoadProgress);
}

MonoObject* ScriptResources::InternalLoad(MonoString* path, ResourceLoadFlag flags)
{
	Path nativePath = MonoUtil::MonoToString(path);

	ResourceLoadFlags loadFlags = flags;

	if(GetApplication().IsEditor())
		loadFlags |= ResourceLoadFlag::KeepSourceData;

	HResource resource = GameResourceManager::Instance().Load(nativePath, loadFlags, false);
	if(!resource.IsLoaded(false))
		return nullptr;

	ScriptResourceBase* scriptResource = ScriptResourceManager::Instance().GetScriptResource(resource, true);
	return scriptResource->GetManagedInstance();
}

MonoObject* ScriptResources::InternalLoadFromUuid(UUID* uuid, ResourceLoadFlag flags)
{
	ResourceLoadFlags loadFlags = flags;

	if(GetApplication().IsEditor())
		loadFlags |= ResourceLoadFlag::KeepSourceData;

	HResource resource = GetResources().LoadFromUuid(*uuid, false, loadFlags);
	if(!resource.IsLoaded(false))
		return nullptr;

	ScriptResourceBase* scriptResource = ScriptResourceManager::Instance().GetScriptResource(resource, true);
	return scriptResource->GetManagedInstance();
}

MonoObject* ScriptResources::InternalLoadAsync(MonoString* path, ResourceLoadFlag flags)
{
	Path nativePath = MonoUtil::MonoToString(path);

	ResourceLoadFlags loadFlags = flags;

	if(GetApplication().IsEditor())
		loadFlags |= ResourceLoadFlag::KeepSourceData;

	HResource resource = GameResourceManager::Instance().Load(nativePath, loadFlags, true);
	if(resource == nullptr)
		return nullptr;

	ScriptRRefBase* scriptResource = ScriptResourceManager::Instance().GetScriptRRef(resource);
	if(scriptResource != nullptr)
		return scriptResource->GetManagedInstance();

	return nullptr;
}

MonoObject* ScriptResources::InternalLoadAsyncFromUuid(UUID* uuid, ResourceLoadFlag flags)
{
	ResourceLoadFlags loadFlags = flags;

	if(GetApplication().IsEditor())
		loadFlags |= ResourceLoadFlag::KeepSourceData;

	HResource resource = GetResources().LoadFromUuid(*uuid, true, loadFlags);
	if(resource == nullptr)
		return nullptr;

	ScriptRRefBase* scriptResource = ScriptResourceManager::Instance().GetScriptRRef(resource);
	if(scriptResource != nullptr)
		return scriptResource->GetManagedInstance();

	return nullptr;
}

float ScriptResources::InternalGetLoadProgress(ScriptRRefBase* resource, bool loadDependencies)
{
	return GetResources().GetLoadProgress(resource->GetHandle(), loadDependencies);
}

void ScriptResources::InternalRelease(ScriptResourceBase* resource)
{
	resource->GetGenericHandle().Release();
}

void ScriptResources::InternalReleaseRef(ScriptRRefBase* resourceRef)
{
	resourceRef->GetHandle().Release();
}

void ScriptResources::InternalUnloadUnused()
{
	GetResources().UnloadAllUnused();
}
#endif
