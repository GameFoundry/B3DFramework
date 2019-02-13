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

	void ScriptResources::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Load", (void*)&ScriptResources::internal_Load);
		metaData.scriptClass->addInternalCall("Internal_LoadFromUUID", (void*)&ScriptResources::internal_LoadFromUUID);
		metaData.scriptClass->addInternalCall("Internal_LoadAsync", (void*)&ScriptResources::internal_LoadAsync);
		metaData.scriptClass->addInternalCall("Internal_LoadAsyncFromUUID", (void*)&ScriptResources::internal_LoadAsyncFromUUID);
		metaData.scriptClass->addInternalCall("Internal_UnloadUnused", (void*)&ScriptResources::internal_UnloadUnused);
		metaData.scriptClass->addInternalCall("Internal_Release", (void*)&ScriptResources::internal_Release);
		metaData.scriptClass->addInternalCall("Internal_ReleaseRef", (void*)&ScriptResources::internal_ReleaseRef);
		metaData.scriptClass->addInternalCall("Internal_GetLoadProgress", (void*)&ScriptResources::internal_GetLoadProgress);
	}

	MonoObject* ScriptResources::internal_Load(MonoString* path, ResourceLoadFlag flags)
	{
		Path nativePath = MonoUtil::monoToString(path);

		HResource resource = GameResourceManager::instance().load(nativePath, flags, false);
		if (resource == nullptr)
			return nullptr;

		ScriptResourceBase* scriptResource = ScriptResourceManager::instance().getScriptResource(resource, true);
		return scriptResource->getManagedInstance();
	}

	MonoObject* ScriptResources::internal_LoadFromUUID(UUID* uuid, ResourceLoadFlag flags)
	{
		ResourceLoadFlags loadFlags = flags;

		if (gApplication().isEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		HResource resource = gResources().loadFromUUID(*uuid, loadFlags);
		if (resource == nullptr)
			return nullptr;

		ScriptResourceBase* scriptResource = ScriptResourceManager::instance().getScriptResource(resource, true);
		return scriptResource->getManagedInstance();
	}

	MonoObject* ScriptResources::internal_LoadAsync(MonoString* path, ResourceLoadFlag flags)
	{
		Path nativePath = MonoUtil::monoToString(path);

		HResource resource = GameResourceManager::instance().load(nativePath, flags, true);
		if (resource == nullptr)
			return nullptr;

		ScriptRRefBase* scriptResource = ScriptResourceManager::instance().getScriptRRef(resource);
		if(scriptResource != nullptr)
			return scriptResource->getManagedInstance();

		return nullptr;
	}

	MonoObject* ScriptResources::internal_LoadAsyncFromUUID(UUID* uuid, ResourceLoadFlag flags)
	{
		ResourceLoadFlags loadFlags = flags;

		if (gApplication().isEditor())
			loadFlags |= ResourceLoadFlag::KeepSourceData;

		HResource resource = gResources().loadFromUUID(*uuid, loadFlags);
		if (resource == nullptr)
			return nullptr;

		ScriptRRefBase* scriptResource = ScriptResourceManager::instance().getScriptRRef(resource);
		if(scriptResource != nullptr)
			return scriptResource->getManagedInstance();

		return nullptr;
	}

	float ScriptResources::internal_GetLoadProgress(ScriptRRefBase* resource, bool loadDependencies)
	{
		return gResources().getLoadProgress(resource->getHandle(), loadDependencies);
	}

	void ScriptResources::internal_Release(ScriptResourceBase* resource)
	{
		resource->getGenericHandle().release();
	}

	void ScriptResources::internal_ReleaseRef(ScriptRRefBase* resourceRef)
	{
		resourceRef->getHandle().release();
	}

	void ScriptResources::internal_UnloadUnused()
	{
		gResources().unloadAllUnused();
	}
#endif
}