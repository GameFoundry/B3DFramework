//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptResources.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "BsApplication.h"
#include "BsScriptResourceWrapper.h"
#include "BsScriptRRefBase.h"

using namespace bs;
#if B3D_IS_ENGINE
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

	ResourceLoadOptions loadOptions;
	loadOptions.LoadDependencies = loadFlags.IsSet(ResourceLoadFlag::LoadDependencies);
	loadOptions.KeepInternalReference = loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef);
	loadOptions.AsynchronousLoad = false;

	HResource resource = GetResources().Load(nativePath, loadOptions);
	if(!resource.IsLoaded(false))
		return nullptr;

	return ScriptResourceWrapper::GetOrCreateScriptObject(resource);
}

MonoObject* ScriptResources::InternalLoadFromUuid(UUID* uuid, ResourceLoadFlag flags)
{
	ResourceLoadFlags loadFlags = flags;

	ResourceLoadOptions loadOptions;
	loadOptions.LoadDependencies = loadFlags.IsSet(ResourceLoadFlag::LoadDependencies);
	loadOptions.KeepInternalReference = loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef);
	loadOptions.AsynchronousLoad = false;

	HResource resource = GetResources().Load(*uuid, loadOptions);
	if(!resource.IsLoaded(false))
		return nullptr;

	return ScriptResourceWrapper::GetOrCreateScriptObject(resource);
}

MonoObject* ScriptResources::InternalLoadAsync(MonoString* path, ResourceLoadFlag flags)
{
	Path nativePath = MonoUtil::MonoToString(path);

	ResourceLoadFlags loadFlags = flags;

	ResourceLoadOptions loadOptions;
	loadOptions.LoadDependencies = loadFlags.IsSet(ResourceLoadFlag::LoadDependencies);
	loadOptions.KeepInternalReference = loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef);
	loadOptions.AsynchronousLoad = true;

	HResource resource = GetResources().Load(nativePath, loadOptions);
	if(resource == nullptr)
		return nullptr;

	ScriptRRefBase* scriptResourceReference = ScriptResourceManager::Instance().GetScriptRRef(resource);
	if(scriptResourceReference != nullptr)
		return scriptResourceReference->GetScriptObject();

	return nullptr;
}

MonoObject* ScriptResources::InternalLoadAsyncFromUuid(UUID* uuid, ResourceLoadFlag flags)
{
	ResourceLoadFlags loadFlags = flags;

	ResourceLoadOptions loadOptions;
	loadOptions.LoadDependencies = loadFlags.IsSet(ResourceLoadFlag::LoadDependencies);
	loadOptions.KeepInternalReference = loadFlags.IsSet(ResourceLoadFlag::KeepInternalRef);

	HResource resource = GetResources().Load(*uuid, loadOptions);
	if(resource == nullptr)
		return nullptr;

	ScriptRRefBase* scriptResource = ScriptResourceManager::Instance().GetScriptRRef(resource);
	if(scriptResource != nullptr)
		return scriptResource->GetScriptObject();

	return nullptr;
}

float ScriptResources::InternalGetLoadProgress(ScriptRRefBase* resource)
{
	return GetResources().GetLoadProgress(resource->GetNativeObjectAsHandle());
}

void ScriptResources::InternalRelease(ScriptResourceWrapper* resource)
{
	HResource mutableResourceHandle = resource->GetBaseNativeObjectAsHandle();
	mutableResourceHandle.ReleaseInternalReference();
}

void ScriptResources::InternalReleaseRef(ScriptRRefBase* resourceRef)
{
	HResource mutableResourceHandle = resourceRef->GetBaseNativeObjectAsHandle();
	mutableResourceHandle.ReleaseInternalReference();
}

void ScriptResources::InternalUnloadUnused()
{
	GetResources().UnloadAllUnused();
}
#endif
