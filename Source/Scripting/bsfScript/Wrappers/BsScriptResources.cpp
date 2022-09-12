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

namespace bs
{
#if BS_IS_BANSHEE3D
	ScriptResources::ScriptResources(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptResources::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_Load", (void*)&ScriptResources::internal_Load);
		metaData.scriptClass->AddInternalCall("Internal_LoadFromUUID", (void*)&ScriptResources::internal_LoadFromUUID);
		metaData.scriptClass->AddInternalCall("Internal_LoadAsync", (void*)&ScriptResources::internal_LoadAsync);
		metaData.scriptClass->AddInternalCall("Internal_LoadAsyncFromUUID", (void*)&ScriptResources::internal_LoadAsyncFromUUID);
		metaData.scriptClass->AddInternalCall("Internal_UnloadUnused", (void*)&ScriptResources::internal_UnloadUnused);
		metaData.scriptClass->AddInternalCall("Internal_Release", (void*)&ScriptResources::internal_Release);
		metaData.scriptClass->AddInternalCall("Internal_ReleaseRef", (void*)&ScriptResources::internal_ReleaseRef);
		metaData.scriptClass->AddInternalCall("Internal_GetLoadProgress", (void*)&ScriptResources::internal_GetLoadProgress);
	}

	MonoObject* ScriptResources::internal_Load(MonoString* path, ResourceLoadFlag flags)
	{
		Path nativePath = MonoUtil::monoToString(path);

		ResourceLoadFlags loadFlags = flags;

		if (gApplication().IsEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		HResource resource = GameResourceManager::instance().Load(nativePath, loadFlags, false);
		if (!resource.IsLoaded(false))
			return nullptr;

		ScriptResourceBase* scriptResource = ScriptResourceManager::instance().GetScriptResource(resource, true);
		return scriptResource->GetManagedInstance();
	}

	MonoObject* ScriptResources::internal_LoadFromUUID(UUID* uuid, ResourceLoadFlag flags)
	{
		ResourceLoadFlags loadFlags = flags;

		if (gApplication().IsEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		HResource resource = gResources().LoadFromUUID(*uuid, false, loadFlags);
		if (!resource.IsLoaded(false))
			return nullptr;

		ScriptResourceBase* scriptResource = ScriptResourceManager::instance().GetScriptResource(resource, true);
		return scriptResource->GetManagedInstance();
	}

	MonoObject* ScriptResources::internal_LoadAsync(MonoString* path, ResourceLoadFlag flags)
	{
		Path nativePath = MonoUtil::monoToString(path);

		ResourceLoadFlags loadFlags = flags;

		if (gApplication().IsEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		HResource resource = GameResourceManager::instance().Load(nativePath, loadFlags, true);
		if (resource == nullptr)
			return nullptr;

		ScriptRRefBase* scriptResource = ScriptResourceManager::instance().GetScriptRRef(resource);
		if(scriptResource != nullptr)
			return scriptResource->GetManagedInstance();

		return nullptr;
	}

	MonoObject* ScriptResources::internal_LoadAsyncFromUUID(UUID* uuid, ResourceLoadFlag flags)
	{
		ResourceLoadFlags loadFlags = flags;

		if (gApplication().IsEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		HResource resource = gResources().LoadFromUUID(*uuid, true, loadFlags);
		if (resource == nullptr)
			return nullptr;

		ScriptRRefBase* scriptResource = ScriptResourceManager::instance().GetScriptRRef(resource);
		if(scriptResource != nullptr)
			return scriptResource->GetManagedInstance();

		return nullptr;
	}

	float ScriptResources::internal_GetLoadProgress(ScriptRRefBase* resource, bool loadDependencies)
	{
		return GResources().GetLoadProgress(resource->GetHandle(), loadDependencies);
	}

	void ScriptResources::internal_Release(ScriptResourceBase* resource)
	{
		resource->GetGenericHandle().Release();
	}

	void ScriptResources::internal_ReleaseRef(ScriptRRefBase* resourceRef)
	{
		resourceRef->GetHandle().Release();
	}

	void ScriptResources::internal_UnloadUnused()
	{
		gResources().UnloadAllUnused();
	}
#endif
}
